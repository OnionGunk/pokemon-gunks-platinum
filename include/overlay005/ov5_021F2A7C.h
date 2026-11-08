#ifndef POKEPLATINUM_OV5_021F2A7C_H
#define POKEPLATINUM_OV5_021F2A7C_H

#include "struct_decls/map_object.h"

#include "field/field_system.h"
#include "overlay005/field_effect_manager.h"
#include "overworld_anim_manager.h"

void *ov5_021F2A7C(FieldEffectManager *param0);
void ov5_021F2A98(void *param0);
void ov5_021F2AE4(MapObject *param0, int param1, int param2, int param3);
OverworldAnimManager *RippleEffect_StartOnTile(FieldSystem *fieldSystem, int x, int z, BOOL fast);

#endif // POKEPLATINUM_OV5_021F2A7C_H
