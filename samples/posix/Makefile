# Makefile for XBee Driver on POSIX platforms

# Autodepend methods from http://make.paulandlesley.org/autodep.html

# If you get a "no rule to make target" error for some random .h file, try
# deleting all .d files.

# extra defines
DEFINE = -DPOSIX \
	-DZCL_ENABLE_TIME_SERVER \
	-DXBEE_DEVICE_ENABLE_ATMODE \
	-DXBEE_XMODEM_TESTING

# directory for driver
DRIVER = ../..

# path to include files
INCDIR = $(DRIVER)/include

# path to common source files
SRCDIR = $(DRIVER)/src

LIBS = -lpthread

# compiler parameters for building each file
# -MMD generates dependency files automatically, omitting system files
# -MP creates phony targets for each prerequisite in a .d file

COMPILE = gcc -iquote$(INCDIR) -Wall $(DEFINE)

EXE = transparent_terminal

all : $(EXE)

# strip debug information from executables
strip :
	strip $(EXE)

zcltime : zcltime.o
	$(COMPILE) -o $@ $^

clean :
	- rm -f *.o *.d $(EXE)

SRCS = \
	atinter.c \
	transparent_terminal.c \
	xbee_term.c _xbee_term.c xbee_term_posix.c \
	zcltime.c \
	parse_serial_args.c \
	_atinter.c \
	xbee_platform_posix.c \
	xbee_serial_posix.c \
	xbee_atcmd.c \
	xbee_atmode.c \
	xbee_cbuf.c \
	xbee_device.c \
	xbee_discovery.c \
	xbee_ebl_file.c \
	xbee_firmware.c \
	xbee_gpm.c \
	xbee_ipv4.c \
	xbee_ota_client.c \
	xbee_platform_posix.c \
	xbee_readline.c \
	xbee_scan.c \
	xbee_serial_posix.c \
	xbee_sms.c \
	xbee_time.c \
	xbee_transparent_serial.c \
	xbee_tx_status.c \
	xbee_wifi.c \
	xbee_xmodem.c \
	zcl_basic.c \
	zcl_client.c \
	zcl_identify.c \
	zcl_time.c \
	zcl_types.c \
	zigbee_zcl.c \
	zigbee_zdo.c \
	wpan_types.c \
	xmodem_crc16.c \
	hexstrtobyte.c \
	swapbytes.c \
	memcheck.c \
	hexdump.c \
	xbee_readline.c \
	runner.c \
	list.c


base_OBJECTS = xbee_platform_posix.o xbee_serial_posix.o hexstrtobyte.o \
					memcheck.o swapbytes.o swapcpy.o hexdump.o parse_serial_args.o

util_OBJECTS = list.o runner.o

xbee_OBJECTS = $(base_OBJECTS) xbee_device.o xbee_atcmd.o wpan_types.o

wpan_OBJECTS = $(xbee_OBJECTS) wpan_aps.o xbee_wpan.o

zigbee_OBJECTS = $(wpan_OBJECTS) zigbee_zcl.o zigbee_zdo.o zcl_types.o

atinter_OBJECTS = xbee_readline.o _atinter.o

atinter : $(xbee_OBJECTS) $(atinter_OBJECTS) atinter.o
	$(COMPILE) -o $@ $^

apply_profile : $(xbee_OBJECTS) apply_profile.o
	$(COMPILE) -o $@ $^

transparent_terminal_OBJECTS = $(zigbee_OBJECTS) $(util_OBJECTS) transparent_terminal.o \
	$(atinter_OBJECTS) _nodetable.o \
	xbee_discovery.o \
	xbee_transparent_serial.o

transparent_terminal : $(transparent_terminal_OBJECTS)
	$(COMPILE) -o $@ $^ $(LIBS)

# to build a .o file, use the .c file in the current dir...
.c.o :
	$(COMPILE) -c $<

# ...or in common samples directory...
%.o : ../common/%.c
	$(COMPILE) -c $<

# ...or in SRCDIR...
%.o : $(SRCDIR)/%.c
	$(COMPILE) -c $<

# ...or in a given subdirectory of SRCDIR.
%.o : $(SRCDIR)/xbee/%.c
	$(COMPILE) -c $<
%.o : $(SRCDIR)/wpan/%.c
	$(COMPILE) -c $<
%.o : $(SRCDIR)/zigbee/%.c
	$(COMPILE) -c $<
%.o : $(SRCDIR)/posix/%.c
	$(COMPILE) -c $<
%.o : $(SRCDIR)/util/%.c
	$(COMPILE) -c $<
