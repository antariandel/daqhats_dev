CC = gcc
INCLUDE_DIR = $(VIRTUAL_ENV)/include/daqhats
LIB_DIR = ../lib
CFLAGS = -I$(INCLUDE_DIR) -I$(LIB_DIR) -g
OFLAGS = $(VIRTUAL_ENV)/lib/libdaqhats.so 
DEPS = $(INCLUDE_DIR)/mcc118.h $(INCLUDE_DIR)/daqhats.h $(LIB_DIR)/mcc118_update.h
INSTALL_DIR = $(VIRTUAL_ENV)/bin

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

mcc118_firmware_update: mcc118_firmware_update.o
	$(CC) -o $@ $^ $(OFLAGS)

daqhats_list_boards: daqhats_list_boards.o
	$(CC) -o $@ $^ $(OFLAGS)
    
.PHONY: clean

all: mcc118_firmware_update daqhats_list_boards

install:
	@install -D mcc118_firmware_update $(INSTALL_DIR)
	@install -D daqhats_read_eeproms $(INSTALL_DIR)
	@install -D daqhats_list_boards $(INSTALL_DIR)

uninstall:
	@rm -f $(INSTALL_DIR)/mcc118_firmware_update
	@rm -f $(INSTALL_DIR)/daqhats_read_eeproms
	@rm -f $(INSTALL_DIR)/daqhats_list_boards

.DEFAULT_GOAL := all

clean:
	@rm -f *.o *~ core mcc118_firmware_update


