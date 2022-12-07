MCU = atmega32u4
MCU_FREQ = 16000000
CC = avr-gcc
CXX = avr-g++
LD = avr-ld
RANLIB = avr-ranlib
AS = avr-as
AR = avr-ar
SIZE = avr-size
CPP = avr-cpp
GDB = avr-gdb
NM = avr-nm
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
NM = avr-nm
STRIP = avr-strip
GPROF = avr-gprof
GCOV = avr-gcov
AVRDUDE = avrdude
QEMU = qemu-system-avr
QEMU_MCU = atmega328p
QEMU_MACHINE = uno
SRC_DIR = .

OUTNAMEBASE = usbtoy_blinky
OUTNAME = $(OUTNAMEBASE)-$(MCU)
QEMU_OUTNAME = $(OUTNAMEBASE)-$(QEMU_MCU)

all: $(OUTNAME).hex

clean:
	rm -f $(OUTNAME).hex $(OUTNAME).elf $(OBJECTS_ALL)

.PHONY: all clean flash hex fuses disassemble qemu-debug debug

COMPILEFLAGS = -mmcu=$(MCU) -g # -flto
#CXXFLAGS = $(COMPILEFLAGS) -O0 -std=c++17 -DF_CPU=$(MCU_FREQ) 
CXXFLAGS = $(COMPILEFLAGS) \
	-W -Wall -Wextra -Werror=return-type \
	-Werror -Wno-error=unused-function -Wno-error=unused-variable \
	-std=c++17 -DF_CPU=$(MCU_FREQ)
CXXFLAGS += -Os -flto
#CXXFLAGS += -O0

SRCS = main.cc clk.cc ctx.S task.cc timer.cc debug.cc

include autodep.mk

$(OUTNAME).elf: $(OBJECTS_ALL)
	$(CXX) $(COMPILEFLAGS) -o $@ $^
	$(SIZE) --format=avr --mcu=$(MCU) $@

$(OUTNAME).hex: $(OUTNAME).elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

hex: $(OUTNAME).hex

flash: $(OUTNAME).hex
	$(AVRDUDE) -c flip1 -p $(MCU) -U flash:w:$(OUTNAME).hex:i

fuses:
	$(AVRDUDE) -c flip1 -p $(MCU) -U lfuse:w:0x40:m -u

disassemble: $(QEMU_OUTNAME).elf
	$(OBJDUMP) -S $^

qemu-debug: $(QEMU_OUTNAME).elf
	$(QEMU) -machine $(QEMU_MACHINE) -bios $^ -s -S

debug: $(QEMU_OUTNAME).elf
	$(GDB) $^ -iex 'target extended-remote :1234'

-include $(DEPFILES)
