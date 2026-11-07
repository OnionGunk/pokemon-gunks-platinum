# Pokémon Gunk's Platinum Version

A quality-of-life hack of Pokémon Platinum, built on [pret's Platinum decompilation](https://github.com/pret/pokeplatinum).
It keeps the game's difficulty and story as they are and takes the grind and friction out, for long playthroughs.

## Features

- **DexNav.** Dawn (or Lucas) gives it to you on Route 202. Its screen shows each area's land and water Pokémon:
  caught in color with a Poké Ball mark, seen as a silhouette, unseen as a question mark. Pick one, then press
  **R** in the tall grass, or while surfing for a water Pokémon, to search for it. A Pokétch app shows the same
  area at a glance. Searches roll legal wild encounters, and search levels and chains raise the odds of good IVs
  and shinies.
- **Pokémon follow you**, ported from Following Platinum. All species are included, with emotes and
  interactions, and the follower returns to its Poké Ball while you Surf.
- **21 PC boxes** (18 in the original game): a living dex through Gen 4 with three boxes to spare.
- **Faster play:** instant text, a faster EXP bar, auto-run, faster egg hatching.
- **Quick Run and Quick Ball** in wild battles: press R to run, or L to throw the ball shown at the bottom
  left of the top screen. Hold L and press Left or Right to pick another ball; it remembers the last one thrown.
- **Items:** reusable TMs, two registered items (Y and Select), bag sorting, a prompt to use another Repel
  when one runs out, and an Endless Repel toggle.
- **Pokémon:** IV and EV view in the summary, Exp. All, vitamins up to 252 EVs, and trade evolutions without
  trading. HM moves work without teaching the move, and every Pokémon Center has a Move Tutor.
- **Info:** honey tree and Poké Radar details.

Save safety comes first: every build passes an emulator test suite that covers saves, power cycles, crashes
during a save, full boxes, Hall of Fame entries and damaged-save recovery.

## Play it

Releases ship a **BPS patch**, not a ROM. You need your own dump of **Pokémon Platinum (USA) Rev 1**:

```
sha1: 0862ec35b24de5c7e2dcb88c9eea0873110d755c
```

1. Download the `.bps` file from [Releases](../../releases).
2. Apply it to your ROM with [Rom Patcher JS](https://www.marcrobledo.com/RomPatcher.js/) (in the browser),
   Floating IPS, or `py tools/patch/bps.py apply <platinum.nds> <patch.bps> <out.nds>`.
3. Play the result in any DS emulator or flashcart. The patcher checks your ROM first and refuses the wrong one.

Saves carry over between releases unless the release notes say otherwise. Back up your save before updating.

**PokeDaisy users:** the companion app addresses game memory directly. Use a PokeDaisy build that lists this
exact release, or turn the companion off; a mismatched build could crash the game.

## Build it

Follow pret's [`INSTALL.md`](INSTALL.md), then run `make`. A clean build reproduces the release ROM, and
`tools/patch/bps.py make` makes the patch from it.

## Credits

- [pret](https://github.com/pret/pokeplatinum) for the Platinum decompilation this hack is built on.
- **Following Platinum** by Mikelan98 and collaborators: the follower sprites, data, interactions and logic
  are ported from it with permission.
- The DexNav idea comes from later Pokémon games and fan hacks such as Pokémon Unbound.

Pokémon is © Nintendo, Creatures Inc. and GAME FREAK inc. This is a non-commercial fan project and contains
no ROM data; you need your own legally dumped copy of the game.
