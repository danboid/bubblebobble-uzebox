# Uzebox BubbleBobble Makefile
# Target ATMega644p @ 28.636 MHz (NTSC)

# MCU and clock settings
MCU = atmega644p
F_CPU = 28636360

# Toolchain
CC = avr-gcc
CXX = avr-g++
AS = avr-as
AR = avr-ar
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
RM = rm -f

# Include paths
UZEBOX_DIR = ../..
UZEBOX_KERNEL = $(UZEBOX_DIR)/kernel
UZEBOX_DEMOS = $(UZEBOX_DIR)/demos

C_INCLUDES = -I$(UZEBOX_KERNEL) -I. -Wall -Wextra

# Compilation flags
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -fsigned-char -Wunused-parameter
CFLAGS += $(C_INCLUDES) -std=gnu99 -ffunction-sections -fdata-sections
CXXFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -fsigned-char -Wunused-parameter
CXXFLAGS += $(C_INCLUDES) -std=c++11 -fno-exceptions -fno-rtti -fno-threadsafe-statics
CXXFLAGS += -fconserve-space -ffunction-sections -fdata-sections
ASFLAGS = -mmcu=$(MCU) -I$(UZEBOX_KERNEL)
LDFLAGS = -mmcu=$(MCU) -Wl,--gc-sections -Wl,-Map,$(TARGET).map,--defsym=heap_end=0x80ff

# Source files
C_SOURCES = $(wildcard src/*.c)
CXX_SOURCES = $(wildcard src/*.cpp)
DATA_FILES = $(wildcard data/*)

# Object files
OBJECTS = $(C_SOURCES:.c=.o)
OBJECTS += $(CXX_SOURCES:.cpp=.o)

# Target binary
TARGET = bubblebobble

# Default target
all: $(TARGET).hex $(TARGET).eep

# Include Uzebox kernel
include $(UZEBOX_KERNEL)/uzebox.mk

# Compile C sources
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ sources
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link
$(TARGET).elf: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -lm -o $@
	$(SIZE) $@

# Create flash ROM image
%.hex: %.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# Create EEPROM image
%.eep: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@

# Disassemble
disasm: $(TARGET).elf
	$(OBJDUMP) -d -S $< > $(TARGET).lst

# Debug info
debug: $(TARGET).elf
	$(OBJDUMP) -x -d $< > $(TARGET).dump

# Clean
clean:
	$(RM) $(OBJECTS) $(TARGET).elf $(TARGET).hex $(TARGET).eep $(TARGET).map $(TARGET).lst $(TARGET).dump

# Flash to device
flash: $(TARGET).hex
	avrdude -c usbasp -p m644p -U flash:w:$<:i

# Flash EEPROM
flash-eeprom: $(TARGET).eep
	avrdude -c usbasp -p m644p -U eeprom:w:$<:i

.PHONY: all clean flash flash-eeprom disasm debug