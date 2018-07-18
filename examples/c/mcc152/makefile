TOPTARGETS := all clean

SUBDIRS := input_wait input_wait_multiple read_rate toggle_output

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)

