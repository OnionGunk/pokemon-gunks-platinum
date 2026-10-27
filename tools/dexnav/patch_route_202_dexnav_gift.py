"""Dawn (or Lucas) gives the player the DexNav right after the catching tutorial on Route 202."""
import json
import os

os.chdir('C:/Projects/pokemon-platinum-dexnav/pkplat-dexnav')


def edit(p, old, new):
    s = open(p, encoding='utf-8').read()
    assert s.count(old) == 1, (p, old[:70], s.count(old))
    open(p, 'w', encoding='utf-8', newline='\n').write(s.replace(old, new))


TEXT = {
    'Route202_Text_DawnAlsoTakeThisDexNav': [
        'Dawn: Oh! I almost forgot!\n',
        'Professor Rowan wanted you to have\r',
        'this, too!\r',
    ],
    'Route202_Text_LucasAlsoTakeThisDexNav': [
        'Lucas: Oh, right! Before I forget...\n',
        'The Professor asked me to give you\r',
        'this, too.\r',
    ],
    'Route202_Text_DexNavExplanation': [
        'That’s a DexNav! It shows which\n',
        'Pokémon live wherever you are.\r',
        'Pick a Pokémon you’ve seen, then press\n',
        'R in tall grass to search for it.\r',
        'Sneak up on the shaking grass, and\n',
        'you’ll find the Pokémon hiding there!\r',
        'The more you search for one kind of\n',
        'Pokémon, the better the ones you find.\r',
        'Once you get a Pokétch, there’s\n',
        'a DexNav app for it, too!\r',
    ],
}

p = 'res/text/route_202.json'
bank = json.load(open(p, encoding='utf-8'))
ids = {m['id'] for m in bank['messages']}
for msg_id, lines in TEXT.items():
    if msg_id not in ids:
        bank['messages'].append({'id': msg_id, 'en_US': lines})
with open(p, 'w', encoding='utf-8', newline='\n') as f:
    json.dump(bank, f, ensure_ascii=False, indent=2)
    f.write('\n')

edit('res/field/scripts/scripts_route_202.s', '''Route202_GivePokeballs:
    SetVar VAR_0x8004, ITEM_POKE_BALL
    SetVar VAR_0x8005, 5
    Common_GiveItemQuantity
    GetPlayerGender VAR_RESULT
    GoToIfEq VAR_RESULT, GENDER_MALE, Route202_DawnLeave
    GoToIfEq VAR_RESULT, GENDER_FEMALE, Route202_LucasLeave
    End
''', '''Route202_GivePokeballs:
    SetVar VAR_0x8004, ITEM_POKE_BALL
    SetVar VAR_0x8005, 5
    Common_GiveItemQuantity
    GetPlayerGender VAR_RESULT
    GoToIfEq VAR_RESULT, GENDER_MALE, Route202_DawnGiveDexNav
    GoToIfEq VAR_RESULT, GENDER_FEMALE, Route202_LucasGiveDexNav
    End

Route202_DawnGiveDexNav:
    Message Route202_Text_DawnAlsoTakeThisDexNav
    GoTo Route202_GiveDexNav

Route202_LucasGiveDexNav:
    Message Route202_Text_LucasAlsoTakeThisDexNav
    GoTo Route202_GiveDexNav

Route202_GiveDexNav:
    SetVar VAR_0x8004, ITEM_DEXNAV
    SetVar VAR_0x8005, 1
    Common_GiveItemQuantity
    RegisterPoketchApp POKETCH_APPID_DEXNAV
    Message Route202_Text_DexNavExplanation
    GetPlayerGender VAR_RESULT
    GoToIfEq VAR_RESULT, GENDER_MALE, Route202_DawnLeave
    GoToIfEq VAR_RESULT, GENDER_FEMALE, Route202_LucasLeave
    End
''')
print('ok')
