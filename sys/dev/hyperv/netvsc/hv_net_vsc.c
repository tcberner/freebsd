/*-
 * Copyright (c) 2009-2012 Microsoft Corp.
 * Copyright (c) 2010-2012 Citrix Inc.
 * Copyright (c) 2012 NetApp Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */

/**
 * HyperV vmbus network VSC (virtual services client) module
 *
 */


#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/lock.h>
#include <net/if.h>
#include <net/if_var.h>
#include <net/if_arp.h>
#include <machine/bus.h>
#include <machine/atomic.h>

#include <dev/hyperv/include/hyperv.h>
#include "hv_net_vsc.h"
#include "hv_rndis.h"
#include "hv_rndis_filter.h"

/* priv1 and priv2 are consumed by the main driver */
#define hv_chan_rdbuf	hv_chan_priv3

MALLOC_DEFINE(M_NETVSC, "netvsc", "Hyper-V netvsc driver");

/*
 * Forward declarations
 */
static void hv_nv_on_channel_callback(void *xchan);
static int  hv_nv_init_send_buffer_with_net_vsp(struct hv_device *device);
static int  hv_nv_init_rx_buffer_with_net_vsp(struct hv_device *device);
static int  hv_nv_destroy_send_buffer(netvsc_dev *net_dev);
static int  hv_nv_destroy_rx_buffer(netvsc_dev *net_dev);
static int  hv_nv_connect_to_vsp(struct hv_device *device);
static void hv_nv_on_send_completion(netvsc_dev *net_dev,
    struct hv_device *device, struct hv_vmbus_channel *, hv_vm_packet_descriptor *pkt);
static void hv_nv_on_receive_completion(struct hv_vmbus_channel *chan,
    uint64_t tid, uint32_t status);
static void hv_nv_on_receive(netvsc_dev *net_dev,
    struct hv_device *device, struct hv_vmbus_channel *chan,
    hv_vm_packet_descriptor *pkt);

/*
 *
 */
static inline netvsc_dev *
hv_nv_alloc_net_device(struct hv_device *device)
{
	netvsc_dev *net_dev;
	hn_softc_t *sc = device_get_softc(device->device);

	net_dev = malloc(sizeof(netvsc_dev), M_NETVSC, M_WAITOK | M_ZERO);

	net_dev->dev = device;
	net_dev->destroy = FALSE;
	sc->net_dev = net_dev;

	return (net_dev);
}

/*
 *
 */
static inline netvsc_dev *
hv_nv_get_outbound_net_device(struct hv_device *device)
{
	hn_softc_t *sc = device_get_softc(device->device);
	netvsc_dev *net_dev = sc->net_dev;;

	if ((net_dev != NULL) && net_dev->destroy) {
		return (NULL);
	}

	return (net_dev);
}

/*
 *
 */
static inline netvsc_dev *
hv_nv_get_inbound_net_device(struct hv_device *device)
{
	hn_softc_t *sc = device_get_softc(device->device);
	netvsc_dev *net_dev = sc->net_dev;;

	if (net_dev == NULL) {
		return (net_dev);
	}
	/*
	 * When the device is being destroyed; we only
	 * permit incoming packets if and only if there
	 * are outstanding sends.
	 */
	if (net_dev->destroy) {
		return (NULL);
	}

	return (net_dev);
}

int
hv_nv_get_next_send_section(netvsc_dev *net_dev)
{
	unsigned long bitsmap_words = net_dev->bitsmap_words;
	unsigned long *bitsmap = net_dev->send_section_bitsmap;
	unsigned long idx;
	int ret = NVSP_1_CHIMNEY_SEND_INVALID_SECTION_INDEX;
	int i;

	for (i = 0; i < bitsmap_words; i++) {
		idx = ffsl(~bitsmap[i]);
		if (0 == idx)
			continue;

		idx--;
		KASSERT(i * BITS_PER_LONG + idx < net_dev->send_section_count,
		    ("invalid i %d and idx %lu", i, idx));

		if (atomic_testandset_long(&bitsmap[i], idx))
			continue;

		ret = i * BITS_PER_LONG + idx;
		break;
	}

	return (ret);
}

/*
 * Net VSC initialize receive buffer with net VSP
 * 
 * Net VSP:  Network virtual services client, also known as the
 *     Hyper-V extensible switch and the synthetic data path.
 */
static int 
hv_nv_init_rx_buffer_with_net_vsp(struct hv_device *device)
{
	netvsc_dev *net_dev;
	nvsp_msg *init_pkt;
	int ret = 0;

	net_dev = hv_nv_get_outbound_net_device(device);
	if (!net_dev) {
		return (ENODEV);
	}

	net_dev->rx_buf = contigmalloc(net_dev->rx_buf_size, M_NETVSC,
	    M_ZERO, 0UL, BUS_SPACE_MAXADDR, PAGE_SIZE, 0);

	/*
	 * Establish the GPADL handle for this buffer on this channel.
	 * Note:  This call uses the vmbus connection rather than the
	 * channel to establish the gpadl handle. 
	 * GPADL:  Guest physical address descriptor list.
	 */
	ret = hv_vmbus_channel_establish_gpadl(
		device->channel, net_dev->rx_buf,
		net_dev->rx_buf_size, &net_dev->rx_buf_gpadl_handle);
	if (ret != 0) {
		goto cleanup;
	}
	
	/* sema_wait(&ext->channel_init_sema); KYS CHECK */

	/* Notify the NetVsp of the gpadl handle */
	init_pkt = &net_dev->channel_init_packet;

	memset(init_pkt, 0, sizeof(nvsp_msg));

	init_pkt->hdr.msg_type = nvsp_msg_1_type_send_rx_buf;
	init_pkt->msgs.vers_1_msgs.send_rx_buf.gpadl_handle =
	    net_dev->rx_buf_gpadl_handle;
	init_pkt->msgs.vers_1_msgs.send_rx_buf.id =
	    NETVSC_RECEIVE_BUFFER_ID;

	/* Send the gpadl notification request */

	ret = hv_vmbus_channel_send_packet(device->channel, init_pkt,
	    sizeof(nvsp_msg), (uint64_t)(uintptr_t)init_pkt,
	    HV_VMBUS_PACKET_TYPE_DATA_IN_BAND,
	    HV_VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	if (ret != 0) {
		goto cleanup;
	}

	sema_wait(&net_dev->channel_init_sema);

	/* Check the response */
	if (init_pkt->msgs.vers_1_msgs.send_rx_buf_complete.status
	    != nvsp_status_success) {
		ret = EINVAL;
		goto cleanup;
	}

	net_dev->rx_section_count =
	    init_pkt->msgs.vers_1_msgs.send_rx_buf_complete.num_sections;

	net_dev->rx_sections = malloc(net_dev->rx_section_count *
	    sizeof(nvsp_1_rx_buf_section), M_NETVSC, M_WAITOK);
	memcpy(net_dev->rx_sections, 
	    init_pkt->msgs.vers_1_msgs.send_rx_buf_complete.sections,
	    net_dev->rx_section_count * sizeof(nvsp_1_rx_buf_section));


	/*
	 * For first release, there should only be 1 section that represents
	 * the entire receive buffer
	 */
	if (net_dev->rx_section_count != 1
	    || net_dev->rx_sections->offset != 0) {
		ret = EINVAL;
		goto cleanup;
	}

	goto exit;

cleanup:
	hv_nv_destroy_rx_buffer(net_dev);
	
exit:
	return (ret);
}

/*
 * Net VSC initialize send buffer with net VSP
 */
static int 
hv_nv_init_send_buffer_with_net_vsp(struct hv_device *device)
{
	netvsc_dev *net_dev;
	nvsp_msg *init_pkt;
	int ret = 0;

	net_dev = hv_nv_get_outbound_net_device(device);
	if (!net_dev) {
		return (ENODEV);
	}

	net_dev->send_buf  = contigmalloc(net_dev->send_buf_size, M_NETVSC,
	    M_ZERO, 0UL, BUS_SPACE_MAXADDR, PAGE_SIZE, 0);
	if (net_dev->send_buf == NULL) {
		ret = ENOMEM;
		goto cleanup;
	}

	/*
	 * Establish the gpadl handle for this buffer on this channel.
	 * Note:  This call uses the vmbus connection rather than the
	 * channel to establish the gpadl handle. 
	 */
	ret = hv_vmbus_channel_establish_gpadl(device->channel,
  	    net_dev->send_buf, net_dev->send_buf_size,
	    &net_dev->send_buf_gpadl_handle);
	if (ret != 0) {
		goto cleanup;
	}

	/* Notify the NetVsp of the gpadl handle */

	init_pkt = &net_dev->channel_init_packet;

	memset(init_pkt, 0, sizeof(nvsp_msg));

	init_pkt->hdr.msg_type = nvsp_msg_1_type_send_send_buf;
	init_pkt->msgs.vers_1_msgs.send_rx_buf.gpadl_handle =
	    net_dev->send_buf_gpadl_handle;
	init_pkt->msgs.vers_1_msgs.send_rx_buf.id =
	    NETVSC_SEND_BUFFER_ID;

	/* Send the gpadl notification request */

	ret = hv_vmbus_channel_send_packet(device->channel, init_pkt,
  	    sizeof(nvsp_msg), (uint64_t)init_pkt,
	    HV_VMBUS_PACKET_TYPE_DATA_IN_BAND,
	    HV_VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	if (ret != 0) {
		goto cleanup;
	}

	sema_wait(&net_dev->channel_init_sema);

	/* Check the response */
	if (init_pkt->msgs.vers_1_msgs.send_send_buf_complete.status
	    != nvsp_status_success) {
		ret = EINVAL;
		goto cleanup;
	}

	net_dev->send_section_size =
	    init_pkt->msgs.vers_1_msgs.send_send_buf_complete.section_size;
	net_dev->send_section_count =
	    net_dev->send_buf_size / net_dev->send_section_size;
	net_dev->bitsmap_words = howmany(net_dev->send_section_count,
	    BITS_PER_LONG);
	net_dev->send_section_bitsmap =
	    malloc(net_dev->bitsmap_words * sizeof(long), M_NETVSC,
	    M_WAITOK | M_ZERO);

	goto exit;

cleanup:
	hv_nv_destroy_send_buffer(net_dev);
	
exit:
	return (ret);
}

/*
 * Net VSC destroy receive buffer
 */
static int
hv_nv_destroy_rx_buffer(netvsc_dev *net_dev)
{
	nvsp_msg *revoke_pkt;
	int ret = 0;

	/*
	 * If we got a section count, it means we received a
	 * send_rx_buf_complete msg 
	 * (ie sent nvsp_msg_1_type_send_rx_buf msg) therefore,
	 * we need to send a revoke msg here
	 */
	if (net_dev->rx_section_count) {
		/* Send the revoke receive buffer */
		revoke_pkt = &net_dev->revoke_packet;
		memset(revoke_pkt, 0, sizeof(nvsp_msg));

		revoke_pkt->hdr.msg_type = nvsp_msg_1_type_revoke_rx_buf;
		revoke_pkt->msgs.vers_1_msgs.revoke_rx_buf.id =
		    NETVSC_RECEIVE_BUFFER_ID;

		ret = hv_vmbus_channel_send_packet(net_dev->dev->channel,
		    revoke_pkt, sizeof(nvsp_msg),
		    (uint64_t)(uintptr_t)revoke_pkt,
		    HV_VMBUS_PACKET_TYPE_DATA_IN_BAND, 0);

		/*
		 * If we failed here, we might as well return and have a leak 
		 * rather than continue and a bugchk
		 */
		if (ret != 0) {
			return (ret);
		}
	}
		
	/* Tear down the gpadl on the vsp end */
	if (net_dev->rx_buf_gpadl_handle) {
		ret = hv_vmbus_channel_teardown_gpdal(net_dev->dev->channel,
		    net_dev->rx_buf_gpadl_handle);
		/*
		 * If we failed here, we might as well return and have a leak 
		 * rather than continue and a bugchk
		 */
		if (ret != 0) {
			return (ret);
		}
		net_dev->rx_buf_gpadl_handle = 0;
	}

	if (net_dev->rx_buf) {
		/* Free up the receive buffer */
		contigfree(net_dev->rx_buf, net_dev->rx_buf_size, M_NETVSC);
		net_dev->rx_buf = NULL;
	}

	if (net_dev->rx_sections) {
		free(net_dev->rx_sections, M_NETVSC);
		net_dev->rx_sections = NULL;
		net_dev->rx_section_count = 0;
	}

	return (ret);
}

/*
 * Net VSC destroy send buffer
 */
static int
hv_nv_destroy_send_buffer(netvsc_dev *net_dev)
{
	nvsp_msg *revoke_pkt;
	int ret = 0;

	/*
	 * If we got a section count, it means we received a
	 * send_rx_buf_complete msg 
	 * (ie sent nvsp_msg_1_type_send_rx_buf msg) therefore,
	 * we need to send a revoke msg here
	 */
	if (net_dev->send_section_size) {
		/* Send the revoke send buffer */
		revoke_pkt = &net_dev->revoke_packet;
		memset(revoke_pkt, 0, sizeof(nvsp_msg));

		revoke_pkt->hdr.msg_type =
		    nvsp_msg_1_type_revoke_send_buf;
		revoke_pkt->msgs.vers_1_msgs.revoke_send_buf.id =
		    NETVSC_SEND_BUFFER_ID;

		ret = hv_vmbus_channel_send_packet(net_dev->dev->channel,
		    revoke_pkt, sizeof(nvsp_msg),
		    (uint64_t)(uintptr_t)revoke_pkt,
		    HV_VMBUS_PACKET_TYPE_DATA_IN_BAND, 0);
		/*
		 * If we failed here, we might as well return and have a leak 
		 * rather than continue and a bugchk
		 */
		if (ret != 0) {
			return (ret);
		}
	}
		
	/* Tear down the gpadl on the vsp end */
	if (net_dev->send_buf_gpadl_handle) {
		ret = hv_vmbus_channel_teardown_gpdal(net_dev->dev->channel,
		    net_dev->send_buf_gpadl_handle);

		/*
		 * If we failed here, we might as well return and have a leak 
		 * rather than continue and a bugchk
		 */
		if (ret != 0) {
			return (ret);
		}
		net_dev->send_buf_gpadl_handle = 0;
	}

	if (net_dev->send_buf) {
		/* Free up the receive buffer */
		contigfree(net_dev->send_buf, net_dev->send_buf_size, M_NETVSC);
		net_dev->send_buf = NULL;
	}

	if (net_dev->send_section_bitsmap) {
		free(net_dev->send_section_bitsmap, M_NETVSC);
	}

	return (ret);
}


/*
 * Attempt to negotiate the caller-specified NVSP version
 *
 * For NVSP v2, Server 2008 R2 does not set
 * init_pkt->msgs.init_msgs.init_compl.negotiated_prot_vers
 * to the negotiated version, so we cannot rely on that.
 */
static int
hv_nv_negotiate_nvsp_protocol(struct hv_device *device, netvsc_dev *net_dev,
    uint32_t nvsp_ver)
{
	nvsp_msg *init_pkt;
	int ret;

	init_pkt = &net_dev->channel_init_packet;
	memset(init_pkt, 0, sizeof(nvsp_msg));
	init_pkt->hdr.msg_type = nvsp_msg_type_init;

	/*
	 * Specify parameter as the only acceptable protocol version
	 */
	init_pkt->msgs.init_msgs.init.p1.protocol_version = nvsp_ver;
	init_pkt->msgs.init_msgs.init.protocol_version_2 = nvsp_ver;

	/* Send the init request */
	ret = hv_vmbus_channel_send_packet(device->channel, init_pkt,
	    sizeof(nvsp_msg), (uint64_t)(uintptr_t)init_pkt,
	    HV_VMBUS_PACKET_TYPE_DATA_IN_BAND,
	    HV_VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	if (ret != 0)
		return (-1);

	sema_wait(&net_dev->channel_init_sema);

	if (init_pkt->msgs.init_msgs.init_compl.status != nvsp_status_success)
		return (EINVAL);

	return (0);
}

/*
 * Send NDIS version 2 config packet containing MTU.
 *
 * Not valid for NDIS version 1.
 */
static int
hv_nv_send_ndis_config(struct hv_device *device, uint32_t mtu)
{
	netvsc_dev *net_dev;
	nvsp_msg *init_pkt;
	int ret;

	net_dev = hv_nv_get_outbound_net_device(device);
	if (!net_dev)
		return (-ENODEV);

	/*
	 * Set up configuration packet, write MTU
	 * Indicate we are capable of handling VLAN tags
	 */
	init_pkt = &net_dev->channel_init_packet;
	memset(init_pkt, 0, sizeof(nvsp_msg));
	init_pkt->hdr.msg_type = nvsp_msg_2_type_send_ndis_config;
	init_pkt->msgs.vers_2_msgs.send_ndis_config.mtu = mtu;
	init_pkt->
		msgs.vers_2_msgs.send_ndis_config.capabilities.u1.u2.ieee8021q
		= 1;

	/* Send the configuration packet */
	ret = hv_vmbus_channel_send_packet(device->channel, init_pkt,
	    sizeof(nvsp_msg), (uint64_t)(uintptr_t)init_pkt,
	    HV_VMBUS_PACKET_TYPE_DATA_IN_BAND, 0);
	if (ret != 0)
		return (-EINVAL);

	return (0);
}

/*
 * Net VSC connect to VSP
 */
static int
hv_nv_connect_to_vsp(struct hv_device *device)
{
	netvsc_dev *net_dev;
	nvsp_msg *init_pkt;
	uint32_t ndis_version;
	uint32_t protocol_list[] = { NVSP_PROTOCOL_VERSION_1,
	    NVSP_PROTOCOL_VERSION_2,
	    NVSP_PROTOCOL_VERSION_4,
	    NVSP_PROTOCOL_VERSION_5 };
	int i;
	int protocol_number = nitems(protocol_list);
	int ret = 0;
	device_t dev = device->device;
	hn_softc_t *sc = device_get_softc(dev);
	struct ifnet *ifp = sc->hn_ifp;

	net_dev = hv_nv_get_outbound_net_device(device);
	if (!net_dev) {
		return (ENODEV);
	}

	/*
	 * Negotiate the NVSP version.  Try the latest NVSP first.
	 */
	for (i = protocol_number - 1; i >= 0; i--) {
		if (hv_nv_negotiate_nvsp_protocol(device, net_dev,
		    protocol_list[i]) == 0) {
			net_dev->nvsp_version = protocol_list[i];
			if (bootverbose)
				device_printf(dev, "Netvsc: got version 0x%x\n",
				    net_dev->nvsp_version);
			break;
		}
	}

	if (i < 0) {
		if (bootverbose)
			device_printf(dev, "failed to negotiate a valid "
			    "protocol.\n");
		return (EPROTO);
	}

	/*
	 * Set the MTU if supported by this NVSP protocol version
	 * This needs to be right after the NVSP init message per Haiyang
	 */
	if (net_dev->nvsp_version >= NVSP_PROTOCOL_VERSION_2)
		ret = hv_nv_send_ndis_config(device, ifp->if_mtu);

	/*
	 * Send the NDIS version
	 */
	init_pkt = &net_dev->channel_init_packet;

	memset(init_pkt, 0, sizeof(nvsp_msg));

	if (net_dev->nvsp_version <= NVSP_PROTOCOL_VERSION_4) {
		ndis_version = NDIS_VERSION_6_1;
	} else {
		ndis_version = NDIS_VERSION_6_30;
	}

	init_pkt->hdr.msg_type = nvsp_msg_1_type_send_ndis_vers;
	init_pkt->msgs.vers_1_msgs.send_ndis_vers.ndis_major_vers =
	    (ndis_version & 0xFFFF0000) >> 16;
	init_pkt->msgs.vers_1_msgs.send_ndis_vers.ndis_minor_vers =
	    ndis_version & 0xFFFF;

	/* Send the init request */

	ret = hv_vmbus_channel_send_packet(device->channel, init_pkt,
	    sizeof(nvsp_msg), (uint64_t)(uintptr_t)init_pkt,
	    HV_VMBUS_PACKET_TYPE_DATA_IN_BAND, 0);
	if (ret != 0) {
		goto cleanup;
	}
	/*
	 * TODO:  BUGBUG - We have to wait for the above msg since the netvsp
	 * uses KMCL which acknowledges packet (completion packet) 
	 * since our Vmbus always set the
	 * HV_VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED flag
	 */
	/* sema_wait(&NetVscChannel->channel_init_sema); */

	/* Post the big receive buffer to NetVSP */
	if (net_dev->nvsp_version <= NVSP_PROTOCOL_VERSION_2)
		net_dev->rx_buf_size = NETVSC_RECEIVE_BUFFER_SIZE_LEGACY;
	else
		net_dev->rx_buf_size = NETVSC_RECEIVE_BUFFER_SIZE;
	net_dev->send_buf_size = NETVSC_SEND_BUFFER_SIZE;

	ret = hv_nv_init_rx_buffer_with_net_vsp(device);
	if (ret == 0)
		ret = hv_nv_init_send_buffer_with_net_vsp(device);

cleanup:
	return (ret);
}

/*
 * Net VSC disconnect from VSP
 */
static void
hv_nv_disconnect_from_vsp(netvsc_dev *net_dev)
{
	hv_nv_destroy_rx_buffer(net_dev);
	hv_nv_destroy_send_buffer(net_dev);
}

/*
 * Callback handler for subchannel offer
 * @@param context new subchannel
 */
static void
hv_nv_subchan_callback(void *xchan)
{
	struct hv_vmbus_channel *chan = xchan;
	netvsc_dev *net_dev;
	uint16_t chn_index = chan->offer_msg.offer.sub_channel_index;
	struct hv_device *device = chan->device;
	hn_softc_t *sc = device_get_softc(device->device);
	int ret;

	net_dev = sc->net_dev;

	if (chn_index >= net_dev->num_channel) {
		/* Would this ever happen? */
		return;
	}
	netvsc_subchan_callback(sc, chan);

	chan->hv_chan_rdbuf = malloc(NETVSC_PACKET_SIZE, M_NETVSC, M_WAITOK);
	ret = hv_vmbus_channel_open(chan, NETVSC_DEVICE_RING_BUFFER_SIZE,
	    NETVSC_DEVICE_RING_BUFFER_SIZE, NULL, 0,
	    hv_nv_on_channel_callback, chan);
}

/*
 * Net VSC on device add
 * 
 * Callback when the device belonging to this driver is added
 */
netvsc_dev *
hv_nv_on_device_add(struct hv_device *device, void *additional_info)
{
	struct hv_vmbus_channel *chan = device->channel;
	netvsc_dev *net_dev;
	int ret = 0;

	net_dev = hv_nv_alloc_net_device(device);
	if (net_dev == NULL)
		return NULL;

	/* Initialize the NetVSC channel extension */

	sema_init(&net_dev->channel_init_sema, 0, "netdev_sema");

	chan->hv_chan_rdbuf = malloc(NETVSC_PACKET_SIZE, M_NETVSC, M_WAITOK);

	/*
	 * Open the channel
	 */
	ret = hv_vmbus_channel_open(chan,
	    NETVSC_DEVICE_RING_BUFFER_SIZE, NETVSC_DEVICE_RING_BUFFER_SIZE,
	    NULL, 0, hv_nv_on_channel_callback, chan);
	if (ret != 0) {
		free(chan->hv_chan_rdbuf, M_NETVSC);
		goto cleanup;
	}
	chan->sc_creation_callback = hv_nv_subchan_callback;

	/*
	 * Connect with the NetVsp
	 */
	ret = hv_nv_connect_to_vsp(device);
	if (ret != 0)
		goto close;

	return (net_dev);

close:
	/* Now, we can close the channel safely */
	free(chan->hv_chan_rdbuf, M_NETVSC);
	hv_vmbus_channel_close(chan);

cleanup:
	/*
	 * Free the packet buffers on the netvsc device packet queue.
	 * Release other resources.
	 */
	sema_destroy(&net_dev->channel_init_sema);
	free(net_dev, M_NETVSC);

	return (NULL);
}

/*
 * Net VSC on device remove
 */
int
hv_nv_on_device_remove(struct hv_device *device, boolean_t destroy_channel)
{
	hn_softc_t *sc = device_get_softc(device->device);
	netvsc_dev *net_dev = sc->net_dev;;
	
	/* Stop outbound traffic ie sends and receives completions */
	net_dev->destroy = TRUE;

	hv_nv_disconnect_from_vsp(net_dev);

	/* At this point, no one should be accessing net_dev except in here */

	/* Now, we can close the channel safely */

	if (!destroy_channel) {
		device->channel->state =
		    HV_CHANNEL_CLOSING_NONDESTRUCTIVE_STATE;
	}

	free(device->channel->hv_chan_rdbuf, M_NETVSC);
	hv_vmbus_channel_close(device->channel);

	sema_destroy(&net_dev->channel_init_sema);
	free(net_dev, M_NETVSC);

	return (0);
}

/*
 * Net VSC on send completion
 */
static void
hv_nv_on_send_completion(netvsc_dev *net_dev,
    struct hv_device *device, struct hv_vmbus_channel *chan,
    hv_vm_packet_descriptor *pkt)
{
	nvsp_msg *nvsp_msg_pkt;
	netvsc_packet *net_vsc_pkt;

	nvsp_msg_pkt =
	    (nvsp_msg *)((unsigned long)pkt + (pkt->data_offset8 << 3));

	if (nvsp_msg_pkt->hdr.msg_type == nvsp_msg_type_init_complete
		|| nvsp_msg_pkt->hdr.msg_type
			== nvsp_msg_1_type_send_rx_buf_complete
		|| nvsp_msg_pkt->hdr.msg_type
			== nvsp_msg_1_type_send_send_buf_complete
		|| nvsp_msg_pkt->hdr.msg_type
			== nvsp_msg5_type_subchannel) {
		/* Copy the response back */
		memcpy(&net_dev->channel_init_packet, nvsp_msg_pkt,
		    sizeof(nvsp_msg));
		sema_post(&net_dev->channel_init_sema);
	} else if (nvsp_msg_pkt->hdr.msg_type ==
		    nvsp_msg_1_type_send_rndis_pkt_complete) {
		/* Get the send context */
		net_vsc_pkt =
		    (netvsc_packet *)(unsigned long)pkt->transaction_id;
		if (NULL != net_vsc_pkt) {
			if (net_vsc_pkt->send_buf_section_idx !=
			    NVSP_1_CHIMNEY_SEND_INVALID_SECTION_INDEX) {
				u_long mask;
				int idx;

				idx = net_vsc_pkt->send_buf_section_idx /
				    BITS_PER_LONG;
				KASSERT(idx < net_dev->bitsmap_words,
				    ("invalid section index %u",
				     net_vsc_pkt->send_buf_section_idx));
				mask = 1UL <<
				    (net_vsc_pkt->send_buf_section_idx %
				     BITS_PER_LONG);

				KASSERT(net_dev->send_section_bitsmap[idx] &
				    mask,
				    ("index bitmap 0x%lx, section index %u, "
				     "bitmap idx %d, bitmask 0x%lx",
				     net_dev->send_section_bitsmap[idx],
				     net_vsc_pkt->send_buf_section_idx,
				     idx, mask));
				atomic_clear_long(
				    &net_dev->send_section_bitsmap[idx], mask);
			}
			
			/* Notify the layer above us */
			net_vsc_pkt->compl.send.on_send_completion(chan,
			    net_vsc_pkt->compl.send.send_completion_context);

		}
	}
}

/*
 * Net VSC on send
 * Sends a packet on the specified Hyper-V device.
 * Returns 0 on success, non-zero on failure.
 */
int
hv_nv_on_send(struct hv_vmbus_channel *chan, netvsc_packet *pkt)
{
	nvsp_msg send_msg;
	int ret;

	send_msg.hdr.msg_type = nvsp_msg_1_type_send_rndis_pkt;
	if (pkt->is_data_pkt) {
		/* 0 is RMC_DATA */
		send_msg.msgs.vers_1_msgs.send_rndis_pkt.chan_type = 0;
	} else {
		/* 1 is RMC_CONTROL */
		send_msg.msgs.vers_1_msgs.send_rndis_pkt.chan_type = 1;
	}

	send_msg.msgs.vers_1_msgs.send_rndis_pkt.send_buf_section_idx =
	    pkt->send_buf_section_idx;
	send_msg.msgs.vers_1_msgs.send_rndis_pkt.send_buf_section_size =
	    pkt->send_buf_section_size;

	if (pkt->page_buf_count) {
		ret = hv_vmbus_channel_send_packet_pagebuffer(chan,
		    pkt->page_buffers, pkt->page_buf_count,
		    &send_msg, sizeof(nvsp_msg), (uint64_t)(uintptr_t)pkt);
	} else {
		ret = hv_vmbus_channel_send_packet(chan,
		    &send_msg, sizeof(nvsp_msg), (uint64_t)(uintptr_t)pkt,
		    HV_VMBUS_PACKET_TYPE_DATA_IN_BAND,
		    HV_VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	}

	return (ret);
}

/*
 * Net VSC on receive
 *
 * In the FreeBSD Hyper-V virtual world, this function deals exclusively
 * with virtual addresses.
 */
static void
hv_nv_on_receive(netvsc_dev *net_dev, struct hv_device *device,
    struct hv_vmbus_channel *chan, hv_vm_packet_descriptor *pkt)
{
	hv_vm_transfer_page_packet_header *vm_xfer_page_pkt;
	nvsp_msg *nvsp_msg_pkt;
	netvsc_packet vsc_pkt;
	netvsc_packet *net_vsc_pkt = &vsc_pkt;
	device_t dev = device->device;
	int count = 0;
	int i = 0;
	int status = nvsp_status_success;

	/*
	 * All inbound packets other than send completion should be
	 * xfer page packet.
	 */
	if (pkt->type != HV_VMBUS_PACKET_TYPE_DATA_USING_TRANSFER_PAGES) {
		device_printf(dev, "packet type %d is invalid!\n", pkt->type);
		return;
	}

	nvsp_msg_pkt = (nvsp_msg *)((unsigned long)pkt
		+ (pkt->data_offset8 << 3));

	/* Make sure this is a valid nvsp packet */
	if (nvsp_msg_pkt->hdr.msg_type != nvsp_msg_1_type_send_rndis_pkt) {
		device_printf(dev, "packet hdr type %d is invalid!\n",
		    pkt->type);
		return;
	}
	
	vm_xfer_page_pkt = (hv_vm_transfer_page_packet_header *)pkt;

	if (vm_xfer_page_pkt->transfer_page_set_id !=
	    NETVSC_RECEIVE_BUFFER_ID) {
		device_printf(dev, "transfer_page_set_id %d is invalid!\n",
		    vm_xfer_page_pkt->transfer_page_set_id);
		return;
	}

	count = vm_xfer_page_pkt->range_count;
	net_vsc_pkt->device = device;

	/* Each range represents 1 RNDIS pkt that contains 1 Ethernet frame */
	for (i = 0; i < count; i++) {
		net_vsc_pkt->status = nvsp_status_success;
		net_vsc_pkt->data = (void *)((unsigned long)net_dev->rx_buf +
		    vm_xfer_page_pkt->ranges[i].byte_offset);
		net_vsc_pkt->tot_data_buf_len = 
		    vm_xfer_page_pkt->ranges[i].byte_count;

		hv_rf_on_receive(net_dev, device, chan, net_vsc_pkt);
		if (net_vsc_pkt->status != nvsp_status_success) {
			status = nvsp_status_failure;
		}
	}
	
	/*
	 * Moved completion call back here so that all received 
	 * messages (not just data messages) will trigger a response
	 * message back to the host.
	 */
	hv_nv_on_receive_completion(chan, vm_xfer_page_pkt->d.transaction_id,
	    status);
}

/*
 * Net VSC on receive completion
 *
 * Send a receive completion packet to RNDIS device (ie NetVsp)
 */
static void
hv_nv_on_receive_completion(struct hv_vmbus_channel *chan, uint64_t tid,
    uint32_t status)
{
	nvsp_msg rx_comp_msg;
	int retries = 0;
	int ret = 0;
	
	rx_comp_msg.hdr.msg_type = nvsp_msg_1_type_send_rndis_pkt_complete;

	/* Pass in the status */
	rx_comp_msg.msgs.vers_1_msgs.send_rndis_pkt_complete.status =
	    status;

retry_send_cmplt:
	/* Send the completion */
	ret = hv_vmbus_channel_send_packet(chan, &rx_comp_msg,
	    sizeof(nvsp_msg), tid, HV_VMBUS_PACKET_TYPE_COMPLETION, 0);
	if (ret == 0) {
		/* success */
		/* no-op */
	} else if (ret == EAGAIN) {
		/* no more room... wait a bit and attempt to retry 3 times */
		retries++;

		if (retries < 4) {
			DELAY(100);
			goto retry_send_cmplt;
		}
	}
}

/*
 * Net VSC receiving vRSS send table from VSP
 */
static void
hv_nv_send_table(struct hv_device *device, hv_vm_packet_descriptor *pkt)
{
	netvsc_dev *net_dev;
	nvsp_msg *nvsp_msg_pkt;
	int i;
	uint32_t count, *table;

	net_dev = hv_nv_get_inbound_net_device(device);
	if (!net_dev)
        	return;

	nvsp_msg_pkt =
	    (nvsp_msg *)((unsigned long)pkt + (pkt->data_offset8 << 3));

	if (nvsp_msg_pkt->hdr.msg_type !=
	    nvsp_msg5_type_send_indirection_table) {
		printf("Netvsc: !Warning! receive msg type not "
			"send_indirection_table. type = %d\n",
			nvsp_msg_pkt->hdr.msg_type);
		return;
	}

	count = nvsp_msg_pkt->msgs.vers_5_msgs.send_table.count;
	if (count != VRSS_SEND_TABLE_SIZE) {
        	printf("Netvsc: Received wrong send table size: %u\n", count);
	        return;
	}

	table = (uint32_t *)
	    ((unsigned long)&nvsp_msg_pkt->msgs.vers_5_msgs.send_table +
	     nvsp_msg_pkt->msgs.vers_5_msgs.send_table.offset);

	for (i = 0; i < count; i++)
        	net_dev->vrss_send_table[i] = table[i];
}

/*
 * Net VSC on channel callback
 */
static void
hv_nv_on_channel_callback(void *xchan)
{
	struct hv_vmbus_channel *chan = xchan;
	struct hv_device *device = chan->device;
	netvsc_dev *net_dev;
	device_t dev = device->device;
	uint32_t bytes_rxed;
	uint64_t request_id;
 	hv_vm_packet_descriptor *desc;
	uint8_t *buffer;
	int bufferlen = NETVSC_PACKET_SIZE;
	int ret = 0;

	net_dev = hv_nv_get_inbound_net_device(device);
	if (net_dev == NULL)
		return;

	buffer = chan->hv_chan_rdbuf;

	do {
		ret = hv_vmbus_channel_recv_packet_raw(chan,
		    buffer, bufferlen, &bytes_rxed, &request_id);
		if (ret == 0) {
			if (bytes_rxed > 0) {
				desc = (hv_vm_packet_descriptor *)buffer;
				switch (desc->type) {
				case HV_VMBUS_PACKET_TYPE_COMPLETION:
					hv_nv_on_send_completion(net_dev, device,
					    chan, desc);
					break;
				case HV_VMBUS_PACKET_TYPE_DATA_USING_TRANSFER_PAGES:
					hv_nv_on_receive(net_dev, device, chan, desc);
					break;
				case HV_VMBUS_PACKET_TYPE_DATA_IN_BAND:
					hv_nv_send_table(device, desc);
					break;
				default:
					device_printf(dev,
					    "hv_cb recv unknow type %d "
					    " packet\n", desc->type);
					break;
				}
			} else {
				break;
			}
		} else if (ret == ENOBUFS) {
			/* Handle large packet */
			if (bufferlen > NETVSC_PACKET_SIZE) {
				free(buffer, M_NETVSC);
				buffer = NULL;
			}

			/* alloc new buffer */
			buffer = malloc(bytes_rxed, M_NETVSC, M_NOWAIT);
			if (buffer == NULL) {
				device_printf(dev,
				    "hv_cb malloc buffer failed, len=%u\n",
				    bytes_rxed);
				bufferlen = 0;
				break;
			}
			bufferlen = bytes_rxed;
		}
	} while (1);

	if (bufferlen > NETVSC_PACKET_SIZE)
		free(buffer, M_NETVSC);

	hv_rf_channel_rollup(chan);
}
