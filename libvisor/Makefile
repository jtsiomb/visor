src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)

liba = libvisor.a

CFLAGS = -pedantic -Wall -g -Iinclude

$(liba): $(obj)
	$(AR) rcs $@ $<

-include $(dep)

%.d: %.c
	@echo "dep $@"
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean
clean:
	rm -f $(obj) $(liba)

.PHONY: cleandep
cleandep:
	rm -f $(dep)
