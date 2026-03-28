C?=		cc
CFLAGS+=	-Wall
SRCS= 		log.c mem.c src.c tiny-json.c
OBJS= 		${SRCS:S,c$,o,}

.SUFFIXES: .c .o
.c.o: lib.h tiny-json.h
	${CC} ${CFLAGS} -o $@ -c $<

.PHONY: all
all: ${OBJS}

