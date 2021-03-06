#include <canopy.h>
#include <red_test.h>
#include <stdio.h>

// Just tests local copy of cloud variable.  Doesn't "sync" w/ server.
int main(int argc, const char *argv[])
{
    CanopyContext canopy;
    CanopyResultEnum result;
    RedTest test;

    test = RedTest_Begin(argv[0], NULL, NULL);

    canopy = canopy_init_context();
    RedTest_Verify(test, "Canopy init", canopy);

    result = canopy_set_opt(canopy,
        CANOPY_CLOUD_SERVER, "dev02.canopy.link",
        CANOPY_DEVICE_UUID, "c31a8ced-b9f1-4b0c-afe9-1afed3b0c21f",
        CANOPY_SYNC_BLOCKING, true,
        CANOPY_SYNC_TIMEOUT_MS, 10000,
        CANOPY_VAR_SEND_PROTOCOL, CANOPY_PROTOCOL_WS,
        CANOPY_VAR_RECV_PROTOCOL, CANOPY_PROTOCOL_WS
    );
    RedTest_Verify(test, "Configure canopy options", result == CANOPY_SUCCESS);

   /* result = canopy_var_config(canopy, "in float32 temp");
    result = canopy_var_config(canopy, "out tuple gps",
            CANOPY_TUPLE_MEMBER("float32 longitude", CANOPY_MIN_VALUE, 0.5f),
            CANOPY_TUPLE_MEMBER("float32 latitude", CANOPY_MIN_VALUE, 0.5f)
    );*/

    result = canopy_var_init(canopy, "inout float32 dimlevel"
            /*CANOPY_VAR_DESCRIPTION, "Dimmer level",
            CANOPY_VAR_MIN_VALUE, 1,
            CANOPY_VAR_MAX_VALUE, 100,*/
    );
    RedTest_Verify(test, "Configure variable \"dimlevel\"", result == CANOPY_SUCCESS);

    result = canopy_sync(canopy, NULL);

    result = canopy_shutdown_context(canopy);
    RedTest_Verify(test, "Shutdown", result == CANOPY_SUCCESS);

    return RedTest_End(test);
}

/*
 *


int main(void)
{
    CanopyContext ctx;

    ctx = canopy_init_context();
    canopy_set_opt(ctx, 
        CANOPY_CLOUD_SERVER, "dev02.canopy.link",
        CANOPY_DEVICE_UUID, "c31a8ced-b9f1-4b0c-afe9-1afed3b0c21f",
    );

    canopy_init_var(ctx, "in float32 mode");
    canopy_init_var(ctx, "out float32 temperature");
    canopy_init_var(ctx, "out tuple gps", 
        CANOPY_INIT_TUPLE_MEMBER("float32 latitude"),
        CANOPY_INIT_TUPLE_MEMBER("float32 longitude"),
        CANOPY_INIT_TUPLE_MEMBER("float32 altitude"),
    );
canopy_var_set_float32(ctx, "temperature", 10.0f);

    canopy_sync(ctx, NULL);

    if (canopy_var_get_float32(ctx, "mode", &mode) == CANOPY_SUCCESS)
    {
        printf("mode: %f", mode);
    }
}
*/
