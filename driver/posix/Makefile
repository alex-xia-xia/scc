# See LICENSE file for copyright and license details.
.POSIX:

include ../../config.mk

OBJS = scc.o

all: scc

$(OBJS): ../../inc/cc.h ../../inc/arg.h

../../lib/libcc.a:
	cd ../../lib && $(MAKE) -e

scc: $(OBJS) ../../lib/libcc.a
	$(CC) $(SCC_LDFLAGS) $(OBJS) ../../lib/libcc.a -o $@

clean:
	rm -f $(OBJS)
	rm -f scc
