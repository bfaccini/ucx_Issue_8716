/**
* Copyright (c) NVIDIA CORPORATION & AFFILIATES, 2022. ALL RIGHTS RESERVED.
*
* See file LICENSE for terms.
*/

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "proto_rndv.inl"


static ucs_status_t
ucp_proto_rndv_ats_init(const ucp_proto_init_params_t *init_params)
{
    ucp_proto_perf_range_t *range0;
    ucp_proto_caps_t caps;
    ucs_status_t status;

    if (ucp_proto_rndv_init_params_is_ppln_frag(init_params)) {
        return UCS_ERR_UNSUPPORTED;
    }

    *init_params->priv_size = sizeof(ucp_proto_rndv_ack_priv_t);

    caps.cfg_thresh   = 0;
    caps.cfg_priority = 1;
    caps.min_length   = 0;
    caps.num_ranges   = 1;
    range0            = &caps.ranges[0];
    range0->node      = NULL;
    ucp_proto_perf_set(range0->perf, UCS_LINEAR_FUNC_ZERO);

    /* This protocols supports either a regular rendezvous receive but without
     * data, or a rendezvous receive which should ignore the data.
     * In either case, we just need to send an ATS.
     */
    if (init_params->select_param->op_id == UCP_OP_ID_RNDV_RECV) {
        range0->max_length = 0;
    } else if (init_params->select_param->op_id == UCP_OP_ID_RNDV_RECV_DROP) {
        range0->max_length = SIZE_MAX;
    } else {
        return UCS_ERR_UNSUPPORTED;
    }

    status = ucp_proto_rndv_ack_init(init_params, UCP_PROTO_RNDV_ATS_NAME,
                                     &caps, UCS_LINEAR_FUNC_ZERO,
                                     init_params->priv);
    ucp_proto_select_caps_cleanup(&caps);

    return status;
}

ucp_proto_t ucp_rndv_ats_proto = {
    .name     = "rndv/ats",
    .desc     = "no data fetch",
    .flags    = 0,
    .init     = ucp_proto_rndv_ats_init,
    .query    = ucp_proto_default_query,
    .progress = {ucp_proto_rndv_ats_progress},
    .abort    = ucp_proto_abort_fatal_not_implemented,
    .reset    = (ucp_request_reset_func_t)ucs_empty_function
};
