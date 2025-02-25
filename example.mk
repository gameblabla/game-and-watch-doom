TARGETS       += $(ELF_TARGET) $(BIN_TARGET)
PREFIX         = v810
CC             = $(PREFIX)-gcc
AS             = $(PREFIX)-as
AR             = $(PREFIX)-ar
LD             = $(PREFIX)-ld
OBJCOPY        = $(PREFIX)-objcopy

CFLAGS        += -DPLATFORM=1 -Iinclude/pcfx -I. -I$(V810GCC)/include/ -I$(V810GCC)/$(PREFIX)/include/ -O2 -Werror=vla -Wall -Wextra -std=gnu99 -mv810 -msda=256 -mprolog-function 
CFLAGS        += -Isrc -I/usr/include/SDL -ICore/Inc -ICore/chocdoom -ICore/Src -DPCFX

CPFLAGS       += -I$(V810GCC)/include/ -I$(V810GCC)/$(PREFIX)/include/ -O2 -Werror=vla -Wall -std=gnu++11 -fno-rtti -fno-exceptions -mv810 -msda=256 -mprolog-function 
LDFLAGS       += -L$(V810GCC)/lib/ -L$(V810GCC)/$(PREFIX)/lib/ -L$(V810GCC)/lib/gcc/$(PREFIX)/4.9.4/ $(V810GCC)/$(PREFIX)/lib/crt0.o

LIBS           = -leris -lgcc -lm -lc -lsim

.PHONY: all cd clean install .FORCE

all: $(OBJECTS) $(TARGETS)

$(CD_OBJECTS): .FORCE lbas.h

%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@
%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@
%.o: %.c
	$(CC) $(CFLAGS) $< -S -o $*.source
	$(CC) $(CFLAGS) $< -c -o $@
%.o: %.cpp
	$(PREFIX)-g++ $(CPFLAGS) $< -S -o $*.source
	$(PREFIX)-g++ $(CPFLAGS) $< -c -o $@
%.elf: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@ -Map $*.map
%.bin: %.elf
	$(OBJCOPY) -O binary $< $@
cd: $(TARGETS)
	bincat out.bin lbas.h $(BIN_TARGET) $(ADD_FILES)
	make -f Makefile.pcfx cdclean -C .
	make -f Makefile.pcfx all -C .
	bincat out.bin lbas.h $(BIN_TARGET) $(ADD_FILES)
	pcfx-cdlink cdlink.txt $(CDOUT)

	
lbas.h:
	bincat out.bin lbas.h $(BIN_TARGET) $(ADD_FILES)

clean:
	rm -rf $(OBJECTS) $(TARGETS) lbas.h out.bin $(CDOUT).cue $(CDOUT).bin *.source *.map

cdclean:
	rm -rf $(OBJECTS) $(TARGETS) $(CDOUT).cue $(CDOUT).bin
