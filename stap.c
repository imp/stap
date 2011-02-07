/*
 * This file and its contents are supplied under the terms of the
 * Common Development and Distribution License ("CDDL"), version 1.0.
 * You may only use this file in accordance with the terms of version
 * 1.0 of the CDDL.
 *
 * A full copy of the text of the CDDL should have accompanied this
 * source.  A copy of the CDDL is also available via the Internet at
 * http://www.illumos.org/license/CDDL.
 */

/*
 * Copyright 2011 Grigale Ltd.  All rights reserved.
 */

/*
 * Solaris GLDv3 tap driver
 */

#include <sys/types.h>
#include <sys/cmn_err.h>
#include <sys/note.h>
#include <sys/conf.h>
#include <sys/devops.h>
#include <sys/modctl.h>
#include <sys/mac.h>
#include <sys/mac_provider.h>
#include <sys/mac_ether.h>
#include <sys/ddi.h>
#include <sys/sunddi.h>

#include "stap.h"


typedef struct {
	dev_info_t	*dip;
	mac_handle_t	mh;
	uint8_t		addr[8];
} stap_state_t;

static void *stap_statep;

/*
 * MAC callbacks
 */
static int
stap_getstat(void *arg, uint_t stat, uint64_t *stat_value)
{
	stap_state_t	*sp = arg;

	return (0);
}


static int
stap_start(void *arg)
{
	stap_state_t	*sp = arg;

	return (0);
}

static void
stap_stop(void *arg)
{
	stap_state_t	*sp = arg;

}

static int
stap_setpromisc(void *arg, boolean_t promisc_mode)
{
	stap_state_t	*sp = arg;

	return (0);
}

static int
stap_multicst(void *arg, boolean_t add, const uint8_t *mcast_addr)
{
	stap_state_t	*sp = arg;

	return (0);
}

static int
stap_unicst(void *arg, const uint8_t *ucast_addr)
{
	stap_state_t	*sp = arg;

	return (0);
}

static mblk_t *
stap_tx(void *arg, mblk_t *mp_chain)
{
	stap_state_t	*sp = arg;

	return (NULL);
}


static void
stap_ioctl(void *arg, queue_t *q, mblk_t *mp)
{
	stap_state_t	*sp = arg;

}

static boolean_t
stap_getcapab(void *arg, mac_capab_t cap, void *cap_data)
{
	stap_state_t	*sp = arg;

	return (B_TRUE);
}

static int
stap_setprop(void *arg, const char *prop_name, mac_prop_id_t prop_id,
	uint_t prop_val_size, const void *prop_val)
{
	stap_state_t	*sp = arg;

	return (0);
}

static int
stap_getprop(void *arg, const char *prop_name, mac_prop_id_t prop_id,
	uint_t prop_val_size, void *prop_val)
{
	stap_state_t	*sp = arg;

	return (0);
}

static void
stap_propinfo(void *arg, const char *prop_name, mac_prop_id_t prop_id,
	mac_prop_info_handle_t prop_handle)
{
	stap_state_t	*sp = arg;

}

#define	STAP_CALLBACKS	(MC_IOCTL | MC_GETCAPAB | MC_PROPERTIES)

static mac_callbacks_t stap_mac_callbacks = {
	.mc_callbacks	= STAP_CALLBACKS,
	.mc_getstat	= stap_getstat,
	.mc_start	= stap_start,
	.mc_stop	= stap_stop,
	.mc_setpromisc	= stap_setpromisc,
	.mc_multicst	= stap_multicst,
	.mc_unicst	= stap_unicst,
	.mc_tx		= stap_tx,
	.mc_ioctl	= stap_ioctl,
	.mc_getcapab	= stap_getcapab,
	.mc_setprop	= stap_setprop,
	.mc_getprop	= stap_getprop,
	.mc_propinfo	= stap_propinfo
};

static int
stap_attach(dev_info_t *dip, ddi_attach_cmd_t cmd)
{
	stap_state_t	*sp;
	mac_register_t	*mp;
	int		instance;
	int		rc;

	switch (cmd) {
	case DDI_ATTACH:
		break;
	case DDI_RESUME:
	default:
		return (DDI_FAILURE);
	}

	instance = ddi_get_instance(dip);
	if (ddi_soft_state_zalloc(stap_statep, instance) != DDI_SUCCESS) {
		return (DDI_FAILURE);
	}

	sp = ddi_get_soft_state(stap_statep, instance);
	ASSERT(sp);
	sp->dip = dip;

	mp = mac_alloc(MAC_VERSION);
	if (mp == NULL) {
		ddi_soft_state_free(stap_statep, instance);
		return (DDI_FAILURE);
	}

	mp->m_type_ident	= MAC_PLUGIN_IDENT_ETHER;
	mp->m_driver		= sp;
	mp->m_dip		= sp->dip;
	mp->m_instance		= 0;
	mp->m_src_addr		= sp->addr;
	mp->m_dst_addr		= NULL;
	mp->m_callbacks		= &stap_mac_callbacks;
	mp->m_min_sdu		= 0;
	mp->m_max_sdu		= ETHERMTU;
	mp->m_margin		= VLAN_TAGSZ;

	rc = mac_register(mp, &sp->mh);
	mac_free(mp);
	if (rc != 0) {
		ddi_soft_state_free(stap_statep, instance);
		return (DDI_FAILURE);
	}

/*
	rc = ddi_create_minor_node(dip, "ctl", S_IFCHR, instance, NT_NET, 0);
*/

	if (rc != DDI_SUCCESS) {
		cmn_err(CE_WARN, "stap_attach: failed to create minor node");
		mac_unregister(sp->mh);
		ddi_soft_state_free(stap_statep, instance);
		return (DDI_FAILURE);
	}

	ddi_report_dev(dip);

	return (DDI_SUCCESS);
}


static int
stap_detach(dev_info_t *dip, ddi_detach_cmd_t cmd)
{
	stap_state_t	*sp;
	int		instance;

	switch (cmd) {
	case DDI_DETACH:
		break;
	case DDI_SUSPEND:
	default:
		return (DDI_FAILURE);
	}

	instance = ddi_get_instance(dip);
	sp = ddi_get_soft_state(stap_statep, instance);

	ASSERT(sp);

	ddi_remove_minor_node(dip, 0);
	mac_unregister(sp->mh);
	ddi_soft_state_free(stap_statep, instance);

	return (DDI_SUCCESS);
}


static struct dev_ops stap_devops = {
	.devo_rev	= DEVO_REV,
	.devo_refcnt	= 0,
	.devo_getinfo	= NULL,
	.devo_identify	= nulldev,
	.devo_probe	= nulldev,
	.devo_attach	= stap_attach,
	.devo_detach	= stap_detach,
	.devo_reset	= nodev,
	.devo_cb_ops	= NULL,
	.devo_bus_ops	= NULL,
	.devo_power	= NULL,
	.devo_quiesce	= ddi_quiesce_not_supported
};


static struct modldrv stap_modldrv = {
	.drv_modops	= &mod_driverops,
	.drv_linkinfo	= "Solaris GLDv3 tap driver v0",
	.drv_dev_ops	= &stap_devops
};

static struct modlinkage stap_modlinkage = {
	.ml_rev		= MODREV_1,
	.ml_linkage	= {&stap_modldrv, NULL, NULL, NULL}
};


/*
 * Loadable module entry points.
 */
int
_init(void)
{
	int error;

#ifdef	STAP_SOFT_STATE
	error = ddi_soft_state_init(&stap_statep, sizeof (stap_state_t), 0);
	if (error != 0) {
		return (error);
	}
#endif
	mac_init_ops(&stap_devops, "stap");
	error = mod_install(&stap_modlinkage);
	if (error != 0) {
		mac_fini_ops(&stap_devops);
#ifdef	STAP_SOFT_STATE
		ddi_soft_state_fini(&stap_statep);
#endif
	}
	return (error);
}

int
_fini(void)
{
	int error;

	error = mod_remove(&stap_modlinkage);
	if (error == 0) {
		mac_fini_ops(&stap_devops);
#ifdef	STAP_SOFT_STATE
		ddi_soft_state_fini(&stap_statep);
#endif
	}
	return (error);
}

int
_info(struct modinfo *modinfop)
{
	return (mod_info(&stap_modlinkage, modinfop));
}
