"""Declare Following Platinum's script commands (ids 840, 844-860 in its ROM) as decomp script commands:
macros in asm/macros/scrcmd.inc and table entries in include/data/scripts/scrcmd.h. Handlers live in
src/pokemon_follower_scrcmd.c."""
import os

os.chdir('C:/Projects/pokemon-platinum-dexnav/pkplat-dexnav')

# (macro, SCRCMD name, handler, [(param, size)])  size: 'h' = halfword, 'b' = byte
COMMANDS = [
    ('SetAllowTwoPokemonCries', 'SCRCMD_SETALLOWTWOPOKEMONCRIES', 'ScrCmd_SetAllowTwoPokemonCries', [('allow', 'h')]),
    ('PokemonFollowerStartCutscene', 'SCRCMD_POKEMONFOLLOWERSTARTCUTSCENE', 'ScrCmd_PokemonFollowerStartCutscene', []),
    ('PokemonFollowerSetPaused', 'SCRCMD_POKEMONFOLLOWERSETPAUSED', 'ScrCmd_PokemonFollowerSetPaused', [('paused', 'h')]),
    ('WaitForPokemonFollower', 'SCRCMD_WAITFORPOKEMONFOLLOWER', 'ScrCmd_WaitForPokemonFollower', []),
    ('PokemonFollowerSetMovementType', 'SCRCMD_POKEMONFOLLOWERSETMOVEMENTTYPE', 'ScrCmd_PokemonFollowerSetMovementType', [('movementType', 'h')]),
    ('PokemonFollowerPlaceNearPlayer', 'SCRCMD_POKEMONFOLLOWERPLACENEARPLAYER', 'ScrCmd_PokemonFollowerPlaceNearPlayer', [('xOffset', 'b'), ('zOffset', 'b')]),
    ('PokemonFollowerEndCutscene', 'SCRCMD_POKEMONFOLLOWERENDCUTSCENE', 'ScrCmd_PokemonFollowerEndCutscene', []),
    ('PokemonFollowerResumeFollowing', 'SCRCMD_POKEMONFOLLOWERRESUMEFOLLOWING', 'ScrCmd_PokemonFollowerResumeFollowing', []),
    ('PokemonFollowerRespawn', 'SCRCMD_POKEMONFOLLOWERRESPAWN', 'ScrCmd_PokemonFollowerRespawn', []),
    ('HidePokemonFollower', 'SCRCMD_HIDEPOKEMONFOLLOWER', 'ScrCmd_HidePokemonFollower', []),
    ('ShowPokemonFollower', 'SCRCMD_SHOWPOKEMONFOLLOWER', 'ScrCmd_ShowPokemonFollower', []),
    ('PlayPokeBallAnimation', 'SCRCMD_PLAYPOKEBALLANIMATION', 'ScrCmd_PlayPokeBallAnimation', [('localID', 'h'), ('partySlot', 'h'), ('mode', 'h')]),
    ('PokemonFollowerHappyHop', 'SCRCMD_POKEMONFOLLOWERHAPPYHOP', 'ScrCmd_PokemonFollowerHappyHop', []),
    ('GetPartyMonFollowerGfx', 'SCRCMD_GETPARTYMONFOLLOWERGFX', 'ScrCmd_GetPartyMonFollowerGfx', [('partySlot', 'h'), ('destVarID', 'h')]),
    ('CheckPokemonFollowerActive', 'SCRCMD_CHECKPOKEMONFOLLOWERACTIVE', 'ScrCmd_CheckPokemonFollowerActive', [('destVarID', 'h')]),
    ('GetPokemonFollowerDirFromPlayer', 'SCRCMD_GETPOKEMONFOLLOWERDIRFROMPLAYER', 'ScrCmd_GetPokemonFollowerDirFromPlayer', [('destVarID', 'h')]),
]


def edit(p, old, new):
    s = open(p, encoding='utf-8').read()
    assert s.count(old) == 1, (p, old[:70], s.count(old))
    open(p, 'w', encoding='utf-8', newline='\n').write(s.replace(old, new))


macros = ''
for macro, scrcmd, _, params in COMMANDS:
    args = ' ' + ', '.join(p for p, _ in params) if params else ''
    body = ''.join(f"    .{'short' if size == 'h' else 'byte'} \\{p}\n" for p, size in params)
    macros += f'\n    .macro {macro}{args}\n    .short {scrcmd}\n{body}    .endm\n'

anchor = '''    .macro GetPokemonFollowerPartySlot destVarID
    .short SCRCMD_GETPOKEMONFOLLOWERPARTYSLOT
    .short \\destVarID
    .endm
'''
edit('asm/macros/scrcmd.inc', anchor, anchor + macros)

entries = ''.join(f'ScriptCommand({scrcmd + ",":<75}{handler})\n' for _, scrcmd, handler, _ in COMMANDS)
anchor = 'ScriptCommand(SCRCMD_GETPOKEMONFOLLOWERPARTYSLOT,                          ScrCmd_GetPokemonFollowerPartySlot)\n'
edit('include/data/scripts/scrcmd.h', anchor, anchor + entries)
print('ok')
