NAME := scope
# change this to yours
TTY := /dev/cu.usbserial-0001

# check that your path is defined.
ifndef CS140E_2021_PATH
$(error CS140E_2021_PATH is not set: this should contain the absolute path to where this directory is.  Define it in your shell's initialiation.  For example, .tcshrc for tcsh or .bashrc for bash)
endif

include $(CS140E_2021_PATH)/libpi/includes.mk
CFLAGS += -Wno-unused-function -Wno-unused-variable -falign-functions=4 

# add all the .h's as dependencies, libpi and the makefile.
DEPS := $(MEMMAP) $(LPI) $(wildcard ./*.h) ./Makefile  $(wildcard ../test-gen/*.h)

LIBS = $(LPI)  

all: libs $(NAME).bin run

libs: 
	make -C $(LPP)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@
%.o: %.S
	$(CC) -c $(CPP_ASFLAGS) $< -o $@

%.elf: %.o $(OBJS) $(MEMMAP) $(DEPS) 
	$(LD) $(START) $< $(OBJS) -T $(LPP)/memmap -o $@  $(LIBS)

%.list: %.elf
	$(OD) -D   $*.elf > $*.list
%.bin: %.list
	$(OCP) $*.elf -O binary $*.bin

run: $(NAME).bin
	my-install $(TTY) $<

tags: 
	ctags *.[chSs] $(LPP)/*.[ch] $(LPP)/*.[chSs]

clean:
	rm -rf $(NAME) *~ tags *.o *.bin *.elf *.list *.img Makefile.bak  *.d

.PHONY: libs all tags clean run no-uart

# stop make from deleting these suffix'd files.
.PRECIOUS: %.list %.elf
