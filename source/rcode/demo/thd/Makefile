CC = gcc

CPPFLAGS += -I. -I./include

LDFLAGS += -L.

LDLIBS += -g -lpthread

DIRS = $(shell find ./ -maxdepth 3 -type d)
FILES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))
OBJS := $(subst .c,.o,$(FILES))

all: thd

thd: $(OBJS)

	$(CC) $(CPPFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

clean distclean maintainer-clean:
	rm -f *.o  ./thd