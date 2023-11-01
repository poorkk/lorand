CC = gcc

CPPFLAGS += -I. -I$(ROOT)/include

LDFLAGS += -L.

LDLIBS += -g

DIRS = $(shell find ./ -maxdepth 3 -type d)
DIRS += $(ROOT)/src/comm

FILES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))
OBJS := $(subst .c,.o,$(FILES))

all: dbinit

dbinit: $(OBJS)
	echo $(CPPFLAGS)
	$(CC) $(CPPFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

clean distclean maintainer-clean:
	rm -f *.o  $(OBJS) ./dbinit