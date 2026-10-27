"""Script command handlers for the Pokemon follower, ported from Following Platinum's commands 840 and
844-860 (see fp/FP_SCRIPT_CHANGES.md)."""
import os

os.chdir('C:/Projects/pokemon-platinum-dexnav/pkplat-dexnav')

HANDLERS = '''
// Following Platinum (command 840): let two Pokemon cries play at once
static BOOL ScrCmd_SetAllowTwoPokemonCries(ScriptContext *ctx)
{
    u8 *allow = SoundSystem_GetParam(SOUND_SYSTEM_PARAM_ALLOW_2_POKEMON_CRIES);

    *allow = ScriptContext_ReadHalfWord(ctx);
    return FALSE;
}

static BOOL PokemonFollower_IsBallAnimationDone(ScriptContext *ctx)
{
    OverworldAnimManager *animMan = (OverworldAnimManager *)ctx->data[0];

    return animMan == NULL || !OverworldAnimManager_IsActive(animMan);
}

// Following Platinum (command 844): before a cutscene the follower returns to its Poke Ball and stops
// following, so the script can move it (or leave it away) freely.
static BOOL ScrCmd_PokemonFollowerStartCutscene(ScriptContext *ctx)
{
    MapObject *follower = PokemonFollower_GetMapObject(ctx->fieldSystem);

    ctx->data[0] = 0;

    if (follower == NULL) {
        return FALSE;
    }

    sub_020656AC(follower);

    if (!MapObject_IsHidden(follower)) {
        ctx->data[0] = (u32)PokemonFollowerBall_Play(follower, POKE_BALL_ANIMATION_RECALL, MapObject_GetDataAt(follower, POKEMON_FOLLOWER_DATA_SPECIES));
    }

    PokemonFollower_SetFlag(follower, POKEMON_FOLLOWER_FLAG_ARRIVING | POKEMON_FOLLOWER_FLAG_RELEASE, FALSE);
    PokemonFollower_SetFlag(follower, POKEMON_FOLLOWER_FLAG_SCRIPT_HIDDEN, TRUE);
    sub_02062DB4(follower, TRUE);
    MapObject_SwitchMovementType(follower, MOVEMENT_TYPE_NONE);

    ScriptContext_Pause(ctx, PokemonFollower_IsBallAnimationDone);
    return TRUE;
}

// Following Platinum (command 846)
static BOOL ScrCmd_PokemonFollowerSetPaused(ScriptContext *ctx)
{
    u16 paused = ScriptContext_ReadHalfWord(ctx);
    MapObject *follower = PokemonFollower_GetMapObject(ctx->fieldSystem);

    if (follower != NULL) {
        if (paused) {
            MapObject_SetPauseMovementOn(follower);
        } else {
            MapObject_SetPauseMovementOff(follower);
        }
    }

    return FALSE;
}

static BOOL PokemonFollower_IsIdle(ScriptContext *ctx)
{
    MapObject *follower = PokemonFollower_GetMapObject(ctx->fieldSystem);

    return follower == NULL || LocalMapObj_CheckAnimationFinished(follower);
}

// Following Platinum (command 847): wait for the follower's current movement to finish
static BOOL ScrCmd_WaitForPokemonFollower(ScriptContext *ctx)
{
    ScriptContext_Pause(ctx, PokemonFollower_IsIdle);
    return TRUE;
}

// Following Platinum (command 848)
static BOOL ScrCmd_PokemonFollowerSetMovementType(ScriptContext *ctx)
{
    u16 movementType = ScriptContext_ReadHalfWord(ctx);
    MapObject *follower = PokemonFollower_GetMapObject(ctx->fieldSystem);

    if (follower != NULL) {
        MapObject_SwitchMovementType(follower, movementType);
    }

    return FALSE;
}

// Following Platinum (command 849): put the follower on the tile next to the player in a direction
static BOOL ScrCmd_PokemonFollowerPlaceNearPlayer(ScriptContext *ctx)
{
    u8 dir = ScriptContext_ReadByte(ctx);
    u8 facingDir = ScriptContext_ReadByte(ctx);
    MapObject *follower = PokemonFollower_GetMapObject(ctx->fieldSystem);
    VecFx32 pos;

    if (follower == NULL) {
        return FALSE;
    }

    MapObject_GetPosPtr(PlayerAvatar_GetMapObject(ctx->fieldSystem->playerAvatar), &pos);

    switch (dir) {
    case DIR_NORTH:
        pos.z -= FX32_CONST(16);
        break;
    case DIR_SOUTH:
        pos.z += FX32_CONST(16);
        break;
    case DIR_WEST:
        pos.x -= FX32_CONST(16);
        break;
    case DIR_EAST:
        pos.x += FX32_CONST(16);
        break;
    }

    MapObject_SetPosDirFromVec(follower, &pos, facingDir);
    return FALSE;
}

// After a cutscene the follower goes back under the player and follows again, appearing once the
// player steps away (popping out of its Poke Ball when release is set).
static void PokemonFollower_ReturnToPlayer(FieldSystem *fieldSystem, BOOL release)
{
    MapObject *follower = PokemonFollower_GetMapObject(fieldSystem);
    MapObject *player = PlayerAvatar_GetMapObject(fieldSystem->playerAvatar);

    if (follower == NULL) {
        return;
    }

    sub_020656AC(follower);
    MapObject_SetPosDirFromCoords(follower, MapObject_GetX(player), MapObject_GetY(player), MapObject_GetZ(player), MapObject_GetFacingDir(player));
    MapObject_SetXInitial(follower, MapObject_GetX(player));
    MapObject_SetZInitial(follower, MapObject_GetZ(player));
    PokemonFollower_SetScriptHidden(follower, TRUE);
    PokemonFollower_SetFlag(follower, POKEMON_FOLLOWER_FLAG_ARRIVING, TRUE);
    PokemonFollower_SetFlag(follower, POKEMON_FOLLOWER_FLAG_RELEASE, release);
    MapObject_SwitchMovementType(follower, MOVEMENT_TYPE_POKEMON_FOLLOWER);
}

// Following Platinum (command 850)
static BOOL ScrCmd_PokemonFollowerEndCutscene(ScriptContext *ctx)
{
    PokemonFollower_ReturnToPlayer(ctx->fieldSystem, TRUE);
    return FALSE;
}

// Following Platinum (command 851): like 850, without the Poke Ball
static BOOL ScrCmd_PokemonFollowerResumeFollowing(ScriptContext *ctx)
{
    PokemonFollower_ReturnToPlayer(ctx->fieldSystem, FALSE);
    return FALSE;
}

// Following Platinum (command 852): the follower pops out of its Poke Ball where it stands
static BOOL ScrCmd_PokemonFollowerRespawn(ScriptContext *ctx)
{
    MapObject *follower = PokemonFollower_GetMapObject(ctx->fieldSystem);

    if (follower != NULL) {
        MapObject_SwitchMovementType(follower, MOVEMENT_TYPE_POKEMON_FOLLOWER);
        PokemonFollower_SetScriptHidden(follower, TRUE);
        PokemonFollowerBall_Play(follower, POKE_BALL_ANIMATION_RELEASE, MapObject_GetDataAt(follower, POKEMON_FOLLOWER_DATA_SPECIES));
    }

    return FALSE;
}

// Following Platinum (command 853)
static BOOL ScrCmd_HidePokemonFollower(ScriptContext *ctx)
{
    MapObject *follower = PokemonFollower_GetMapObject(ctx->fieldSystem);

    if (follower != NULL) {
        PokemonFollower_SetScriptHidden(follower, TRUE);
    }

    return FALSE;
}

// Following Platinum (command 854)
static BOOL ScrCmd_ShowPokemonFollower(ScriptContext *ctx)
{
    MapObject *follower = PokemonFollower_GetMapObject(ctx->fieldSystem);

    if (follower != NULL) {
        PokemonFollower_SetScriptHidden(follower, FALSE);
    }

    return FALSE;
}

// Following Platinum (command 855): an object pops out of (mode 0) or returns to (mode 1) the Poke Ball
// of a party Pokemon, e.g. the new starter on Route 201 and the party in the Hall of Fame.
static BOOL ScrCmd_PlayPokeBallAnimation(ScriptContext *ctx)
{
    u16 localID = ScriptContext_GetVar(ctx);
    u16 partySlot = ScriptContext_GetVar(ctx);
    u16 mode = ScriptContext_GetVar(ctx);
    MapObject *mapObj = MapObjMan_LocalMapObjByIndex(ctx->fieldSystem->mapObjMan, localID);
    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(ctx->fieldSystem->saveData), partySlot);

    if (mapObj != NULL) {
        sub_020656AC(mapObj);
        PokemonFollowerBall_Play(mapObj, mode, Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL));
    }

    return FALSE;
}

// Following Platinum (command 857): the follower notices something, then walks off two tiles south
static const MapObjectAnimCmd sPokemonFollowerHappyHop[] = {
    { MOVEMENT_ACTION_DELAY_8, 3 },
    { MOVEMENT_ACTION_DELAY_4, 1 },
    { MOVEMENT_ACTION_FACE_SOUTH, 1 },
    { MOVEMENT_ACTION_FOLLOWER_EMOTE_EXCLAMATION, 1 },
    { MOVEMENT_ACTION_DELAY_8, 2 },
    { MOVEMENT_ACTION_WALK_FAST_SOUTH, 2 },
    { MOVEMENT_ACTION_END, 0 }
};

static BOOL ScrCmd_PokemonFollowerHappyHop(ScriptContext *ctx)
{
    MapObject *follower = PokemonFollower_GetMapObject(ctx->fieldSystem);

    if (follower != NULL) {
        SysTask *task = MapObject_StartAnimation(follower, (MapObjectAnimCmd *)sPokemonFollowerHappyHop);
        u8 *movementCount = FieldSystem_GetScriptMemberPtr(ctx->fieldSystem, SCRIPT_MANAGER_MOVEMENT_COUNT);

        (*movementCount)++;
        sub_02040F28(ctx->fieldSystem, task, NULL);
    }

    return FALSE;
}

// Following Platinum (command 858): overworld graphics of a party Pokemon
static BOOL ScrCmd_GetPartyMonFollowerGfx(ScriptContext *ctx)
{
    u16 partySlot = ScriptContext_GetVar(ctx);
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(ctx->fieldSystem->saveData), partySlot);

    *destVar = PokemonFollower_GraphicsID(mon);
    return FALSE;
}

// Following Platinum (command 859)
static BOOL ScrCmd_CheckPokemonFollowerActive(ScriptContext *ctx)
{
    u16 *destVar = ScriptContext_GetVarPointer(ctx);

    *destVar = PokemonFollower_GetMapObject(ctx->fieldSystem) != NULL;
    return FALSE;
}

// Following Platinum (command 860): which side of the player the follower is on
static BOOL ScrCmd_GetPokemonFollowerDirFromPlayer(ScriptContext *ctx)
{
    u16 *destVar = ScriptContext_GetVarPointer(ctx);
    MapObject *follower = PokemonFollower_GetMapObject(ctx->fieldSystem);
    MapObject *player = PlayerAvatar_GetMapObject(ctx->fieldSystem->playerAvatar);

    if (follower == NULL) {
        *destVar = DIR_SOUTH;
        return FALSE;
    }

    int dx = MapObject_GetX(follower) - MapObject_GetX(player);
    int dz = MapObject_GetZ(follower) - MapObject_GetZ(player);

    if (dx == 0 && dz < 0) {
        *destVar = DIR_NORTH;
    } else if (dx == 0) {
        *destVar = DIR_SOUTH;
    } else if (dx < 0) {
        *destVar = DIR_WEST;
    } else {
        *destVar = DIR_EAST;
    }

    return FALSE;
}
'''

DECLS = '''static BOOL ScrCmd_SetAllowTwoPokemonCries(ScriptContext *ctx);
static BOOL ScrCmd_PokemonFollowerStartCutscene(ScriptContext *ctx);
static BOOL ScrCmd_PokemonFollowerSetPaused(ScriptContext *ctx);
static BOOL ScrCmd_WaitForPokemonFollower(ScriptContext *ctx);
static BOOL ScrCmd_PokemonFollowerSetMovementType(ScriptContext *ctx);
static BOOL ScrCmd_PokemonFollowerPlaceNearPlayer(ScriptContext *ctx);
static BOOL ScrCmd_PokemonFollowerEndCutscene(ScriptContext *ctx);
static BOOL ScrCmd_PokemonFollowerResumeFollowing(ScriptContext *ctx);
static BOOL ScrCmd_PokemonFollowerRespawn(ScriptContext *ctx);
static BOOL ScrCmd_HidePokemonFollower(ScriptContext *ctx);
static BOOL ScrCmd_ShowPokemonFollower(ScriptContext *ctx);
static BOOL ScrCmd_PlayPokeBallAnimation(ScriptContext *ctx);
static BOOL ScrCmd_PokemonFollowerHappyHop(ScriptContext *ctx);
static BOOL ScrCmd_GetPartyMonFollowerGfx(ScriptContext *ctx);
static BOOL ScrCmd_CheckPokemonFollowerActive(ScriptContext *ctx);
static BOOL ScrCmd_GetPokemonFollowerDirFromPlayer(ScriptContext *ctx);
'''

p = 'src/scrcmd.c'
s = open(p, encoding='utf-8').read()
anchor = 'static BOOL ScrCmd_GetPokemonFollowerPartySlot(ScriptContext *ctx);\n'
assert s.count(anchor) == 1
s = s.replace(anchor, anchor + DECLS)
s = s.rstrip('\n') + '\n' + HANDLERS
if '#include "overlay005/pokemon_follower_ball.h"' not in s:
    s = s.replace('#include "field_map_change.h"\n', '#include "field_map_change.h"\n', 1)
    marker = '#include "overlay005/'
    i = s.index(marker)
    s = s[:i] + '#include "overlay005/pokemon_follower_ball.h"\n' + s[i:]
open(p, 'w', encoding='utf-8', newline='\n').write(s)
print('ok')
