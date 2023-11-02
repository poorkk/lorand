CC = gcc

CPPFLAGS += -I. -I$(ROOT)/include
LDFLAGS += -L.
LDLIBS += -g

DIRS += $(ROOT)/src/comm
DIRS += $(ROOT)/src/storage
DIRS = $(ROOT)/src/bin/dbinit
FILES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))
OBJS := $(subst .c,.o,$(FILES))

all: dbinit

dbinit: $(OBJS)
	echo $(CPPFLAGS)
	$(CC) $(CPPFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

clean distclean maintainer-clean:
	rm -f *.o  $(OBJS) ./dbinit