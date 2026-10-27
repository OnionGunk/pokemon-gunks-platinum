"""Keep the follower hidden after a map load until it has stepped off the arrival tile (doors, warps)."""
import os

os.chdir('C:/src/pkplat-dexnav')


def edit(p, old, new):
    s = open(p, encoding='utf-8').read()
    assert s.count(old) == 1, (p, old[:60], s.count(old))
    open(p, 'w', encoding='utf-8', newline='\n').write(s.replace(old, new))


# shared follower data layout
edit('include/field_map_change.h', 'BOOL PokemonFollower_CanSpawn(int species, enum MapHeaderID mapHeaderID);', '''BOOL PokemonFollower_CanSpawn(int species, enum MapHeaderID mapHeaderID);

// follower map object data: [0] species, [2] flags
#define POKEMON_FOLLOWER_DATA_SPECIES   0
#define POKEMON_FOLLOWER_DATA_FLAGS     2
#define POKEMON_FOLLOWER_FLAG_SHINY     0x80
// spawned on the player's arrival tile (often a door); stays hidden until it steps off it
#define POKEMON_FOLLOWER_FLAG_ARRIVING  0x20''')

# spawn hidden
edit('src/field_map_change.c', '''    MapObject_SetDataAt(follower, species, 0);
    // data[2] bit 0x80 tells the follower renderer to use the texture's shiny palette
    MapObject_SetDataAt(follower, Pokemon_IsShiny(lead) ? 0x80 : 0, 2);
''', '''    MapObject_SetDataAt(follower, species, POKEMON_FOLLOWER_DATA_SPECIES);
    MapObject_SetDataAt(follower, POKEMON_FOLLOWER_FLAG_ARRIVING | (Pokemon_IsShiny(lead) ? POKEMON_FOLLOWER_FLAG_SHINY : 0), POKEMON_FOLLOWER_DATA_FLAGS);
    MapObject_SetHidden(follower, TRUE);
''')

# renderer: use the shared flags, and don't reveal an arriving follower
p = 'src/overlay005/ov5_021ECA70.c'
edit(p, '''#define FOLLOWER_SHINY_DATA   2
#define FOLLOWER_SHINY_FLAG   0x80
''', '''#define FOLLOWER_SHINY_DATA   POKEMON_FOLLOWER_DATA_FLAGS
#define FOLLOWER_SHINY_FLAG   POKEMON_FOLLOWER_FLAG_SHINY
''')
edit(p, '''            MapObject_SetHidden(mapObj, !PokemonFollower_CanSpawn(species, fieldSystem->location->mapHeaderID));''',
     '''            MapObject_SetHidden(mapObj, !PokemonFollower_CanSpawn(species, fieldSystem->location->mapHeaderID)
                    || (MapObject_GetDataAt(mapObj, POKEMON_FOLLOWER_DATA_FLAGS) & POKEMON_FOLLOWER_FLAG_ARRIVING));''')

# movement: reveal once the follower is off its spawn tile
p = 'src/unk_02069BE0.c'
edit(p, '#include "field/field_system.h"\n', '#include "field/field_system.h"\n#include "field_map_change.h"\n')
edit(p, '''static int PokemonFollower_StepState(MapObject *mapObj, UnkStruct_02069CA8 *param1)
{
    sub_02062D10(mapObj);
    MapObject_SetEndMovementOff(mapObj);
''', '''// The follower spawns on the player's arrival tile, which is often a door, and appears once it has
// stepped onto the free tile behind the player.
static void PokemonFollower_RevealAfterArrival(MapObject *mapObj)
{
    int flags = MapObject_GetDataAt(mapObj, POKEMON_FOLLOWER_DATA_FLAGS);

    if ((flags & POKEMON_FOLLOWER_FLAG_ARRIVING) == 0
        || (MapObject_GetX(mapObj) == MapObject_GetXInitial(mapObj) && MapObject_GetZ(mapObj) == MapObject_GetZInitial(mapObj))) {
        return;
    }

    MapObject_SetDataAt(mapObj, flags & ~POKEMON_FOLLOWER_FLAG_ARRIVING, POKEMON_FOLLOWER_DATA_FLAGS);

    int species = MapObject_GetDataAt(mapObj, POKEMON_FOLLOWER_DATA_SPECIES);

    if (PokemonFollower_CanSpawn(species, MapObject_FieldSystem(mapObj)->location->mapHeaderID)) {
        MapObject_SetHidden(mapObj, FALSE);
    }
}

static int PokemonFollower_StepState(MapObject *mapObj, UnkStruct_02069CA8 *param1)
{
    sub_02062D10(mapObj);
    MapObject_SetEndMovementOff(mapObj);
    PokemonFollower_RevealAfterArrival(mapObj);
''')
print('patched')
