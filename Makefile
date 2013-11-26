BUILD_TAG	= $(shell git tag -l | tail -n 1)
BUILD_STYLE	= DEVELOPMENT

OBJECTS		= start.o printf.o bcopy.o bzero.o libc_stub.o tlsf.o \
	device_tree.o xml.o mach.o xmdt.o strcmp.o strchr.o strncmp.o strlen.o \
	malloc.o main.o debug.o bootx.o image3.o macho_loader.o memory_region.o \
	json_parser.o rdsk.o
CFLAGS		= -mcpu=cortex-a8 -std=c99 -fno-builtin -Os -fPIC -Wall -Werror -Wno-error=multichar
CPPFLAGS	= -Iinclude -D__LITTLE_ENDIAN__ -DTEXT_BASE=$(TEXT_BASE) -DBUILD_STYLE=\"$(BUILD_STYLE)\" \
		  -DBUILD_TAG=\"$(BUILD_TAG)\"
ASFLAGS		= -mcpu=cortex-a8 -DTEXT_BASE=$(TEXT_BASE) -D__ASSEMBLY__
LDFLAGS		= -nostdlib -Wl,-Tldscript.ld
TEXT_BASE	= 0x88000000
CROSS		= arm-none-eabi-
CC		= $(CROSS)gcc
AS		= $(CROSS)gcc
OBJCOPY		= $(CROSS)objcopy
TARGET		= SampleBooter.elf

SIZE		= 32768

all: $(TARGET) $(OBJECTS)

mach.o: mach.img3
	$(CROSS)ld -r -b binary -o mach.o mach.img3
	$(CROSS)objcopy --rename-section .data=.kernel mach.o mach.o

rdsk.o: rdsk.img3
	$(CROSS)ld -r -b binary -o rdsk.o rdsk.img3
	$(CROSS)objcopy --rename-section .data=.ramdisk rdsk.o rdsk.o

xmdt.o: xmdt.img3
	$(CROSS)ld -r -b binary -o xmdt.o xmdt.img3
	$(CROSS)objcopy --rename-section .data=.devicetree xmdt.o xmdt.o

$(TARGET): $(OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o version.o version.c
	$(CC) $(LDFLAGS) $(OBJECTS) version.o -o $(TARGET)  -lgcc 
	$(OBJCOPY) -g -S -O binary $(TARGET) $(TARGET).raw
	mkimage -A arm -O linux -T kernel -C none -a $(TEXT_BASE) -e $(TEXT_BASE) -n "Linux 2.6" -d $(TARGET).raw $(TARGET).uImage
#	rm -f $(TARGET) $(TARGET).raw

%.o: %.s
	$(CC) $(CFLAGS) $(ASFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET)* $(OBJECTS) version.o
