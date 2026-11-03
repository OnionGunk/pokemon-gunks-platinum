#include "battle/quick_ball.h"

#include <nitro.h>
#include <string.h>

#include "constants/battle.h"
#include "constants/battle/condition.h"
#include "constants/battle/moves.h"
#include "constants/charcode.h"
#include "constants/heap.h"
#include "generated/items.h"

#include "battle/battle_context.h"
#include "battle/battle_system.h"

#include "bag.h"
#include "bg_window.h"
#include "dexnav_search.h"
#include "font.h"
#include "item.h"
#include "narc.h"
#include "palette.h"
#include "party.h"
#include "pc_boxes.h"
#include "sound_playback.h"
#include "sprite_system.h"
#include "string_gf.h"
#include "system.h"
#include "text.h"

#define QUICK_BALL_RES_ID   20500
#define QUICK_BALL_COUNT_MAX 99

// a small label just above the message box (tiles y 19-22): "L ×99", and the ball's icon left of it
#define LABEL_BG_LAYER   BG_LAYER_MAIN_1
#define LABEL_LEFT       4
#define LABEL_TOP        17
#define LABEL_WIDTH      6
#define LABEL_HEIGHT     2
#define LABEL_PALETTE    11
// after the message box window's tiles (base 31, 27 x 4)
#define LABEL_BASE_TILE  (31 + 27 * 4)
#define ICON_X           18
#define ICON_Y           144

typedef struct QuickBall {
    SaveData *saveData; // the battle only has copies of the Bag and party; the last ball goes to the real save
    BOOL shown;
    BOOL lHeld;
    BOOL picked;
    u16 ball;
    Window *label;
    ManagedSprite *icon;
} QuickBall;

static QuickBall sQuickBall;

static BOOL IsPlainWildBattle(BattleSystem *battleSys)
{
    u32 type = BattleSystem_GetBattleType(battleSys);

    if (type != BATTLE_TYPE_WILD_MON && type != BATTLE_TYPE_ROAMER) {
        return FALSE;
    }

    return (BattleSystem_GetBattleStatusMask(battleSys) & (BATTLE_STATUS_FIRST_BATTLE | BATTLE_STATUS_RECORDING)) == 0;
}

static BOOL IsQuickBall(u16 item)
{
    return item >= ITEM_MASTER_BALL && item <= ITEM_CHERISH_BALL && item != ITEM_SAFARI_BALL;
}

static u16 BallCount(BattleSystem *battleSys, u16 ball)
{
    return Bag_GetItemQuantity(BattleSystem_GetBag(battleSys), ball, HEAP_ID_BATTLE);
}

// the next ball the player has, after `from` in item order (wrapping), or ITEM_NONE
static u16 NextBall(BattleSystem *battleSys, u16 from, int step)
{
    u16 ball = from;

    for (int i = 0; i < ITEM_CHERISH_BALL; i++) {
        ball += step;

        if (ball > ITEM_CHERISH_BALL) {
            ball = ITEM_MASTER_BALL;
        } else if (ball < ITEM_MASTER_BALL) {
            ball = ITEM_CHERISH_BALL;
        }

        if (IsQuickBall(ball) && BallCount(battleSys, ball) > 0) {
            return ball;
        }
    }

    return ITEM_NONE;
}

// the last ball thrown (kept in the save), else a Poke Ball, else the first ball the player has
static u16 DefaultBall(BattleSystem *battleSys)
{
    u16 last = sQuickBall.saveData != NULL ? DexNavSave_GetLastBall(sQuickBall.saveData) : ITEM_NONE;

    if (IsQuickBall(last) && BallCount(battleSys, last) > 0) {
        return last;
    }

    if (BallCount(battleSys, ITEM_POKE_BALL) > 0) {
        return ITEM_POKE_BALL;
    }

    return NextBall(battleSys, ITEM_NONE, 1);
}

static void DrawLabel(BattleSystem *battleSys)
{
    u16 count = BallCount(battleSys, sQuickBall.ball);
    String *string = String_Init(8, HEAP_ID_BATTLE);

    if (count > QUICK_BALL_COUNT_MAX) {
        count = QUICK_BALL_COUNT_MAX;
    }

    String_AppendChar(string, CHAR_L);
    String_AppendChar(string, CHAR_SPACE);
    String_AppendChar(string, CHAR_MULTIPLY);

    if (count >= 10) {
        String_AppendChar(string, CHAR_0 + count / 10);
    }

    String_AppendChar(string, CHAR_0 + count % 10);

    Window_FillTilemap(sQuickBall.label, 15);
    Text_AddPrinterWithParamsAndColor(sQuickBall.label, FONT_SYSTEM, string, 2, 0, TEXT_SPEED_NO_TRANSFER, TEXT_COLOR(1, 2, 15), NULL);
    Window_CopyToVRAM(sQuickBall.label);
    String_Free(string);
}

static void LoadIcon(BattleSystem *battleSys)
{
    SpriteSystem *spriteSys = BattleSystem_GetSpriteSystem(battleSys);
    SpriteManager *spriteMan = BattleSystem_GetSpriteManager(battleSys);
    NARC *narc = NARC_ctor(NARC_INDEX_ITEMTOOL__ITEMDATA__ITEM_ICON, HEAP_ID_BATTLE);
    SpriteTemplate template;

    // with the battle's own OBJ mapping: the icon file's mapping type would switch the whole top screen's
    // (the healthboxes came out scrambled)
    SpriteSystem_LoadCharResObjFromOpenNarcWithHardwareMappingType(spriteSys, spriteMan, narc, Item_FileID(sQuickBall.ball, ITEM_FILE_TYPE_ICON), FALSE, NNS_G2D_VRAM_TYPE_2DMAIN, QUICK_BALL_RES_ID);
    u8 palette = SpriteSystem_LoadPaletteBufferFromOpenNarc(BattleSystem_GetPaletteData(battleSys), PLTTBUF_MAIN_OBJ, spriteSys, spriteMan, narc, Item_FileID(sQuickBall.ball, ITEM_FILE_TYPE_PALETTE), FALSE, 1, NNS_G2D_VRAM_TYPE_2DMAIN, QUICK_BALL_RES_ID);

    // the battle uploads its palette buffer every frame: put this ball's colors in it too, or a slot freed
    // by the last ball comes back with that ball's colors (as the Bag does for its item icons)
    PaletteData_LoadBufferFromFileStart(BattleSystem_GetPaletteData(battleSys), NARC_INDEX_ITEMTOOL__ITEMDATA__ITEM_ICON, Item_FileID(sQuickBall.ball, ITEM_FILE_TYPE_PALETTE), HEAP_ID_BATTLE, PLTTBUF_MAIN_OBJ, PALETTE_SIZE_BYTES, PLTT_DEST(palette));
    SpriteSystem_LoadCellResObjFromOpenNarc(spriteSys, spriteMan, narc, Item_IconNCERFile(), FALSE, QUICK_BALL_RES_ID);
    SpriteSystem_LoadAnimResObjFromOpenNarc(spriteSys, spriteMan, narc, Item_IconNANRFile(), FALSE, QUICK_BALL_RES_ID);
    NARC_dtor(narc);

    template.x = ICON_X;
    template.y = ICON_Y;
    template.z = 0;
    template.animIdx = 0;
    template.priority = 0;
    template.plttIdx = 0;
    template.vramType = NNS_G2D_VRAM_TYPE_2DMAIN;
    template.resources[SPRITE_RESOURCE_CHAR] = QUICK_BALL_RES_ID;
    template.resources[SPRITE_RESOURCE_PLTT] = QUICK_BALL_RES_ID;
    template.resources[SPRITE_RESOURCE_CELL] = QUICK_BALL_RES_ID;
    template.resources[SPRITE_RESOURCE_ANIM] = QUICK_BALL_RES_ID;
    template.bgPriority = 0;
    template.vramTransfer = FALSE;

    sQuickBall.icon = SpriteSystem_NewSprite(spriteSys, spriteMan, &template);
    ManagedSprite_SetDrawFlag(sQuickBall.icon, TRUE);
}

static void FreeIcon(BattleSystem *battleSys)
{
    if (sQuickBall.icon != NULL) {
        SpriteManager *spriteMan = BattleSystem_GetSpriteManager(battleSys);

        // the sprite, then its resources: loading a resource ID still in use keeps the old ball's graphics
        Sprite_DeleteAndFreeResources(sQuickBall.icon);
        SpriteManager_UnloadCharObjById(spriteMan, QUICK_BALL_RES_ID);
        SpriteManager_UnloadPlttObjById(spriteMan, QUICK_BALL_RES_ID);
        SpriteManager_UnloadCellObjById(spriteMan, QUICK_BALL_RES_ID);
        SpriteManager_UnloadAnimObjById(spriteMan, QUICK_BALL_RES_ID);
        sQuickBall.icon = NULL;
    }
}

// at the start of each battle: the shown ball starts from the save again
void QuickBall_Reset(SaveData *saveData)
{
    memset(&sQuickBall, 0, sizeof(sQuickBall));
    sQuickBall.saveData = saveData;
}

// a ball was thrown, by the Bag or by L: show it next time, and keep it in the save
void QuickBall_SetLastThrown(BattleSystem *battleSys, u16 ball)
{
    if (IsQuickBall(ball)) {
        sQuickBall.ball = ball;

        if (sQuickBall.saveData != NULL) {
            DexNavSave_SetLastBall(sQuickBall.saveData, ball);
        }
    }
}

void QuickBall_Show(BattleSystem *battleSys)
{
    QuickBall_Hide(battleSys);

    if (!IsPlainWildBattle(battleSys)) {
        return;
    }

    if (!IsQuickBall(sQuickBall.ball) || BallCount(battleSys, sQuickBall.ball) == 0) {
        sQuickBall.ball = DefaultBall(battleSys);
    }

    if (sQuickBall.ball == ITEM_NONE) {
        return;
    }

    sQuickBall.label = Window_New(HEAP_ID_BATTLE, 1);
    Window_Add(BattleSystem_GetBgConfig(battleSys), sQuickBall.label, LABEL_BG_LAYER, LABEL_LEFT, LABEL_TOP, LABEL_WIDTH, LABEL_HEIGHT, LABEL_PALETTE, LABEL_BASE_TILE);
    DrawLabel(battleSys);
    LoadIcon(battleSys);

    sQuickBall.shown = TRUE;
    sQuickBall.lHeld = FALSE;
    sQuickBall.picked = FALSE;
}

void QuickBall_Hide(BattleSystem *battleSys)
{
    if (!sQuickBall.shown) {
        return;
    }

    FreeIcon(battleSys);
    Window_ClearAndCopyToVRAM(sQuickBall.label);
    Window_Remove(sQuickBall.label);
    Heap_Free(sQuickBall.label);
    sQuickBall.label = NULL;
    sQuickBall.shown = FALSE;
}

static void PickBall(BattleSystem *battleSys, int step)
{
    u16 ball = NextBall(battleSys, sQuickBall.ball, step);

    if (ball == ITEM_NONE || ball == sQuickBall.ball) {
        return;
    }

    sQuickBall.ball = ball;
    FreeIcon(battleSys);
    LoadIcon(battleSys);
    DrawLabel(battleSys);
    Sound_PlayEffect(SEQ_SE_DP_BAG_004_sseq);
}

// the checks the Bag makes before it lets a ball be thrown (battle_bag.c TryUseItem)
static BOOL CanThrow(BattleSystem *battleSys)
{
    BattleContext *battleCtx = BattleSystem_GetBattleContext(battleSys);

    if (battleCtx->battleMons[BATTLER_ENEMY_1].moveEffectsMask & MOVE_EFFECT_SEMI_INVULNERABLE) {
        return FALSE;
    }

    if (battleCtx->battleMons[BATTLER_ENEMY_1].statusVolatile & VOLATILE_CONDITION_SUBSTITUTE) {
        return FALSE;
    }

    return !(Party_GetCurrentCount(BattleSystem_GetParty(battleSys, BATTLER_PLAYER_1)) == MAX_PARTY_SIZE
        && PCBoxes_FirstEmptyBox(BattleSystem_GetPCBoxes(battleSys)) == MAX_PC_BOXES);
}

enum QuickBallInput QuickBall_HandleInput(BattleSystem *battleSys)
{
    if (!sQuickBall.shown) {
        return QUICK_BALL_INPUT_NONE;
    }

    if (gSystem.heldKeys & PAD_BUTTON_L) {
        if (!sQuickBall.lHeld) {
            sQuickBall.lHeld = TRUE;
            sQuickBall.picked = FALSE;
        }

        if (gSystem.pressedKeys & (PAD_KEY_LEFT | PAD_KEY_RIGHT)) {
            PickBall(battleSys, (gSystem.pressedKeys & PAD_KEY_LEFT) ? -1 : 1);
            sQuickBall.picked = TRUE;
        }

        return QUICK_BALL_INPUT_HELD;
    }

    if (sQuickBall.lHeld) {
        // L let go: a tap throws, a hold that picked a ball doesn't
        sQuickBall.lHeld = FALSE;

        if (sQuickBall.picked) {
            return QUICK_BALL_INPUT_HELD;
        }

        return CanThrow(battleSys) ? QUICK_BALL_INPUT_THROW : QUICK_BALL_INPUT_BAG;
    }

    return QUICK_BALL_INPUT_NONE;
}

u16 QuickBall_Ball(void)
{
    return sQuickBall.ball;
}
