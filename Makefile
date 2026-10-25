# Forwards every target to the firmware Makefile.
.DEFAULT_GOAL := all

%:
	$(MAKE) -C firmware $@
