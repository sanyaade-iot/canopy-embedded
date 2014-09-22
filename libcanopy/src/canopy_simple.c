/*
 * Copyright 2014 SimpleThings, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../include/canopy.h"
#include "red_string.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 *
 * COMPILE WITH:
 * gcc -Isrc -Iinclude -I3rdparty/libred/include -I3rdparty/libred/under_construction -c src/canopy_simple.c -Wall -Werror
 */

static CanopyCtx gCtx=NULL;

typedef struct
{
} CanopyPromise_t;

typedef struct
{
} CanopyResult_t;

/*
 * _ConfigOpt_t
 *
 *  Represents a set of configuration options.
 */
typedef struct _ConfigOpts_t
{
    bool has_cloud_server;
    char *cloud_server;

    bool has_control_protocol;
    CanopyProtocolEnum control_protocol;

    bool has_device_uuid;
    char *device_uuid;

    bool has_notify_protocol;
    CanopyProtocolEnum notify_protocol;

    bool has_notify_type;
    CanopyNotifyTypeEnum notify_type;

    bool has_property_name;
    char *property_name;

    bool has_report_protocol;
    CanopyProtocolEnum report_protocol;
} _ConfigOpts_t;

typedef struct CanopyCtx_t
{
    _ConfigOpts_t opts;
} CanopyCtx_t;

typedef struct _ConfigOpts_t * _ConfigOpts;

static void _config_opts_extend(_ConfigOpts dest, _ConfigOpts base, _ConfigOpts override);

/*
 * _config_opts_defaults
 *
 *  Set a configuration set to system defaults.
 */
static void _config_opts_defaults(_ConfigOpts dest)
{
    dest->has_cloud_server = true;
    dest->cloud_server = RedString_strdup("canopy.link");

    dest->has_control_protocol = true;
    dest->control_protocol = CANOPY_PROTOCOL_WS;

    dest->has_device_uuid = false;
    dest->device_uuid = NULL;

    dest->has_notify_protocol = true;
    dest->notify_protocol = CANOPY_PROTOCOL_HTTP;

    dest->has_notify_type = true;
    dest->notify_type = CANOPY_NOTIFY_MED_PRIORITY;

    dest->has_property_name = false;
    dest->property_name = NULL;

    dest->has_report_protocol = true;
    dest->report_protocol = CANOPY_PROTOCOL_HTTP;
}


/*
 * _new_config_opts_default
 *
 *  Create a new configuration set, using system default settings.
 */
#if 0
static _ConfigOpts _new_config_opts_default()
{
    _ConfigOpts opts;

    opts = calloc(1, sizeof(struct _ConfigOpts_t));
    if (!opts)
    {
        return NULL;
    }

    _config_opts_defaults(opts);

    return opts;
}
#endif

/*
 * _new_config_opts_empty
 *
 *  Create a new configuration set that does not have any options set.
 */
static _ConfigOpts _new_config_opts_empty()
{
    _ConfigOpts opts;
    opts = calloc(1, sizeof(struct _ConfigOpts_t));
    return opts;
}

static void _copy_config_opts(_ConfigOpts dest, _ConfigOpts src)
{
    /* TODO: duplicate strings */
    memcpy(dest, src, sizeof(struct _ConfigOpts_t));
}

/*
 * _config_opts_extend_va
 *
 *  Update configuration options using an argument list.
 */
static CanopyResultEnum _config_opts_extend_va(_ConfigOpts opts, va_list ap)
{
    _ConfigOpts new_opts = _new_config_opts_empty();
    bool done;

    if (!new_opts)
    {
        return CANOPY_ERROR_OUT_OF_MEMORY;
    }
    while (!done)
    {
        CanopyOptEnum param;
        param = va_arg(ap, CanopyOptEnum);
        /* TODO: free old strings */

        switch (param)
        {
            case CANOPY_CLOUD_SERVER:
            {
                new_opts->has_cloud_server = true;
                new_opts->cloud_server = va_arg(ap, char *);
                break;
            }
            case CANOPY_CONTROL_PROTOCOL:
            {
                new_opts->has_control_protocol = true;
                new_opts->control_protocol = va_arg(ap, CanopyProtocolEnum);
                break;
            }
            case CANOPY_DEVICE_UUID:
            {
                new_opts->has_device_uuid = true;
                new_opts->device_uuid = va_arg(ap, char *);
                break;
            }
            case CANOPY_NOTIFY_PROTOCOL:
            {
                new_opts->has_notify_protocol = true;
                new_opts->notify_protocol = va_arg(ap, CanopyProtocolEnum);
                break;
            }
            case CANOPY_NOTIFY_TYPE:
            {
                new_opts->has_notify_type = true;
                new_opts->notify_type = va_arg(ap, CanopyNotifyTypeEnum);
                break;
            }
            case CANOPY_PROPERTY_NAME:
            {
                new_opts->has_property_name = true;
                new_opts->property_name = va_arg(ap, char *);
                break;
            }
            case CANOPY_REPORT_PROTOCOL:
            {
                new_opts->has_report_protocol = true;
                new_opts->report_protocol = va_arg(ap, CanopyProtocolEnum);
                break;
            }
            case CANOPY_OPT_LIST_END:
            {
                done = true;
                break;
            }
            default:
            {
                /* Invalid parameter.  Throw error. */
                return CANOPY_ERROR_INVALID_OPT;
            }
        }
    }
    va_end(ap);

    /* No errors.  Extend the original */
    _config_opts_extend(opts, opts, new_opts);

    free(new_opts);
    return CANOPY_SUCCESS;
}

/*
 * _new_config_opts_va
 *
 *  Create a new configuration set, using supplied argument list.
 */
#if 0
static _ConfigOpts _new_config_opts_va(va_list ap)
{
    _ConfigOpts opts = _new_config_opts_empty();
    if (!opts)
    {
        return NULL;
    }

    _config_opts_extend_va(opts, ap);
    return opts;
}
#endif

/*
 * _config_opts_extend
 *
 *  Merge two configuration sets.  Starts with configuration options specified
 *  in <base>, and overrides with any options specified in <override>.  Stores
 *  result in <dest>, which may be equal to <base> or <override> or another
 *  ConfigOpt object.
 */
static void _config_opts_extend(_ConfigOpts dest, _ConfigOpts base, _ConfigOpts override)
{
    /* TODO: Free memory when strings get overridden */

    dest->cloud_server = override->has_cloud_server ? override->cloud_server : base->cloud_server;
    dest->has_cloud_server = base->has_cloud_server || override->has_cloud_server;

    dest->control_protocol = override->has_control_protocol ? override->control_protocol : base->control_protocol;
    dest->has_control_protocol = base->control_protocol || override->control_protocol;

    dest->device_uuid = override->has_device_uuid ? override->device_uuid : base->device_uuid;
    dest->has_device_uuid = base->device_uuid || override->device_uuid;

    dest->notify_protocol = override->has_notify_protocol ? override->notify_protocol : base->notify_protocol;
    dest->has_notify_protocol = base->notify_protocol || override->notify_protocol;

    dest->notify_type = override->has_notify_type ? override->notify_type : base->notify_type;
    dest->has_notify_type = base->notify_type || override->notify_type;

    dest->property_name = override->has_property_name ? override->property_name : base->property_name;
    dest->has_property_name = base->property_name || override->property_name;

    dest->report_protocol = override->has_report_protocol ? override->report_protocol : base->report_protocol;
    dest->has_report_protocol = base->report_protocol || override->report_protocol;
}

void _init_libcanopy_if_needed()
{
    /* Create global ctx */
    gCtx = canopy_create_ctx(NULL);
}

CanopyCtx canopy_create_ctx(CanopyCtx copyOptsFrom)
{
    CanopyCtx ctx;

    ctx = calloc(1, sizeof(struct CanopyCtx_t));
    if (!ctx)
    {
        return NULL;
    }

    if (copyOptsFrom == NULL)
    {
        _config_opts_defaults(&ctx->opts);
    }
    else
    {
        _copy_config_opts(&ctx->opts, &copyOptsFrom->opts);
    }

    return ctx;
}

CanopyResultEnum canopy_ctx_opt_impl(CanopyCtx ctx, ...)
{
    va_list ap;
    CanopyResultEnum result;
    _init_libcanopy_if_needed();

    va_start(ap, ctx);
    result = _config_opts_extend_va(&ctx->opts, ap);
    va_end(ap);

    return result;
}

CanopyCtx canopy_global_ctx()
{
    _init_libcanopy_if_needed();
    return gCtx;
}

CanopyResultEnum canopy_post_sample_impl(void * start, ...)
{
    _init_libcanopy_if_needed();
    return CANOPY_ERROR_NOT_IMPLEMENTED;

}

CanopyResultEnum canopy_promise_on_done(
        CanopyPromise promise, 
        CanopyResultCallback cb)
{

    return CANOPY_ERROR_NOT_IMPLEMENTED;
    /* TODO: Implement */
}

CanopyResultEnum canopy_promise_on_failure(
        CanopyPromise promise, 
        CanopyResultCallback cb)
{
    /* TODO: Implement */
    return CANOPY_ERROR_NOT_IMPLEMENTED;
}

CanopyResultEnum canopy_promise_on_success(
        CanopyPromise promise, 
        CanopyResultCallback cb)
{
    /* TODO: Implement */
    return CANOPY_ERROR_NOT_IMPLEMENTED;
}

CanopyResultEnum canopy_promise_result(CanopyPromise promise)
{
    /* TODO: Implement */
    return CANOPY_ERROR_NOT_IMPLEMENTED;
}

CanopyResultEnum canopy_promise_wait(CanopyPromise promise, ...)
{
    /* TODO: Implement */
    return CANOPY_ERROR_NOT_IMPLEMENTED;
}

CanopyResultEnum canopy_run_event_loop_impl(void *start, ...)
{
    /* TODO: Implement */
    return CANOPY_ERROR_NOT_IMPLEMENTED;
}

CanopyResultEnum canopy_service_impl(void *start, ...)
{
    /* TODO: Implement */
    return CANOPY_ERROR_NOT_IMPLEMENTED;
}
