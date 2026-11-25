#!/bin/bash
# Builds the Level 9 games from the Level 9 archive and copies them, with
# their pictures, into games/ in the layout Kronos reads (see README.md).
#
#   tools/level9-games.sh [path to Level9-Public]
#
# The archive (https://github.com/MikeTheTechie/Level9-Public) defaults to
# level9/Level9-Public. Its l9dev tools compile the game sources; they are
# Linux programs, so this builds them in a scratch copy with stand-ins for
# the GNU tools they expect (m68k-linux-gnu-as from prc-tools, GNU sed from
# Homebrew). The archive itself is left untouched.
#
# Needs: a C compiler, gsed (brew install gnu-sed) and the prc-tools
# m68k-palmos-as / m68k-palmos-objcopy (PALMDEV_TOOLCHAIN, as for make).

set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
ARCHIVE=$(cd "${1:-$ROOT/level9/Level9-Public}" && pwd)
TOOLCHAIN=${PALMDEV_TOOLCHAIN:-$HOME/Development/palmdev-macos/toolchain}
GAMES=$ROOT/games
WORK=$(mktemp -d "${TMPDIR:-/tmp}/l9games.XXXXXX")
trap 'rm -rf "$WORK"' EXIT

SOURCES="$ARCHIVE/game sources"
PCINGRID="$ARCHIVE/floppy disk archive/game disks/ingrids_back/ingrid00033 pc 3.5 ingrids back master disc 20-10-88"

for f in "$TOOLCHAIN/bin/m68k-palmos-as" "$TOOLCHAIN/bin/m68k-palmos-objcopy"; do
  [[ -x $f ]] || { echo "$f not found (set PALMDEV_TOOLCHAIN)"; exit 1; }
done
type -P gsed >/dev/null || { echo "gsed not found (brew install gnu-sed)"; exit 1; }

# --- stand-ins for the GNU/Linux tools l9dev's scripts call -------------
mkdir "$WORK/bin"
ln -s "$TOOLCHAIN/bin/m68k-palmos-as" "$WORK/bin/m68k-linux-gnu-as"
ln -s "$TOOLCHAIN/bin/m68k-palmos-objcopy" "$WORK/bin/m68k-linux-gnu-objcopy"
ln -s "$(type -P gsed)" "$WORK/bin/sed"
cat > "$WORK/bin/stat" <<'EOF'
#!/bin/bash
# GNU "stat -c %s file"
if [[ $1 == -c && $2 == %s ]]; then exec /usr/bin/stat -f %z "$3"; fi
exec /usr/bin/stat "$@"
EOF
chmod +x "$WORK/bin/stat"
export PATH="$WORK/bin:$PATH" LC_ALL=C

# --- build l9dev and compile the games -----------------------------------
cp -R "$ARCHIVE/l9dev" "$WORK/l9dev"
cd "$WORK/l9dev"
rm -f l9comp l9finish l9int l9menu l9squash	# the Linux builds
# (squash.c uses the GNU mempcpy; CC, not CFLAGS, keeps the Makefile's
# per-program flags)
make -s CC="cc -w '-Dmempcpy(d,s,n)=((char*)memcpy(d,s,n)+(n))'" \
  l9comp l9finish l9int l9menu l9squash

# l9tablegen halves "rept" counts because modern binutils pads each
# repeated dc.b to a word; the prc-tools assembler does not
gsed -i 's|^\( *\)sed -i -e "s/^ rept .*/2/g" \$table.s$|\1: # keep the rept count|' l9tablegen
grep -q "keep the rept count" l9tablegen || { echo "l9tablegen: rept fix not applied"; exit 1; }

# compileall builds every game with source, and compares each file with
# the reference copy kept in the archive (cmp prints any difference)
echo "Compiling the games (checking them against the archive's copies) ..."
if regress/compileall 2>&1 | grep -v "^/" | grep . ; then
  echo "Some compiled files differ from the reference copies"; exit 1
fi
COMPILED="$WORK/l9dev/regress/games.l9"
RELEASED="$WORK/l9dev/regress/data"

# --- games/ ---------------------------------------------------------------
# One file per part, NAME1.L9, NAME2.L9 ...; pictures in NAME/
mkdir -p "$GAMES"
cd "$GAMES"

released () {		# released () NAME regress-data-folder parts
  for ((i = 1; i <= $3; i++)); do
    cp "$RELEASED/$2/gamedat$i.dat" "$1$i.L9"
  done
}

# The games as released (the compiled sources are other versions of most)
released JEWELS jewels.l9 3		# Jewels of Darkness
released SILICON silicon.l9 3		# Silicon Dreams
released TIMEANDMAGIK tm.l9 3		# Time and Magik
released KNIGHTORC knight.l9 3
released GNOMERANGER gnome.l9 3
released LANCELOT lancelt.l9 3		# (the compiled part 2 does not load)
released GROWINGPAINS growing.l9 4	# The Growing Pains of Adrian Mole
released SECRETDIARY secret.l9 4	# The Secret Diary of Adrian Mole

# Scapeghost: compiled from source, the version its pictures belong to
for i in 1 2 3; do
  cp "$COMPILED/ghost.l9/part$i.l9/main.l9/gamedata.dat" "SCAPEGHOST$i.L9"
done

# Ingrid's Back: the PC release, the version its pictures belong to
for i in 1 2 3; do
  cp "$PCINGRID/GAMEDAT$i.DAT" "INGRID$i.L9"
done

# Line drawings (Atari ST picture files)
mkdir -p JEWELS SILICON
cp "$SOURCES/COLOSSAL.L9/ST.L9/PICTURE.DAT" JEWELS/PICTURE1.DAT
cp "$SOURCES/QUEST.L9/PICTURE.DAT" JEWELS/PICTURE2.DAT
cp "$SOURCES/DUNGEON.L9/PICTURE.DAT" JEWELS/PICTURE3.DAT
cp "$SOURCES/SNOWBALL.L9/ST.L9/PICTURE.DAT" SILICON/PICTURE1.DAT
cp "$SOURCES/EDEN.L9/ST.L9/PICTURE.DAT" SILICON/PICTURE2.DAT
cp "$SOURCES/WORM.L9/ST.L9/PICTURE.DAT" SILICON/PICTURE3.DAT

# Bitmaps: PC (numbered .PIC files) and Atari ST (.SQU files)
mkdir -p INGRID SCAPEGHOST
cp "$PCINGRID"/*.PIC INGRID/
cp "$SOURCES/Scapeghost PART1.L9/GHOSTPIC/"*.SQU SCAPEGHOST/

echo "Level 9 games copied to $GAMES:"
ls "$GAMES" | grep '\.L9$' | sed 's/[0-9]*\.L9$//' | sort -u | tr '\n' ' '
echo
