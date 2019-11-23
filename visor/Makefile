src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)
bin = visor

vidir = ../libvisor

CFLAGS = -pedantic -Wall -g -I$(vidir)/include
LDFLAGS = -L$(vidir) -lvisor

$(bin): $(obj) $(vidir)/libvisor.a
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

%.d: %.c
	@echo dep $@
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)
