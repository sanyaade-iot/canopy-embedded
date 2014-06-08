#include "canopy_boilerplate.h"
#include <canopy.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* returns true on success */
static bool set_gpio(int pin, int value)
{
    FILE *fp;
    char buf[1024];

    snprintf(buf, 1024, "/sys/class/gpio/gpio%d/value", pin);
    fp = fopen(buf, "w");
    if (!fp)
    {
        fprintf(stderr, "Failed to set value of gpio%d\n", pin);
        return false;
    }

    printf("writing %d to %s\n", value, buf);
    fprintf(fp, "%d", value);
    fclose(fp);
    return true;
}

static bool set_gpio_direction(int pin, const char *direction) 
{
    FILE *fp;
    char buf[1024];

    /* set GPIO pin direction */
    snprintf(
        buf, 
        1024, 
        "/sys/class/gpio/gpio%d/direction", 
        pin);
    fp = fopen(buf, "w");
    if (!fp)
    {
        fprintf(stderr, "Failed to set direction of gpio%d\n", pin);
        goto cleanup;
    }
    printf("writing %s to %s\n", direction, buf);
    fprintf(fp, direction);
    fclose(fp);
    return true;
cleanup:
    return false;
}


static bool init_gpio(int pin)
{
    FILE *fp;

    /* export GPIO pin */
    fp = fopen("/sys/class/gpio/export", "w");
    if (!fp)
    {
        fprintf(stderr, "Failed to export pin %d\n", pin);
        goto cleanup;
    }
    printf("writing %d to /sys/class/gpio/export\n", pin);
    fprintf(fp, "%d", pin);
    fclose(fp);

    return true;
cleanup:
    return false;
}

static bool on_change__speed(CanopyContext canopy, int8_t value)
{
    set_gpio(14, 1);
    set_gpio(15, 1);
    set_gpio(18, 1);
    switch (value)
    {
        case 1:
            /* slowest */
            set_gpio(18, 0);
            break;
        case 2:
            set_gpio(15, 0);
            break;
        case 3:
            /* fastest */
            set_gpio(14, 0);
            break;
    }
    return true;
}

static bool on_canopy_init(CanopyContext canopy)
{
    printf("starting...\n");
    init_gpio(14);
    set_gpio_direction(14, "in");
    init_gpio(15);
    set_gpio_direction(15, "in");
    init_gpio(18);
    set_gpio_direction(18, "in");
    set_gpio_direction(14, "out");
    set_gpio(14, 1);
    set_gpio_direction(15, "out");
    set_gpio(15, 1);
    set_gpio_direction(18, "out");
    set_gpio(18, 1);
    return true;
}

static bool on_canopy_shutdown(CanopyContext canopy)
{
    return false;
}

static bool on_connected(CanopyContext canopy)
{
    return false;
}

static bool on_disconnected(CanopyContext canopy)
{
    return false;
}

static bool on_report_requested(CanopyContext canopy)
{
    CanopyReport report = canopy_begin_report(canopy);
    canopy_send_report(report);
    return true;
}

