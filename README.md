# openlba
## A Little Big Adventure series engine reimplementation

Community driver project to enhance the original game engine, fix bugs, add support for modern systems and multiple platforms.
It mergers multiple efforts from the community done so far but staled or unmaintained with the goal to create a single code base for all games in the series.

Little Big Adventure (aka Relentless: Twinsen's Adventure) is an action-adventure game, developed by Adeline Software International in 1994. 
Time Commando is a 1996 action-adventure not part of the LBA series but sharing the same engine utilities.
Little Big Adventure 2 (aka Twinsen's Odyssey) is the sequel released in 1997.

The main goal of this project is to support all games in a single code base having LBA2 as the base engine which has a lot of overlap with LBA1 engine, but differs from Time Commando while sharing the same LIB386 library. Each game had a different version of the LIB386.

LIB386:
- V1 - LBA1
- V2 - Time Commando
- V3 - LBA2

We also intend to support wider screen resolutions, mainly 16x10 which fits perfectly a Steam Deck screen. This codebase already have support for resolution 768x480 instead of the native 640x480 which should scale well to 1280x800. Supporting other resolutions are not a priority since it will require significant changes to the rendering engine to support hardware rendering.

## Build

> cmake -S src -B build

> cmake --build build

Default target is openlba which is based on LBA2 engine with the intention to support both LBA1 and LBA2 games.


### Build LBA1 target (WIP)
> cmake -S src -B build-lba1

> cmake --build build-lba1

### Build LBA2 target
> cmake -S src -B build-lba2

> cmake --build build-lba2


## Links:
**Official Website:** https://littlebigadventure.com/

**Discord:** https://discord.gg/gfzna5SfZ5

**Docs:** https://lbalab.github.io/lba-engine-architecture/

**Community Git:** https://github.com/lbalab

## Buy the game:
### LBA1
 [[Steam]](https://store.steampowered.com/app/397330/Little_Big_Adventure__Enhanced_Edition/?l=french)  [[GoG]](https://www.gog.com/game/little_big_adventure)  
### LBA2
 [[Steam]](https://store.steampowered.com/app/398000/Little_Big_Adventure_2/)  [[GoG]](https://www.gog.com/game/little_big_adventure_2)

### Licence
This source code is licensed under the [GNU General Public License](https://github.com/2point21/lba1-classic-community/blob/main/LICENSE).

Please note this license only applies to **Little Big Adventure 1**, **Little Big Adventure 2** engine source code. **Little Big Adventure 1** and **Little Big Adventure 2** game assets (art, models, textures, audio, etc.) are not open-source and therefore aren't redistributable.

## Copyright
The intellectual property is currently owned by [2.21]. Copyright [2.21]

Originally developed by Adeline Software International in 1994
