# Kronos - interactive fiction interpreter for Palm OS
#
# Builds build/Kronos.prc with prc-tools (m68k-palmos-gcc 2.95) and PilRC.
# See README.md for the toolchain set-up.

APPNAME   = Kronos
CREATOR   = Magn

# --- Toolchain locations (override on the command line if needed) ----
PALMDEV_TOOLCHAIN ?= $(HOME)/Development/palmdev-macos/toolchain
PALMDEV_SDK       ?= /usr/local/palmdev
PILRC             ?= tools/pilrc/pilrc

export GCC_EXEC_PREFIX = $(PALMDEV_TOOLCHAIN)/lib/gcc-lib/

CC       = $(PALMDEV_TOOLCHAIN)/bin/m68k-palmos-gcc
MULTIGEN = $(PALMDEV_TOOLCHAIN)/bin/m68k-palmos-multigen
BUILDPRC = $(PALMDEV_TOOLCHAIN)/bin/build-prc
OBJDUMP  = $(PALMDEV_TOOLCHAIN)/bin/m68k-palmos-objdump
ARMCC    = $(PALMDEV_TOOLCHAIN)/bin/arm-palmos-gcc
ARMLD    = $(PALMDEV_TOOLCHAIN)/bin/arm-palmos-ld
ARMCOPY  = $(PALMDEV_TOOLCHAIN)/bin/arm-palmos-objcopy
ARMDUMP  = $(PALMDEV_TOOLCHAIN)/bin/arm-palmos-objdump

ifeq ($(wildcard $(CC)),)
  $(error m68k-palmos-gcc not found in $(PALMDEV_TOOLCHAIN)/bin - set PALMDEV_TOOLCHAIN)
endif

OBJDIR = obj
BUILD  = build

CFLAGS = -O2 -Wall -palmos5r3 -Isrc -Isrc/magnetic -Isrc/frotz -Isrc/scottfree -Isrc/level9 -Irsc

CORE_SRC  = src/Starter.c src/MainForm.c src/adapter.c src/display.c src/curses.c \
            src/kronos.c src/map.c src/mapper.c src/note.c src/pic.c \
            src/picker.c src/stdfile.c
MAG_SRC   = src/magnetic/emu.c
SCOTT_SRC = src/scottfree/ScottCurses.c
L9_SRC    = src/level9/level9.c src/level9/l9palm.c src/level9/bitmap.c
L9_CFLAGS = -DKRONOS
# level9.c and bitmap.c are upstream code; quieten their harmless style warnings
L9_WARN   = -Wno-missing-braces -Wno-parentheses -Wno-uninitialized -Wno-switch
FROTZ_SRC = $(addprefix src/frotz/, buffer.c err.c fastmem.c files.c getopt.c \
            hotkey.c input.c main.c math.c object.c process.c quetzal.c \
            random.c redirect.c screen.c sound.c stream.c table.c text.c \
            variable.c dumb_init.c dumb_input.c dumb_output.c dumb_pic.c)

SRC  = $(CORE_SRC) $(MAG_SRC) $(SCOTT_SRC) $(FROTZ_SRC) $(L9_SRC)
OBJS = $(patsubst src/%.c,$(OBJDIR)/%.o,$(SRC))

HEADERS = $(wildcard src/*.h src/*/*.h rsc/*.h)

PRC = $(BUILD)/$(APPNAME).prc

all: $(PRC)

objs: $(OBJS)

# Every object depends on every header: the project is small and a missing
# rebuild after a section attribute changes produces a crashing binary.
# (listed first: make 3.81 uses the first matching pattern rule)
$(OBJDIR)/level9/%.o: src/level9/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(L9_CFLAGS) -c $< -o $@

$(OBJDIR)/level9/level9.o $(OBJDIR)/level9/bitmap.o: CFLAGS += $(L9_WARN)

$(OBJDIR)/%.o: src/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# --- Multiple code sections (see src/sections.h) ----------------------
$(OBJDIR)/kronos-sections.s $(OBJDIR)/kronos-sections.ld: kronos.def
	@mkdir -p $(OBJDIR)
	cd $(OBJDIR) && $(MULTIGEN) ../kronos.def

$(OBJDIR)/kronos-sections.o: $(OBJDIR)/kronos-sections.s
	$(CC) -c $< -o $@

# Link, then verify every cross-section reference.
$(OBJDIR)/kronos: $(OBJS) $(OBJDIR)/kronos-sections.o $(OBJDIR)/kronos-sections.ld
	$(CC) -palmos5r3 -o $@ $(OBJS) $(OBJDIR)/kronos-sections.o \
	    $(OBJDIR)/kronos-sections.ld -Wl,-Map,$(OBJDIR)/kronos.map
	python3 tools/check-sections.py $(OBJDUMP) $(OBJS)

# --- ARM native helper used by the picture viewer (resource ARMC 1001) ----
# Position independent, no globals: it must link to a bare .text section.
SDKINC    = $(PALMDEV_SDK)/sdk-5r3/include
ARMCFLAGS = -O3 -funroll-all-loops -Wall -nostartfiles -nostdlib -fno-builtin \
            -fPIC -ffixed-r8 -ffixed-r9 -mpic-register=r10 -msingle-pic-base \
            -fsigned-char -I$(SDKINC) -I$(SDKINC)/Core -I$(SDKINC)/Core/System \
            -I$(SDKINC)/Core/Hardware

$(OBJDIR)/arm/picview.o: src/arm/picview.c $(wildcard src/arm/*.h)
	@mkdir -p $(dir $@)
	$(ARMCC) $(ARMCFLAGS) -c $< -o $@

$(OBJDIR)/arm/picview.elf: $(OBJDIR)/arm/picview.o src/arm/armletldscript
	$(ARMLD) --section-start .text=0 -T src/arm/armletldscript -o $@ $<
	@$(ARMDUMP) -h $@ | awk '$$2 ~ /^\.(data|bss|got)$$/ && $$3 != "00000000" \
	    { print "picview: unexpected " $$2 " section"; bad = 1 } END { exit bad }'

$(OBJDIR)/picview.bin: $(OBJDIR)/arm/picview.elf
	$(ARMCOPY) -j .text -O binary $< $@

# --- Resources -----------------------------------------------------------
$(OBJDIR)/resources.stamp: rsc/kronos.rcp rsc/KronosRsc.h $(wildcard rsc/bitmaps/*.bmp) \
                           $(OBJDIR)/picview.bin $(PILRC)
	rm -rf $(OBJDIR)/res && mkdir -p $(OBJDIR)/res
	$(PILRC) -q -I rsc -I $(OBJDIR) rsc/kronos.rcp $(OBJDIR)/res
	touch $@

$(PILRC):
	@echo "PilRC not found at $(PILRC): run tools/build-pilrc.sh" >&2; exit 1

# --- The application -----------------------------------------------------
$(PRC): $(OBJDIR)/kronos $(OBJDIR)/resources.stamp kronos.def
	@mkdir -p $(BUILD)
	$(BUILDPRC) -o $@ kronos.def $(OBJDIR)/kronos $(OBJDIR)/res/*.bin
	@ls -l $@

clean:
	rm -rf $(OBJDIR)

distclean: clean
	rm -f $(PRC)

.PHONY: all objs clean distclean
.DELETE_ON_ERROR:
