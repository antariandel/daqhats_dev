INSTALL_DIR = $(VIRTUAL_ENV)/include/daqhats

.PHONY: all clean install

all: install
clean: ;

install:
	@install -d $(INSTALL_DIR)
	@install -D -m0644 *.h $(INSTALL_DIR)

uninstall:
	@rm -rf $(INSTALL_DIR)
