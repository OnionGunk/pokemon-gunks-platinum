#include "location.h"

#include "generated/map_headers.h"

#include "field_overworld_state.h"
#include "savedata.h"

#ifdef TEST_SKIP_INTRO
// test builds start on a route with wild encounters
static const Location sPlayerStartLocation = {
    .mapHeaderID = MAP_HEADER_ROUTE_201,
    .warpId = WARP_ID_NONE,
    .x = 125,
    .z = 850,
    .faceDirection = FACE_UP,
};
#else
static const Location sPlayerStartLocation = {
    .mapHeaderID = MAP_HEADER_TWINLEAF_TOWN_PLAYER_HOUSE_2F,
    .warpId = WARP_ID_NONE,
    .x = 4,
    .z = 6,
    .faceDirection = FACE_UP,
};
#endif

static const Location sPlayerFirstRespawnLocation = {
    .mapHeaderID = MAP_HEADER_TWINLEAF_TOWN,
    .warpId = WARP_ID_NONE,
    .x = 116,
    .z = 886,
    .faceDirection = FACE_DOWN,
};

void SetPlayerStartLocation(Location *outLocation)
{
    *outLocation = sPlayerStartLocation;
}

void SetPlayerFirstRespawnLocation(Location *outLocation)
{
    *outLocation = sPlayerFirstRespawnLocation;
}

void InitPlayerStartLocation(SaveData *saveData)
{
    SetPlayerStartLocation(FieldOverworldState_GetPlayerLocation(SaveData_GetFieldOverworldState(saveData)));
}
