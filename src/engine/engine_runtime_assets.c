#include "engine_runtime.h"

#include <stddef.h>
#include <string.h>

static const EngineRuntimeKnownAsset known_assets[] = 
{
    {
      "Supply Crate",
      "assets/models/supply_crate/supply_crate.gltf",
    },
    {
      "Outpost Scene",
      "assets/models/outpost_kit/outpost_scene.gltf",
    },
    {
      "Outpost Canister",
      "assets/models/outpost_kit/canister.gltf",
    },
    {
      "Outpost Power Unit",
      "assets/models/outpost_kit/power_unit.gltf",
    },
    {
      "Outpost Storage Locker",
      "assets/models/outpost_kit/storage_locker.gltf",
    },
    {
      "Outpost Workbench",
      "assets/models/outpost_kit/workbench.gltf",
    },
};

size_t engine_runtime_get_known_asset_count(void)
{
    return sizeof(known_assets) / sizeof(known_assets[0]);
}

bool engine_runtime_get_known_asset(
    size_t index,
    EngineRuntimeKnownAsset *out_asset
)
{
    if (out_asset == NULL || index >= engine_runtime_get_known_asset_count())
    {
        return false;
    }

    *out_asset = known_assets[index];

    return true;
}

bool engine_runtime_is_known_asset_path(const char *model_path)
{
    if (model_path == NULL)
    {
        return false;
    }

    for (size_t index = 0;
        index < engine_runtime_get_known_asset_count();
        index++ )
    {
        if (strcmp(known_assets[index].model_path, model_path) == 0)
        {
            return true;
        }
    }

    return false;
}
