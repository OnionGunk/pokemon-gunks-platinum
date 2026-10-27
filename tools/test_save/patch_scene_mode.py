"""Test builds: gTestSaveMode 2 = jump straight to Dawn's catching tutorial on Route 202 (a plain new
game with a Piplup and the story flags that scene needs), two steps east of its trigger."""
import os

os.chdir('C:/Projects/pokemon-platinum-dexnav/pkplat-dexnav')


def edit(p, old, new):
    s = open(p, encoding='utf-8').read()
    assert s.count(old) == 1, (p, old[:70], s.count(old))
    open(p, 'w', encoding='utf-8', newline='\n').write(s.replace(old, new))


edit('src/location.c', '''// test builds start on Twinleaf's north road''', '''static const Location sRoute202TutorialLocation = {
    .mapHeaderID = MAP_HEADER_ROUTE_202,
    .warpId = WARP_ID_NONE,
    .x = 182,
    .z = 827,
    .faceDirection = FACE_LEFT,
};

// test builds start on Twinleaf's north road''')
edit('src/location.c', '''    if (gTestSaveMode == 1) {
        *outLocation = sStoryStartLocation;
        return;
    }''', '''    if (gTestSaveMode == 1) {
        *outLocation = sStoryStartLocation;
        return;
    }

    if (gTestSaveMode == 2) {
        *outLocation = sRoute202TutorialLocation;
        return;
    }''')

edit('src/game_start.c', '''    // story start: a plain new game, so story cutscenes can be tested from the beginning
    if (gTestSaveMode == 1) {
        goto testSaveDone;
    }''', '''    // story start: a plain new game, so story cutscenes can be tested from the beginning
    if (gTestSaveMode == 1) {
        goto testSaveDone;
    }

    // Route 202 tutorial: just a starter (the story flags are set after the new-game script)
    if (gTestSaveMode == 2) {
        Pokemon_GiveMonFromScript(HEAP_ID_GAME_START, saveData, SPECIES_PIPLUP, 10, 0, 0, 0);
        SystemVars_SetPlayerStarter(varsFlags, SPECIES_PIPLUP);
        goto testSaveDone;
    }''')

edit('src/field_map_change.c', '''    if (gTestSaveMode == 0) {
        TestSave_ApplyPostgame(SaveData_GetVarsFlags(fieldSystem->saveData));
    }''', '''    if (gTestSaveMode == 0) {
        TestSave_ApplyPostgame(SaveData_GetVarsFlags(fieldSystem->saveData));
    } else if (gTestSaveMode == 2) {
        VarsFlags *varsFlags = SaveData_GetVarsFlags(fieldSystem->saveData);

        // what the story has done by the time Dawn's catching tutorial starts
        VarsFlags_SetFlag(varsFlags, FLAG_RECEIVED_PARCEL);
        VarsFlags_ClearFlag(varsFlags, FLAG_HIDE_ROUTE_202_COUNTERPART);
        *VarsFlags_GetVarAddress(varsFlags, VAR_ROUTE_202_STATE) = 0;
    }''')
print('ok')
