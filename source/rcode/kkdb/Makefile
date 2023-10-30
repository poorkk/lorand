CC = gcc

CPPFLAGS += -I. -I./include

LDFLAGS += -L.

LDLIBS += -g

DIRS = $(shell find ./ -maxdepth 3 -type d)
FILES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))
OBJS := $(subst .c,.o,$(FILES))

all: bin

bin: $(OBJS)

	$(CC) $(CPPFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

clean distclean maintainer-clean:
	rm -f *.o  $(OBJS)