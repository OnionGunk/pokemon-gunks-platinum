"""Register the DexNav Pokétch app (src/applications/poketch/dexnav): app ID, overlay, build entries,
its text bank, and the shared area builder it uses."""
import json
import os

os.chdir('C:/Projects/pokemon-platinum-dexnav/pkplat-dexnav')


def edit(p, old, new):
    s = open(p, encoding='utf-8').read()
    assert s.count(old) == 1, (p, old[:70], s.count(old))
    open(p, 'w', encoding='utf-8', newline='\n').write(s.replace(old, new))


edit('generated/poketch_apps.txt', 'POKETCH_APPID_ALARMCLOCK\nPOKETCH_APPID_MAX\n', 'POKETCH_APPID_ALARMCLOCK\nPOKETCH_APPID_DEXNAV\nPOKETCH_APPID_MAX\n')

p = 'src/applications/poketch/poketch_system.c'
edit(p, 'FS_EXTERN_OVERLAY(poketch_pokemon_history);\n', 'FS_EXTERN_OVERLAY(poketch_pokemon_history);\nFS_EXTERN_OVERLAY(poketch_dexnav);\n')
edit(p, '    { POKETCH_APPID_POKEMONHISTORY, FS_OVERLAY_ID(poketch_pokemon_history) }',
     '    { POKETCH_APPID_POKEMONHISTORY, FS_OVERLAY_ID(poketch_pokemon_history) },\n    { POKETCH_APPID_DEXNAV, FS_OVERLAY_ID(poketch_dexnav) }')

edit('platinum.us/main.lsf.in', '''Overlay poketch_pokemon_history
{
	After poketch
	Object src/applications/poketch/pokemon_history/main.c
	Object src/applications/poketch/pokemon_history/graphics.c
}
''', '''Overlay poketch_pokemon_history
{
	After poketch
	Object src/applications/poketch/pokemon_history/main.c
	Object src/applications/poketch/pokemon_history/graphics.c
}

Overlay poketch_dexnav
{
	After poketch
	Object src/applications/poketch/dexnav/main.c
	Object src/applications/poketch/dexnav/graphics.c
}
''')
edit('src/meson.build', "    'applications/poketch/pokemon_history/graphics.c',\n",
     "    'applications/poketch/pokemon_history/graphics.c',\n    'applications/poketch/dexnav/main.c',\n    'applications/poketch/dexnav/graphics.c',\n")

# text bank
with open('generated/text_banks.txt', 'a', encoding='utf-8', newline='\n') as f:
    f.write('TEXT_BANK_POKETCH_DEXNAV\n')
bank = {'key': 4127, 'messages': [
    {'id': 'PoketchDexNav_Text_Title', 'en_US': 'DEXNAV'},
    {'id': 'PoketchDexNav_Text_NothingHere', 'en_US': 'No Pokémon here'},
]}
with open('res/text/poketch_dexnav.json', 'w', encoding='utf-8', newline='\n') as f:
    json.dump(bank, f, ensure_ascii=False, indent=2)
    f.write('\n')

# share the DexNav area builder with the Pokétch app
edit('src/scrcmd.c', '// Fills the land and water halves of the DexNav grid from the current map\'s encounter table.\nstatic void DexNav_BuildAreaData(',
     '// Fills the land and water halves of the DexNav grid from the current map\'s encounter table.\nvoid DexNav_BuildAreaData(')
s = open('src/scrcmd.c', encoding='utf-8').read()
s = s.replace('static void DexNav_BuildAreaData(FieldSystem *fieldSystem, DexNavData *data);\n', '')
open('src/scrcmd.c', 'w', encoding='utf-8', newline='\n').write(s)
edit('include/applications/dexnav.h', 'BOOL DexNav_Init(ApplicationManager *appMan, int *state);',
     '// Fills a DexNavData from the current map\'s encounter table (also used by the DexNav Pokétch app).\n'
     'void DexNav_BuildAreaData(FieldSystem *fieldSystem, DexNavData *data);\n\n'
     'BOOL DexNav_Init(ApplicationManager *appMan, int *state);')
print('ok')
