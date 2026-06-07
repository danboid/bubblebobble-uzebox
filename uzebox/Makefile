###############################################################################
# Makefile for BubbleBobble Uzebox port
###############################################################################

PROJECT = BubbleBobble
GAME   = BubbleBobble
MCU    = atmega644
TARGET = $(GAME).elf
CC     = avr-gcc
INFO   = ../gameinfo.properties

## Kernel settings
KERNEL_DIR    = ../../../kernel
KERNEL_OPTIONS = -DVIDEO_MODE=3
KERNEL_OPTIONS += -DSCROLLING=0
KERNEL_OPTIONS += -DVRAM_TILES_H=32
KERNEL_OPTIONS += -DVRAM_TILES_V=28
KERNEL_OPTIONS += -DSCREEN_TILES_H=32
KERNEL_OPTIONS += -DSCREEN_TILES_V=28
KERNEL_OPTIONS += -DRESOLUTION_EXT=1
KERNEL_OPTIONS += -DRAM_TILES_COUNT=4
KERNEL_OPTIONS += -DMAX_SPRITES=8
KERNEL_OPTIONS += -DINTRO_LOGO=0
KERNEL_OPTIONS += -DMIXER_CHAN4_TYPE=1

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DF_CPU=28636360UL -Os -fsigned-char -ffunction-sections
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d
CFLAGS += $(KERNEL_OPTIONS)

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,-Map=$(GAME).map
LDFLAGS += -Wl,-gc-sections

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom
HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## Objects that must be built in order to link
KERNEL_OBJECTS = uzeboxVideoEngineCore.o uzeboxSoundEngineCore.o uzeboxCore.o uzeboxSoundEngine.o uzeboxVideoEngine.o

## Game source files
GAME_SOURCES = $(wildcard src/*.c)
GAME_OBJECTS = $(GAME_SOURCES:src/%.c=%.o)

OBJECTS = $(KERNEL_OBJECTS) $(GAME_OBJECTS)

## Include Directories
INCLUDES = -I"$(KERNEL_DIR)" -I"src" -I"data"

## Build
all: $(TARGET) $(GAME).hex $(GAME).eep $(GAME).lss size

## Compile Kernel files
uzeboxVideoEngineCore.o: $(KERNEL_DIR)/uzeboxVideoEngineCore.s
	$(CC) $(INCLUDES) $(ASMFLAGS) -c $<

uzeboxSoundEngineCore.o: $(KERNEL_DIR)/uzeboxSoundEngineCore.s
	$(CC) $(INCLUDES) $(ASMFLAGS) -c $<

uzeboxCore.o: $(KERNEL_DIR)/uzeboxCore.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

uzeboxSoundEngine.o: $(KERNEL_DIR)/uzeboxSoundEngine.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

uzeboxVideoEngine.o: $(KERNEL_DIR)/uzeboxVideoEngine.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

## Compile game sources
%.o: src/%.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

## Link
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -lm -o $@

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS) $< $@

%.eep: $(TARGET)
	avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: $(TARGET)
	avr-size -A $(TARGET)

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) $(GAME).* dep/*

## Dependencies
-include $(shell mkdir -p dep 2>/dev/null) $(wildcard dep/*)