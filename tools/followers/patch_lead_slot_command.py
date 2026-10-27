"""Add PokemonFollower_GetLeadSlot and the GetPokemonFollowerPartySlot script command
(Following Platinum's command 856, used by the talk-to-follower script)."""
import os

os.chdir('C:/projects/pokemon-platinum-dexnav/pkplat-dexnav')


def edit(p, old, new):
    s = open(p, encoding='utf-8').read()
    assert s.count(old) == 1, (p, old[:70], s.count(old))
    open(p, 'w', encoding='utf-8', newline='\n').write(s.replace(old, new))


p = 'src/field_map_change.c'
edit(p, '''Pokemon *PokemonFollower_GetLead(SaveData *saveData)
{
    Party *party = SaveData_GetParty(saveData);

    if (Party_GetCurrentCount(party) == 0) {
        return NULL;
    }

    return Party_AliveMonsCount(party) > 0 ? Party_FindFirstEligibleBattler(party) : Party_FindFirstHatchedMon(party);
}

''', '''// The first Pokemon able to battle follows the player, or the first hatched one if all have fainted.
int PokemonFollower_GetLeadSlot(SaveData *saveData)
{
    Party *party = SaveData_GetParty(saveData);
    int partyCount = Party_GetCurrentCount(party);
    int firstHatched = PARTY_SLOT_NONE;

    for (int i = 0; i < partyCount; i++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(party, i);

        if (Pokemon_CanBattle(mon)) {
            return i;
        }

        if (firstHatched == PARTY_SLOT_NONE && Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL) == FALSE) {
            firstHatched = i;
        }
    }

    return firstHatched;
}

Pokemon *PokemonFollower_GetLead(SaveData *saveData)
{
    int slot = PokemonFollower_GetLeadSlot(saveData);

    return slot == PARTY_SLOT_NONE ? NULL : Party_GetPokemonBySlotIndex(SaveData_GetParty(saveData), slot);
}
''')
# the spawn comment belongs to the spawn function, not the exclusion table
edit(p, '''// Ported from Following Platinum's spawn routine: the first Pokemon able to battle (or the first
// hatched one if all have fainted) follows the player, starting on the player's own tile.
// Following Platinum's spawn rules''', '''// Following Platinum's spawn rules''')
edit(p, '''static void FieldMapChange_CreatePokemonFollower(FieldSystem *fieldSystem)
{''', '''// Ported from Following Platinum's spawn routine: the lead Pokemon follows the player, starting
// hidden on the player's own tile.
static void FieldMapChange_CreatePokemonFollower(FieldSystem *fieldSystem)
{''')

edit('include/field_map_change.h', 'Pokemon *PokemonFollower_GetLead(SaveData *saveData);',
     'int PokemonFollower_GetLeadSlot(SaveData *saveData);\nPokemon *PokemonFollower_GetLead(SaveData *saveData);')

# script command
edit('asm/macros/scrcmd.inc', '''    .macro DexNavPrintMessage messageID
    .short SCRCMD_DEXNAVPRINTMESSAGE
    .short \\messageID
    .endm
''', '''    .macro DexNavPrintMessage messageID
    .short SCRCMD_DEXNAVPRINTMESSAGE
    .short \\messageID
    .endm

    .macro GetPokemonFollowerPartySlot destVarID
    .short SCRCMD_GETPOKEMONFOLLOWERPARTYSLOT
    .short \\destVarID
    .endm
''')
edit('include/data/scripts/scrcmd.h', 'ScriptCommand(SCRCMD_DEXNAVPRINTMESSAGE,                                   ScrCmd_DexNavPrintMessage)\n',
     'ScriptCommand(SCRCMD_DEXNAVPRINTMESSAGE,                                   ScrCmd_DexNavPrintMessage)\n'
     'ScriptCommand(SCRCMD_GETPOKEMONFOLLOWERPARTYSLOT,                          ScrCmd_GetPokemonFollowerPartySlot)\n')

p = 'src/scrcmd.c'
edit(p, 'static BOOL ScrCmd_OpenDexNav(ScriptContext *ctx);\n',
     'static BOOL ScrCmd_OpenDexNav(ScriptContext *ctx);\nstatic BOOL ScrCmd_GetPokemonFollowerPartySlot(ScriptContext *ctx);\n')
s = open(p, encoding='utf-8').read()
s = s.rstrip('\n') + '''

// Following Platinum (command 856): party slot of the Pokemon following the player
static BOOL ScrCmd_GetPokemonFollowerPartySlot(ScriptContext *ctx)
{
    u16 *destVar = ScriptContext_GetVarPointer(ctx);

    *destVar = PokemonFollower_GetLeadSlot(ctx->fieldSystem->saveData);
    return FALSE;
}
'''
open(p, 'w', encoding='utf-8', newline='\n').write(s)
print('ok')
