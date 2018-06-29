CC = gcc
INCLUDE_DIR = /usr/local/include
LIB_DIR = ../lib
CFLAGS = -I$(INCLUDE_DIR) -I$(LIB_DIR) -g
OFLAGS = -ldaqhats 
DEPS = $(INCLUDE_DIR)/mcc118.h $(INCLUDE_DIR)/daqhats.h $(LIB_DIR)/mcc118_update.h
INSTALL_DIR = /usr/local/bin

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

mcc118_firmware_update: mcc118_firmware_update.o
	$(CC) -o $@ $^ $(OFLAGS)

.PHONY: clean

all: mcc118_firmware_update

install:
	@install -D mcc118_firmware_update $(INSTALL_DIR)
	@install -D daqhats_read_eeproms $(INSTALL_DIR)

uninstall:
	@rm -f $(INSTALL_DIR)/mcc118_firmware_update
	@rm -f $(INSTALL_DIR)/daqhats_read_eeproms

.DEFAULT_GOAL := all

clean:
	@rm -f *.o *~ core mcc118_firmware_update


