# name of the program (Minix service)
PROG=lab4

# source code files to be compiled
SRCS = lab4.c mouse.c

# additional compilation flags
# "-Wall -Wextra -Werror -I . -std=c11 -Wno-unused-parameter" are already set
CFLAGS += -pedantic -DLAB4 -I../lab2

# list of library dependencies
DPADD += ${LIBLCF} ${LIB_NAME} ../lab2/libtimer.a
LDADD += -llcf -L. -L../lab2
LDADD += -lmouse -ltimer

LIB_NAME = libmouse.a
LIB_SRCS = mouse.c
LIB_OBJS = ${LIB_SRCS:.c=.o}
CLEANFILES += ${LIB_NAME} ${LIB_OBJS}

all: lib
lib: ${LIB_OBJS}
	ar rcs ${LIB_NAME} ${LIB_OBJS}

# include LCOM's makefile that does all the "heavy lifting"
.include <minix.lcom.mk>
