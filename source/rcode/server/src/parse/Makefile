# https://blog.csdn.net/huyansoft/article/details/8860224

CC = gcc
LEX=flex
YACC=bison

CPPFLAGS += -g -I. -I./include

LDFLAGS += -L.

LDLIBS += -lpthread -lssl

DIRS = $(shell find ./ -maxdepth 3 -type d)
FILES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))
OBJS := $(subst .c,.o,$(FILES))



pars: lex.yy.o  yacc.tab.o $(OBJS)
	$(CC) lex.yy.o yacc.tab.o -o $@
	#@./$(OBJECT) #编译后立刻运行
 
lex.yy.o: lex.yy.c  yacc.tab.h  node.h
	$(CC) -c lex.yy.c
 
yacc.tab.o: yacc.tab.c  node.h
	$(CC) -c yacc.tab.c

yacc.tab.c  yacc.tab.h: yacc.y
	$(YACC) -d yacc.y
 
lex.yy.c: lex.l
	$(LEX) lex.l

# parse: $(OBJS)
# 	$(CC) $(CPPFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

clean distclean maintainer-clean:
	rm -f *.o  ./parse $(OBJS) yacc.tab.* lex.yy.*