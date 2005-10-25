#include <stdio.h>

#include "vector.h"
#include "structs.h"
#include "config.h"
#include "debug.h"
#include "pgpolicies.h"
#include "alias.h"

#include "../libcheckers/checkers.h"

/*
 * selectors :
 * traverse the configuration layers from most specific to most generic
 * stop at first explicit setting found
 */
extern int
select_rr_weight (struct multipath * mp)
{
	if (mp->mpe && mp->mpe->rr_weight) {
		mp->rr_weight = mp->mpe->rr_weight;
		condlog(3, "rr_weight = %i (LUN setting)",
			mp->rr_weight);
		return 0;
	}
	if (mp->hwe && mp->hwe->rr_weight) {
		mp->rr_weight = mp->hwe->rr_weight;
		condlog(3, "rr_weight = %i (controler setting)",
			mp->rr_weight);
		return 0;
	}
	if (conf->rr_weight) {
		mp->rr_weight = conf->rr_weight;
		condlog(3, "rr_weight = %i (config file default)",
			mp->rr_weight);
		return 0;
	}
	mp->rr_weight = RR_WEIGHT_NONE;
	condlog(3, "rr_weight = %i (internal default)",
		mp->rr_weight);
	return 0;
}

extern int
select_pgfailback (struct multipath * mp)
{
	if (mp->mpe && mp->mpe->pgfailback != FAILBACK_UNDEF) {
		mp->pgfailback = mp->mpe->pgfailback;
		condlog(3, "pgfailback = %i (LUN setting)", mp->pgfailback);
		return 0;
	}
	if (mp->hwe && mp->hwe->pgfailback != FAILBACK_UNDEF) {
		mp->pgfailback = mp->hwe->pgfailback;
		condlog(3, "pgfailback = %i (controler setting)",
			mp->pgfailback);
		return 0;
	}
	if (conf->pgfailback != FAILBACK_UNDEF) {
		mp->pgfailback = conf->pgfailback;
		condlog(3, "pgfailback = %i (config file default)",
			mp->pgfailback);
		return 0;
	}
	mp->pgfailback = -FAILBACK_MANUAL;
	condlog(3, "pgfailover = %i (internal default)", mp->pgfailback);
	return 0;
}

extern int
select_pgpolicy (struct multipath * mp)
{
	struct path * pp;
	char pgpolicy_name[POLICY_NAME_SIZE];

	pp = VECTOR_SLOT(mp->paths, 0);

	if (conf->pgpolicy_flag > 0) {
		mp->pgpolicy = conf->pgpolicy_flag;
		get_pgpolicy_name(pgpolicy_name, mp->pgpolicy);
		condlog(3, "pgpolicy = %s (cmd line flag)", pgpolicy_name);
		return 0;
	}
	if (mp->mpe && mp->mpe->pgpolicy > 0) {
		mp->pgpolicy = mp->mpe->pgpolicy;
		get_pgpolicy_name(pgpolicy_name, mp->pgpolicy);
		condlog(3, "pgpolicy = %s (LUN setting)", pgpolicy_name);
		return 0;
	}
	if (mp->hwe && mp->hwe->pgpolicy > 0) {
		mp->pgpolicy = mp->hwe->pgpolicy;
		get_pgpolicy_name(pgpolicy_name, mp->pgpolicy);
		condlog(3, "pgpolicy = %s (controler setting)", pgpolicy_name);
		return 0;
	}
	if (conf->default_pgpolicy > 0) {
		mp->pgpolicy = conf->default_pgpolicy;
		get_pgpolicy_name(pgpolicy_name, mp->pgpolicy);
		condlog(3, "pgpolicy = %s (config file default)", pgpolicy_name);
		return 0;
	}
	mp->pgpolicy = FAILOVER;
	get_pgpolicy_name(pgpolicy_name, FAILOVER);
	condlog(3, "pgpolicy = %s (internal default)", pgpolicy_name);
	return 0;
}

extern int
select_selector (struct multipath * mp)
{
	if (mp->mpe && mp->mpe->selector) {
		mp->selector = mp->mpe->selector;
		condlog(3, "selector = %s (LUN setting)", mp->selector);
		return 0;
	}
	if (mp->hwe && mp->hwe->selector) {
		mp->selector = mp->hwe->selector;
		condlog(3, "selector = %s (controler setting)", mp->selector);
		return 0;
	}
	mp->selector = conf->selector;
	condlog(3, "selector = %s (internal default)", mp->selector);
	return 0;
}

extern int
select_alias (struct multipath * mp)
{
	if (mp->mpe && mp->mpe->alias)
		mp->alias = mp->mpe->alias;
	else {
		mp->alias = NULL;
		if (conf->user_friendly_names)
			mp->alias = get_user_friendly_alias(mp->wwid);
		if (mp->alias == NULL)
			mp->alias = mp->wwid;
	}

	return 0;
}

extern int
select_features (struct multipath * mp)
{
	if (mp->hwe && mp->hwe->features) {
		mp->features = mp->hwe->features;
		condlog(3, "features = %s (controler setting)", mp->features);
		return 0;
	}
	mp->features = conf->features;
	condlog(3, "features = %s (internal default)", mp->features);
	return 0;
}

extern int
select_hwhandler (struct multipath * mp)
{
	if (mp->hwe && mp->hwe->hwhandler) {
		mp->hwhandler = mp->hwe->hwhandler;
		condlog(3, "hwhandler = %s (controler setting)", mp->hwhandler);
		return 0;
	}
	mp->hwhandler = conf->default_hwhandler;
	condlog(3, "hwhandler = %s (internal default)", mp->hwhandler);
	return 0;
}

extern int
select_checkfn(struct path *pp)
{
	char checker_name[CHECKER_NAME_SIZE];

	if (pp->hwe && pp->hwe->checker_index > 0) {
		get_checker_name(checker_name, pp->hwe->checker_index);
		condlog(3, "path checker = %s (controler setting)", checker_name);
		pp->checkfn = get_checker_addr(pp->hwe->checker_index);
		return 0;
	}
	pp->checkfn = get_checker_addr(conf->default_checker_index);
	get_checker_name(checker_name, conf->default_checker_index);
	condlog(3, "path checker = %s (internal default)", checker_name);
	return 0;
}

extern int
select_getuid (struct path * pp)
{
	if (pp->hwe && pp->hwe->getuid) {
		pp->getuid = pp->hwe->getuid;
		condlog(3, "getuid = %s (controler setting)", pp->getuid);
		return 0;
	}
	pp->getuid = conf->default_getuid;
	condlog(3, "getuid = %s (internal default)", pp->getuid);
	return 0;
}

extern int
select_getprio (struct path * pp)
{
	if (pp->hwe && pp->hwe->getprio) {
		pp->getprio = pp->hwe->getprio;
		condlog(3, "getprio = %s (controler setting)", pp->getprio);
		return 0;
	}
	pp->getprio = conf->default_getprio;
	condlog(3, "getprio = %s (internal default)", pp->getprio);
	return 0;
}

extern int
select_no_path_retry(struct multipath *mp)
{
	if (mp->mpe && mp->mpe->no_path_retry != NO_PATH_RETRY_UNDEF) {
		mp->no_path_retry = mp->mpe->no_path_retry;
		condlog(3, "no_path_retry = %i (multipath setting)",
			mp->no_path_retry);
		return 0;
	}
	if (mp->hwe && mp->hwe->no_path_retry != NO_PATH_RETRY_UNDEF) {
		mp->no_path_retry = mp->hwe->no_path_retry;
		condlog(3, "no_path_retry = %i (controler setting)",
			mp->no_path_retry);
		return 0;
	}
	if (conf->no_path_retry != NO_PATH_RETRY_UNDEF) {
		mp->no_path_retry = conf->no_path_retry;
		condlog(3, "no_path_retry = %i (config file default)",
			mp->no_path_retry);
		return 0;
	}
	mp->no_path_retry = NO_PATH_RETRY_UNDEF;
	condlog(3, "no_path_retry = NONE (internal default)");
	return 0;
}
