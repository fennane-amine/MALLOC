CFLAGS = -Wall -Werror -Wextra -pedantic -std=c99 -g -fPIC\
         -fvisibility=hidden -fno-builtin
LDFLAGS = -shared
TARGET_LIB = libmalloc.so
OBJS = malloc.o
VPATH = src/
.PHONY: all ${TARGET_LIB} clean

${TARGET_LIB}: ${OBJS}
	${CC} ${LDFLAGS} -o $@ $^

all: ${TARGET_LIB}

clean:
	${RM} ${TARGET_LIB} ${OBJS}

check:
	./tests/testing.sh
