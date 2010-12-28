###############################################################################
# Makefile for the project VFD
###############################################################################

## General Flags
PROJECT = VFD
MCU = atmega88p
TARGET = VFD.elf
CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99    -ffunction-sections   -fdata-sections   -Wstrict-prototypes     -fno-inline-small-functions   -fno-split-wide-types   -fno-tree-scev-cprop    -DF_CPU=18432000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,--relax   -Wl,--gc-sections -Wl,-Map=VFD.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Libraries
LIBS = -lc -lm 

## Objects that must be built in order to link
OBJECTS = VFD.o timeout.o usart.o vfd_fsm.o boost.o max6921.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) VFD.hex VFD.eep VFD.lss size

## Compile
VFD.o: ../VFD.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

timeout.o: ../timeout.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usart.o: ../usart.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

vfd_fsm.o: ../vfd_fsm.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

boost.o: ../boost.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

max6921.o: ../max6921.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) VFD.elf dep/* VFD.hex VFD.eep VFD.lss VFD.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

