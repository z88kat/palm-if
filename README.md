# Kronos

Kronos is an interactive fiction interpreter for Palm OS handhelds. It plays:

| Games | Interpreter | Story file names |
|---|---|---|
| Magnetic Scrolls (The Pawn, The Guild of Thieves, Jinxter, …) | Magnetic 2.2 by Niclas Karlsson | `*.mag`, with optional `*.gfx` pictures |
| Infocom and other Z-machine games | Frotz 2.43 by Stefan Jokisch | `*.dat`, `*.z3` … `*.z8` |
| Scott Adams adventures (ScottFree format) | ScottFree 1.14 by Alan Cox | `*.sa`, or `*.sa.dat` |
| Level 9 adventures (Jewels of Darkness, Knight Orc, Scapeghost, …) | [Level9](https://github.com/DavidKinder/Level9) 5.3 by Glen Summers and others | `*.l9` (see below), with optional pictures, or Spectrum snapshots `*.sna` |

It was written by Jeff Mitchell (Codejedi) in 2003. This repository holds the
1.4.1 sources, converted from CodeWarrior to the free prc-tools toolchain so
that it builds on a modern Mac, and Kronos 1.5, which adds Scott Adams and
Level 9 games.

## Using Kronos

**Requirements:** Palm OS 4.0 or later, and an expansion card. Kronos reads
its games from the card. High-resolution and
colour devices are supported.

1. Install `build/Kronos.prc` on the handheld.
2. Copy story files to `/Palm/Programs/Kronos/` on the expansion card. For
   Magnetic Scrolls pictures, put the matching `.gfx` file next to the `.mag`
   file (for example `PAWN.MAG` and `PAWN.GFX`); without it the game runs as
   text only.
3. Start Kronos, pick a game, tap **Play**, choose a font size and tap
   **Start**.

**Level 9 games** come as one data file per part, usually called
`GAMEDAT1.DAT`, `GAMEDAT2.DAT` and so on. The files of different games
would clash in one folder, so rename them to the game's name followed by the part
number, with the extension `.l9`: for example `INGRID1.L9`, `INGRID2.L9`,
`INGRID3.L9`. Start the game from part 1. When it moves to the next part,
Kronos loads the file with the next number. Level 9 Computing released its
games under the Apache License 2.0; the
[Level9-Public](https://github.com/MikeTheTechie/Level9-Public) archive has the data
files, and documentation including some of the original booklets.

Some Level 9 games ask a question from the manual (for example "the word at
page 37, line 2, word 3") when you type `restore`. The **Load** button uses the interpreter's own
`#restore` command instead, which loads the file directly and skips the
game's question, and the autosave is loaded the same way. `#save` is the
matching save command.

**Level 9 pictures** go in a folder named after the game, without the part
number: `INGRID1.L9` looks in `/Palm/Programs/Kronos/INGRID/`. Older games
have line drawings: put the picture file as `PICTURE1.DAT`, `PICTURE2.DAT`
… (one per part) or `PICTURE.DAT` (one for all parts). Later games have
bitmaps: copy the numbered picture files from the original disks as they
are (PC `1.PIC` … with `PALETTE.PIC`, Atari ST `*.SQU`, Amiga files with no
extension, and so on). The picture appears in the corner of the screen; tap
it to see it full size. Kronos adds the picture on screen to its saved
games, so it is redrawn after a restore (games only draw a room's picture
when the player arrives there).

**Ready-made Level 9 games:** `tools/level9-games.sh` builds the games from
the Level9-Public archive (cloned to `level9/Level9-Public`) and copies them,
with every picture set that matches, into `games/`; copy the contents of
`games/` to `/Palm/Programs/Kronos/`. It compiles the games that have
sources with the archive's own l9dev tools (rebuilt for macOS; each compiled
file is checked against the archive's reference copy), and otherwise uses
the released game files from the archive:

| Game | Files | Pictures |
|---|---|---|
| Jewels of Darkness | `JEWELS1-3.L9` | line drawings (Atari ST) |
| Silicon Dreams | `SILICON1-3.L9` | line drawings (Atari ST) |
| Time and Magik | `TIMEANDMAGIK1-3.L9` | |
| Knight Orc | `KNIGHTORC1-3.L9` | |
| Gnome Ranger | `GNOMERANGER1-3.L9` | |
| Lancelot | `LANCELOT1-3.L9` | |
| Ingrid's Back | `INGRID1-3.L9` (PC release) | bitmaps (PC) |
| Scapeghost | `SCAPEGHOST1-3.L9` (compiled) | bitmaps (Atari ST) |
| The Secret Diary of Adrian Mole | `SECRETDIARY1-4.L9` | |
| The Growing Pains of Adrian Mole | `GROWINGPAINS1-4.L9` | |

The Grange Murders and Billy the Kid, unreleased games in the archive, keep
their code and data in separate files, which the Level9 interpreter cannot
read.

Saved games, the automap and game notes are stored in a folder per game
next to the story file (for example `/Palm/Programs/Kronos/PAWN.dir/`). If
you switch to another application in the middle of a game, Kronos saves
the position automatically and offers to restore it the next time you
start that game.

The button bar under the text gives one-tap commands (look, compass
directions, inventory, word list, load and save). The menus also give access
to the automap, game notes, colour themes and picture zoom.

## Building

Kronos builds on macOS with the palmdev-macos build of prc-tools and the
Palm OS 5 SDK.

### Prerequisites

| What | Where the Makefile expects it | Override with |
|---|---|---|
| prc-tools (`m68k-palmos-gcc` 2.95, `arm-palmos-gcc`, `build-prc`, `m68k-palmos-multigen`) | `~/Development/palmdev-macos/toolchain` | `PALMDEV_TOOLCHAIN=…` |
| Palm OS SDK 5r3 | `/usr/local/palmdev/sdk-5r3` | `PALMDEV_SDK=…` |
| PilRC 3.2, patched for 64-bit hosts | `tools/pilrc/pilrc` | `PILRC=…` |
| Python 3 | on `PATH` | |

Stock PilRC 3.2 misreads BMP files on 64-bit macOS. `tools/build-pilrc.sh`
downloads PilRC 3.2, applies the fixes and builds it into `tools/pilrc/`:

```sh
tools/build-pilrc.sh
```

### Build

```sh
make            # builds build/Kronos.prc
make clean      # removes obj/
make distclean  # also removes build/Kronos.prc
```

The build compiles the C sources, links them into seven code resources,
checks every call between code resources (see below), builds the ARM helper,
compiles the resources with PilRC, and packs everything with `build-prc`.
Intermediate files go to `obj/`.

### Testing in an emulator

Kronos runs in [CloudpilotEmu](https://github.com/cloudpilot-emu/cloudpilot-emu).
Make a FAT-formatted SD card image with the games in
`/Palm/Programs/Kronos/`, start the emulator with `--sd <image>`, then
`install build/Kronos.prc` and `launch Kronos`. This build has been tested
on an emulated Tungsten C (Palm OS 5.2) with Wishbringer and Trinity
(Z-machine), The Pawn and The Guild of Thieves with pictures (Magnetic
Scrolls), Scott Adams adventures 1 and 14, and Ingrid's Back, Jewels of
Darkness, Silicon Dreams and Scapeghost (Level 9, with pictures): game
selection, play, pictures (drawn by the ARM helper), save, restore,
autosave, moving on to the next part of a Level 9 game and quitting back to
the game list all work. It has not yet been tried
on a real device.

The Level 9 interpreter was also checked on the Mac against the 16-bit
`int` of the Palm compiler: built once normally and once with `int`
defined as `short`, it gives identical output for the regression
walkthroughs in Level9-Public (`l9dev/regress`) and for the opening moves
of 51 games, and both draw the same line drawings for the whole Jewels of
Darkness part 1 walkthrough.

## Source layout

```
Makefile            build rules
kronos.def          application name, creator ID ('Magn'), stack size and code sections
src/                Kronos itself: user interface, display, Palm OS shims for the C library
  Starter.c         application entry point and event loop
  sections.h        code section layout and rules (read this before adding functions)
  arm/              ARM native code used to scale pictures on Palm OS 5 devices
  frotz/            Frotz Z-machine interpreter and its "dumb" front end
  level9/           Level 9 interpreter (level9.c and bitmap.c, from Level9 5.3) and its Kronos front end (l9palm.c)
  magnetic/         Magnetic Scrolls interpreter
  scottfree/        ScottFree interpreter
rsc/
  kronos.rcp        PilRC resources: forms, menus, alerts, bitmaps, icons
  KronosRsc.h       resource IDs, shared by kronos.rcp and the C sources
  bitmaps/          button bitmaps and application icons
tools/
  build-pilrc.sh    builds the patched PilRC
  check-sections.py verifies cross-section calls (run by make)
  level9-games.sh   builds the Level 9 games and pictures into games/
docs/               the original change log and readme
build/Kronos.prc    the built application
```

### Code sections

Kronos has about 145K of 68K code, but a Palm OS code resource can only call
within ±32K. The program is therefore split into seven code resources:

| Section | Contents |
|---|---|
| `.text` | start-up code, libgcc and the Frotz core |
| `kronos` | the Kronos user interface and C library shims |
| `magnetic` | the Magnetic Scrolls interpreter |
| `scott` | ScottFree and its curses shims |
| `frotzio` | Frotz screen, save/restore, error and hot-key code, and its front end |
| `level9` | the Level 9 interpreter and its front end |
| `level9b` | the Level 9 bitmap picture decoders (`bitmap.c`) |

A function in a named section must be declared with its section attribute
(`SEC_KRONOS`, `SEC_MAGNETIC`, `SEC_SCOTT`, `SEC_FROTZIO`, `SEC_LEVEL9` or `SEC_LEVEL9B`) everywhere it is
called. Static data must not point into a named section, because prc-tools
cannot relocate such pointers. Breaking either rule still links, but crashes
on the device. `tools/check-sections.py` reads the relocations of every
object file and fails the build if a call or pointer is wrong. The details
are in `src/sections.h`.

## Changes from the original 1.4.1 release

**Build**
- Builds with prc-tools and PilRC instead of CodeWarrior and Constructor.
  The Constructor resource file was converted to `rsc/kronos.rcp` and BMP
  files.
- The code is split into seven code resources, with an automatic check of
  cross-section calls (see above).
- Frotz's opcode dispatch tables are filled in at run time (`init_opcodes`)
  rather than statically, so Frotz can span more than one code resource.

**New: Level 9 games**
- The Level9 5.3 interpreter is included, with its line drawings and
  bitmap pictures (see "Level 9 pictures" above). Its upstream source is
  kept as it is apart from changes marked "Kronos", listed at the top of
  `src/level9/level9.c` and `src/level9/bitmap.c`:
  - its word-reading macros returned a signed 16-bit `int`, which
    sent words of 0x8000 and above backwards, and `CheckCallDriverV4` kept
    file offsets in an `int`, which overflows in game files over 32K (this
    broke Ingrid's Back);
  - the RAM save slots and the `#cheat` game state (about 30K) are
    allocated when a game starts, as Palm OS applications can only have
    32K of globals;
  - `RestoreGame` now closes its file;
  - pictures are only looked for in the picture file, and the next part
    of a game loads its own picture file;
  - `bitmap.c` is compiled with `int` defined as `long` (its decoders
    assume 32-bit `int`), and the Amiga palette uses a table instead of
    `pow()`.
- The launch stack is now 12K (it was 6K): the interpreter keeps a 5K game
  state on the stack while restoring.

**User interface**
- The About box lists all four interpreters and the version (now 1.5); its
  text was cut off.
- The picture view is full width, so 320-pixel pictures are shown at full
  resolution on high-resolution screens.

**Platform**
- Sony CLIÉ support was removed. Kronos targets standard Palm OS devices
  only.
- The unused preferences the CodeWarrior starter project saved under the
  creator ID `'STRT'` were removed.

**Fixes**
- ScottFree: game flags above bit 15 (including the "light out" flag) were
  lost because `1 << n` was computed in 16 bits.
- ScottFree: command words longer than nine letters could leave the verb
  or noun unterminated.
- ScottFree: a fatal error left the device frozen in an endless loop. It now
  shows the error and returns to the game list.
- Frotz: the Quetzal save code built its IFF chunk IDs and 32-bit fields in
  16 bits, writing corrupt files. Kronos still saves in Frotz's original
  format (the original author had turned Quetzal off), so existing saves
  remain compatible. Quetzal can now be switched on safely in
  `src/frotz/dumb_init.c`.
- ScottFree: rooms with multi-line descriptions put the wrong line in the
  title bar and the automap, and the room was often shown twice after an
  action. Loading a game also printed "You use word(s) I don't know!".
- ScottFree game files with DOS line endings could not be read, and a
  long number could overflow the reader's buffer.
- Files named `*.sa.dat` were treated as Z-code games.
- Magnetic Scrolls: restoring a typed save name always failed (the load
  path lacked `.sav`), and a failed open read through an invalid file
  handle, which could crash the game. Errors are now reported by the game
  ("Ouch - bad load").
- Magnetic Scrolls: moving to a room with a shorter picture left the
  previous picture's bottom rows in the thumbnail, in the wrong colours.
- `fgetc` never returned `EOF` at the end of a file.
- The interpreters' `exit()` calls (on fatal errors) now show a message and
  return to the game list. CodeWarrior's runtime used to provide `exit()`.
- The code now compiles without warnings.

## Licence

Kronos is free software under the GNU General Public License, version 2
(see `COPYING`), as are Magnetic, Frotz, ScottFree and Level9, whose licences
are included with their sources. The games themselves are copyrighted by their
publishers and are not included.

Kronos would not exist without the authors of Magnetic, Frotz, ScottFree and
Level9, nor without Infocom, Magnetic Scrolls, Adventure International and
Level 9 Computing.
