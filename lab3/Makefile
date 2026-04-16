# name of the program (Minix service)
PROG=lab3

# source code files to be compiled
SRCS = lab3.c kbc.c utils.c

# additional compilation flags
# "-Wall -Wextra -Werror -I . -std=c11 -Wno-unused-parameter" are already set
CFLAGS += -pedantic -DLAB3 -I../lab2

# list of library dependencies
DPADD += ${LIBLCF} ${LIB_NAME} ../lab2/libtimer.a
LDADD += -llcf -L. -L../lab2
LDADD += -lkbc -ltimer

LIB_NAME = libkbc.a
LIB_SRCS = kbc.c utils.c
LIB_OBJS = ${LIB_SRCS:.c=.o}
CLEANFILES += ${LIB_NAME} ${LIB_OBJS}

all: lib
lib: ${LIB_OBJS}
	ar rcs ${LIB_NAME} ${LIB_OBJS}

# include LCOM's makefile that does all the "heavy lifting"
.include <minix.lcom.mk>
