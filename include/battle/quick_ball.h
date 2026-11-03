#ifndef POKEPLATINUM_BATTLE_QUICK_BALL_H
#define POKEPLATINUM_BATTLE_QUICK_BALL_H

#include "struct_decls/battle_system.h"

#include "savedata.h"

// Quick Run and Quick Ball: at the action menu of a plain wild battle, R runs and L throws the ball shown
// in a small widget at the top screen's bottom left; holding L and pressing Left or Right picks another ball.
enum QuickBallInput {
    QUICK_BALL_INPUT_NONE = 0, // nothing for the action menu: let it read its own input
    QUICK_BALL_INPUT_HELD,     // L is down (or was used to pick a ball): the action menu ignores this frame
    QUICK_BALL_INPUT_THROW,    // throw QuickBall_Ball()
    QUICK_BALL_INPUT_BAG,      // the ball can't be thrown right now: open the Bag, which says why
};

void QuickBall_Reset(SaveData *saveData);
void QuickBall_SetLastThrown(BattleSystem *battleSys, u16 ball);
void QuickBall_Show(BattleSystem *battleSys);
void QuickBall_Hide(BattleSystem *battleSys);
enum QuickBallInput QuickBall_HandleInput(BattleSystem *battleSys);
u16 QuickBall_Ball(void);

#endif // POKEPLATINUM_BATTLE_QUICK_BALL_H
