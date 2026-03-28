# Compatible with BSD make
# On Linux run with bmake instead of make

.POSIX:

.if exists(config.mk)
.include "config.mk"
.endif

CC?= cc
BUILDIR?= build

PROG_CLI= ${BUILDIR}/cli/refpersys
OBJS_CLI= ${BUILDIR/cli/main.o

.PHONY: all clean
.SUFFIXES: .c .o

all: ${PROG_CLI}

clean:
	@rm -rf ${BUILDIR}

${BUILDIR}/cli/main.o: src/cli/main.c
	@mkdir -p ${BUILDIR}/cli
	${CC} -o $@ -c src/cli/main.c
