"""Test builds: gTestSaveMode picks the save the harness starts. 0 = post-game test save (default),
1 = story start (a normal new game in the player's room, for testing story cutscenes).
Both name the rival BARRY so NPC lines aren't blank."""
import os

os.chdir('C:/Projects/pokemon-platinum-dexnav/pkplat-dexnav')


def edit(p, old, new):
    s = open(p, encoding='utf-8').read()
    assert s.count(old) == 1, (p, old[:70], s.count(old))
    open(p, 'w', encoding='utf-8', newline='\n').write(s.replace(old, new))


edit('src/dexnav_search.c', '''// Test hook: the emulator harness writes a species here to choose the test save's lead Pokemon.
u16 gTestLeadSpecies;
''', '''// Test hooks: the emulator harness writes these before a new game. gTestLeadSpecies picks the
// test save's lead Pokemon; gTestSaveMode picks post-game (0) or story start (1).
u16 gTestLeadSpecies;
u16 gTestSaveMode;
''')

# start location: story mode starts where a real new game does
edit('src/location.c', '''#ifdef TEST_SKIP_INTRO
// test builds start on Twinleaf's north road: Route 201's grass and the map border are a few steps north
static const Location sPlayerStartLocation = {''', '''#ifdef TEST_SKIP_INTRO
extern u16 gTestSaveMode;

static const Location sStoryStartLocation = {
    .mapHeaderID = MAP_HEADER_TWINLEAF_TOWN_PLAYER_HOUSE_2F,
    .warpId = WARP_ID_NONE,
    .x = 4,
    .z = 6,
    .faceDirection = FACE_UP,
};

// test builds start on Twinleaf's north road: Route 201's grass and the map border are a few steps north
static const Location sPlayerStartLocation = {''')
edit('src/location.c', '''void SetPlayerStartLocation(Location *outLocation)
{
    *outLocation = sPlayerStartLocation;
}''', '''void SetPlayerStartLocation(Location *outLocation)
{
#ifdef TEST_SKIP_INTRO
    if (gTestSaveMode == 1) {
        *outLocation = sStoryStartLocation;
        return;
    }
#endif
    *outLocation = sPlayerStartLocation;
}''')

p = 'src/game_start.c'
edit(p, '''extern u16 gTestLeadSpecies;
''', '''extern u16 gTestLeadSpecies;
extern u16 gTestSaveMode;
''')
edit(p, '''#ifdef TEST_SKIP_INTRO
    // test save: Poketch, Pokedex, all badges, every town visited, Master Balls and a flyer
    VarsFlags *varsFlags = SaveData_GetVarsFlags(saveData);
    TrainerInfo *trainerInfo = SaveData_GetTrainerInfo(saveData);

    Poketch_Enable''', '''#ifdef TEST_SKIP_INTRO
    VarsFlags *varsFlags = SaveData_GetVarsFlags(saveData);
    TrainerInfo *trainerInfo = SaveData_GetTrainerInfo(saveData);
    static const charcode_t testName[] = { CHAR_T, CHAR_E, CHAR_S, CHAR_T, CHAR_EOS };
    static const charcode_t rivalName[] = { CHAR_B, CHAR_A, CHAR_R, CHAR_R, CHAR_Y, CHAR_EOS };
    String *rivalNameString = String_Init(8, HEAP_ID_GAME_START);

    TrainerInfo_SetName(trainerInfo, testName);
    String_CopyChars(rivalNameString, rivalName);
    MiscSaveBlock_SetRivalName(SaveData_MiscSaveBlock(saveData), rivalNameString);
    String_Free(rivalNameString);

    // story start: a plain new game, so story cutscenes can be tested from the beginning
    if (gTestSaveMode == 1) {
        goto testSaveDone;
    }

    // test save: Poketch, Pokedex, all badges, every town visited, Master Balls and a flyer
    Poketch_Enable''')
edit(p, '''    static const charcode_t testName[] = { CHAR_T, CHAR_E, CHAR_S, CHAR_T, CHAR_EOS };
    TrainerInfo_SetName(trainerInfo, testName);

    for (int badge''', '''    for (int badge''')
print('ok')
