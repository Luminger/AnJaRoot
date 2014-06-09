.PHONY: delegate
.DEFAULT: delegate

delegate:
	$(MAKE) -C $(CURDIR)/app/src/main $(MAKECMDGOALS) $(MAKEFLAGS)
