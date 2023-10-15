CC = gcc

CPPFLAGS += -g -I. -I./include

LDFLAGS += -L.

LDLIBS += -lpthread -lssl

DIRS = $(shell find ./ -maxdepth 3 -type d)
FILES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))
OBJS := $(subst .c,.o,$(FILES))

all: kdb

kdb: $(OBJS)

	$(CC) $(CPPFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

clean distclean maintainer-clean:
	rm -f *.o  ./kdb $(OBJS)