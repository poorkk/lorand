CC = gcc
CPPFLAGS += -I.
LDFLAGS += -L.
LDLIBS += -g

S_OBJS = server.o
C_OBJS = client.o


all: server client
server: $(S_OBJS)
	$(CC) $(CPPFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@
client: $(C_OBJS)
	$(CC) $(CPPFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@


clean distclean maintainer-clean:
	rm -f *.o 
	rm -f server client