#include "app/IncubatorApp.h"

extern IncubatorApp g_app;

void ui_factory_reset()
{
    g_app.requestFactoryReset();
}

void ui_provisioning_reset()
{
    g_app.requestProvisioningReset();
}

void ui_time_sync()
{
    g_app.requestTimeSync();
}