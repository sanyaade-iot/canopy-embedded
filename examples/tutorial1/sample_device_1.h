/*
 * THIS FILE WAS AUTOGENERATED BY CANOGEN.
 * MODIFICATIONS WILL BE LOST IF YOU RUN CANOGEN AGAIN.
 */
#ifndef CANOPY_BOILERPLATE_INCLUDED
#define CANOPY_BOILERPLATE_INCLUDED

#include <canopy.h>
#include <stdbool.h>

#define CANOPY_CLOUD_HOST "sandbox.canopy.link"
#define CANOPY_CLOUD_PORT 8080

static bool on_canopy_init(CanopyContext canopy);
static bool on_canopy_shutdown(CanopyContext canopy);
static bool on_connected(CanopyContext canopy);
static bool on_disconnected(CanopyContext canopy);
static bool on_report_requested(CanopyContext canopy);
static void dispatch(CanopyEventDetails event);


#define SDDL_FILENAME "tutorial.sddl"
#define SDDL_CLASSNAME "canopy.tutorial.sample_device_1"
static bool on_trigger__reboot(CanopyContext canopy);
static void dispatch(CanopyEventDetails event)
{
    CanopyContext ctx = canopy_event_context(event);
    if (canopy_event_control_name_matches(event, "reboot"))
    {
        on_trigger__reboot(ctx);
    }
}


static bool handle_canopy_event(CanopyContext ctx, CanopyEventDetails event)
{
    switch (canopy_get_event_type(event))
    {
        case CANOPY_EVENT_CONNECTION_ESTABLISHED:
        {
            on_connected(ctx);
            break;
        }
        case CANOPY_EVENT_CONNECTION_LOST:
        {
            on_disconnected(ctx);
            break;
        }
        case CANOPY_EVENT_REPORT_REQUESTED:
        {
            on_report_requested(ctx);
            break;
        }
        case CANOPY_EVENT_CONTROL_TRIGGER:
        {
            dispatch(event);
        }
        default:
        {
            break;
        }
    }
    return true;
}

int main(int argc, const char *argv[])
{
    CanopyContext ctx;

    ctx = canopy_init();
    if (!ctx)
    {
        fprintf(stderr, "Failed to initialize canopy\n");
        return false;
    }
    on_canopy_init(ctx);
    canopy_load_sddl(ctx, SDDL_FILENAME, SDDL_CLASSNAME);

    canopy_register_event_callback(ctx, handle_canopy_event, NULL);

    canopy_set_auto_reconnect(ctx, true);
    canopy_connect(ctx);

    canopy_event_loop(ctx);
    on_canopy_shutdown(ctx);
    canopy_shutdown(ctx);
    return 0;
}

#endif

