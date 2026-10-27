"""Port Following Platinum's follower spawn exclusions (fp_023c84ac) into field_map_change.c and the renderer."""
import os

os.chdir('C:/projects/pokemon-platinum-dexnav/pkplat-dexnav')


def edit(p, old, new):
    s = open(p, encoding='utf-8').read()
    assert s.count(old) == 1, (p, old[:60], s.count(old))
    open(p, 'w', encoding='utf-8', newline='\n').write(s.replace(old, new))


CAN_SPAWN = '''// Following Platinum's spawn rules (fp_023c84ac): no follower on link/gimmick maps or in the
// Distortion World, none in elevators, and no oversized Pokemon inside buildings.
static const u16 sFollowerExcludedMaps[] = {
    MAP_HEADER_UNDERGROUND,
    MAP_HEADER_GLOBAL_TERMINAL_1F,
    MAP_HEADER_GLOBAL_TERMINAL_2F,
    MAP_HEADER_GLOBAL_TERMINAL_3F,
    MAP_HEADER_CANALAVE_CITY_GYM,
    MAP_HEADER_PASTORIA_CITY_GYM,
    MAP_HEADER_SUNYSHORE_CITY_GYM_ROOM_1,
    MAP_HEADER_SUNYSHORE_CITY_GYM_ROOM_2,
    MAP_HEADER_SUNYSHORE_CITY_GYM_ROOM_3,
    MAP_HEADER_SNOWPOINT_CITY_GYM,
    MAP_HEADER_FUEGO_IRONWORKS_BUILDING,
    MAP_HEADER_COMMUNICATION_CLUB_COLOSSEUM_2P,
    MAP_HEADER_COMMUNICATION_CLUB_COLOSSEUM_4P,
    MAP_HEADER_RECORD_MIXING_ROOM,
    MAP_HEADER_UNION_ROOM,
};

static const u16 sFollowerTooBigIndoors[] = {
    SPECIES_STEELIX,
    SPECIES_LUGIA,
    SPECIES_HO_OH,
    SPECIES_WAILORD,
    SPECIES_KYOGRE,
    SPECIES_GROUDON,
    SPECIES_RAYQUAZA,
    SPECIES_DIALGA,
    SPECIES_PALKIA,
    SPECIES_REGIGIGAS,
    SPECIES_GIRATINA,
    SPECIES_ARCEUS,
};

#define FOLLOWER_ELEVATOR_MAP_MATRIX 207

BOOL PokemonFollower_CanSpawn(int species, enum MapHeaderID mapHeaderID)
{
    int i;

    for (i = 0; i < NELEMS(sFollowerExcludedMaps); i++) {
        if (mapHeaderID == sFollowerExcludedMaps[i]) {
            return FALSE;
        }
    }

    if (mapHeaderID >= MAP_HEADER_DISTORTION_WORLD_1F && mapHeaderID <= MAP_HEADER_DISTORTION_WORLD_TURNBACK_CAVE_ROOM) {
        return FALSE;
    }

    if (MapHeader_GetMapMatrixID(mapHeaderID) == FOLLOWER_ELEVATOR_MAP_MATRIX) {
        return FALSE;
    }

    if (MapHeader_IsBuilding(mapHeaderID)) {
        for (i = 0; i < NELEMS(sFollowerTooBigIndoors); i++) {
            if (species == sFollowerTooBigIndoors[i]) {
                return FALSE;
            }
        }
    }

    return TRUE;
}

static void FieldMapChange_CreatePokemonFollower(FieldSystem *fieldSystem)'''

p = 'src/field_map_change.c'
edit(p, 'static void FieldMapChange_CreatePokemonFollower(FieldSystem *fieldSystem)', CAN_SPAWN)
edit(p, '''    int graphicsID = PokemonFollower_GraphicsID(lead);

    if (graphicsID < 0) {
        return;
    }
''', '''    int graphicsID = PokemonFollower_GraphicsID(lead);

    if (graphicsID < 0 || !PokemonFollower_CanSpawn(species, fieldSystem->location->mapHeaderID)) {
        return;
    }
''')

edit('include/field_map_change.h', 'int PokemonFollower_GraphicsID(Pokemon *mon);',
     'int PokemonFollower_GraphicsID(Pokemon *mon);\nBOOL PokemonFollower_CanSpawn(int species, enum MapHeaderID mapHeaderID);')

# renderer refresh: a new lead that isn't allowed here hides the follower instead of showing it
edit('src/overlay005/ov5_021ECA70.c', '''    if (MapObject_GetLocalID(mapObj) == FOLLOWER_LOCAL_ID) {
        Pokemon *lead = PokemonFollower_GetLead(MapObject_FieldSystem(mapObj)->saveData);

        if (lead != NULL) {''', '''    if (MapObject_GetLocalID(mapObj) == FOLLOWER_LOCAL_ID) {
        FieldSystem *fieldSystem = MapObject_FieldSystem(mapObj);
        Pokemon *lead = PokemonFollower_GetLead(fieldSystem->saveData);

        if (lead != NULL) {
            int species = Pokemon_GetValue(lead, MON_DATA_SPECIES, NULL);

            MapObject_SetHidden(mapObj, !PokemonFollower_CanSpawn(species, fieldSystem->location->mapHeaderID));
''')
print('patched')
