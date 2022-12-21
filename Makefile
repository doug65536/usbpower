.SUFFIXES:

define gcc_from_arch=
$(2)CC = $(1)gcc
$(2)CXX = $(1)g++
$(2)LD = $(1)ld
$(2)RANLIB = $(1)ranlib
$(2)AS = $(1)as
$(2)AR = $(1)ar
$(2)SIZE = $(1)size
$(2)CPP = $(1)cpp
$(2)GDB = $(1)gdb
$(2)NM = $(1)nm
$(2)OBJCOPY = $(1)objcopy
$(2)OBJDUMP = $(1)objdump
$(2)NM = $(1)nm
$(2)STRIP = $(1)strip
$(2)GPROF = $(1)gprof
$(2)GCOV = $(1)gcov
endef

$(eval $(call gcc_from_arch,,BUILD_))
$(eval $(call gcc_from_arch,avr-,AVRC_))
$(eval $(call gcc_from_arch,arm-none-eabi-,ARMC_))

SRCDIR ?= .
AVR_MCU = atmega32u4
AVR_MCU_FREQ = 16000000
AVRDUDE = avrdude
QEMU = qemu-system-avr
QEMU_AVR_MCU = atmega328p
QEMU_MACHINE = uno
LESS = less
MKDIR = mkdir
RM = rm
SRC_DIR = .

LANGUAGEFLAGS = -W -Wall -Wextra -Werror=return-type \
	-Werror -Wno-error=unused-function -Wno-error=unused-variable \
	-Wno-error=unused-parameter -std=c++17

# BUILD_ named things are for things that compile and run on the build machine
# For generators that run on the build machine
BUILD_CC = gcc
BUILD_CXX = g++
BUILD_CXXFLAGS = $(BUILD_COMPILEFLAGS) $(LANGUAGEFLAGS)
OUTNAMEBASE = usbtoy_blinky
OUTNAME = $(OUTNAMEBASE)-$(AVR_MCU)
QEMU_OUTNAME = $(OUTNAMEBASE)-$(QEMU_AVR_MCU)

all: $(OUTNAME).hex genfont

clean:
	$(RM) -f $(OUTNAME).hex $(QEMU_OUTNAME).elf $(OUTNAME).elf \
		$(COMBINED_OBJECTS_ALL) \
		genfont 

.PHONY: all clean flash hex fuses disassemble qemu-debug debug

AVRC_COMPILEFLAGS = -mmcu=$(AVR_MCU) -g # -flto
#CXXFLAGS = $(COMPILEFLAGS) -O0 -std=c++17 -DF_CPU=$(MCU_FREQ) 
AVRC_CXXFLAGS = $(AVRC_COMPILEFLAGS) $(LANGUAGEFLAGS)
AVRC_CXXFLAGS +=  -DF_CPU=$(AVR_MCU_FREQ)
AVRC_CXXFLAGS += -Os -flto
#AVRC_CXXFLAGS += -O0

BUILD_CXXFLAGS = $(BUILD_COMPILEFLAGS) $(BUILD_LANGUAGEFLAGS)
BUILD_CXXFLAGS += -g -Os

AVRC_SRCS = main.cc clk.cc ctx_init_avr.cc ctx_avr.S task.cc timer.cc \
	usb.cc debug.cc render.cc display-ST7735R-bitbang.S

GENFONT_SRCS = genfont.cc

BUILD_SRCS = $(GENFONT_SRCS)

include autodep.mk

$(eval $(call extract_names,AVRC,AVRC,$(AVR_MCU),0))
$(eval $(call compile_targets,AVRC,AVRC))

$(eval $(call extract_names,BUILD,BUILD,host,1))
$(eval $(call compile_targets,BUILD,BUILD))

# Font data generator

$(eval $(call extract_names,GENFONT,BUILD,host,1))

genfont: $(GENFONT_OBJECTS_ALL)
	$(BUILD_CXX) $(BUILD_CXXFLAGS) -o $@ $^ -lpng

FONT_NAMES = liberation_sans_narrow_bold_48 \
	liberation_sans_narrow_regular_16

FONT_SRCS = $(patsubst %,%.cc,$(FONT_NAMES))

$(eval $(call extract_names,FONT,FONT,$(AVR_MCU),0))

# $(info Adding rule to generate $$(AVRC_OBJDIR)/$(1).cc \
# 	from $$(SRCDIR)/textimg/$(1).sfl)
define generate_font=
$$(AVRC_OBJDIR)/$(1).cc: $$(SRCDIR)/textimg/$(1).sfl genfont
	$$(MKDIR) -p $$(@D)
	./genfont $$< > $$@
endef

$(eval $(call combine_objects))

$(foreach file,$(FONT_NAMES), \
	$(eval $(call generate_font,$(file))))

# $(foreach file,$(FONT_SOURCE_NAMES_CC), \
# 	$(info font source name cc: $(file)))

FONT_CXXFLAGS = $(AVRC_CXXFLAGS) -I$(abspath $(SRC_DIR))

$(foreach file,$(FONT_SOURCE_NAMES_CC), \
	$(eval $(call compile_extension,$(file),cc,AVRC_CXX,FONT_CXXFLAGS,AVRC_OBJDIR,AVRC_OBJDIR)))

$(OUTNAME).elf: $(OBJECTS_ALL) $(FONT_OBJECTS_ALL)
	$(AVRC_CXX) $(AVRC_CXXFLAGS) -Wl,-Map,$@.map -o $@ $^
	$(AVRC_SIZE) --format=avr --mcu=$(AVR_MCU) $@

$(OUTNAME).hex: $(OUTNAME).elf
	$(AVRC_OBJCOPY) -j .text -j .data -O ihex $< $@

hex: $(OUTNAME).hex

flash: $(OUTNAME).hex
	$(AVRDUDE) -c flip1 -p $(AVR_MCU) -U flash:w:$(OUTNAME).hex:i

fuses:
	$(AVRDUDE) -c flip1 -p $(AVR_MCU) -U lfuse:w:0x40:m -u

disassemble: $(OUTNAME).elf
	$(AVRC_OBJDUMP) -S $^

qemu-disassemble: $(QEMU_OUTNAME).elf
	$(AVRC_OBJDUMP) -S $^

qemu-debug: $(QEMU_OUTNAME).elf
	$(QEMU) -machine $(QEMU_MACHINE) -bios $^ -s -S

debug: $(QEMU_OUTNAME).elf
	$(AVRC_GDB) $^ -iex 'target extended-remote :1234' \
		$(GDBFLAGS) \
		-ex 'b __vector_21' \
		-ex 'b __vector_13' \
		-ex 'b __vector_11' \
		-ex 'b __vector_16'

-include $(DEPFILES)
