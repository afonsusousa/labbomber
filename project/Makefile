# name of the program (Minix service)
PROG = proj

# Tell MINIX Make where to look for your .c files
.PATH: ${.CURDIR}/src
.PATH: ${.CURDIR}/lib

# List ALL your .c files here (don't include the folder names, just the files)
SRCS = main.c game.c # Add your other files like mouse.c, keyboard.c, timer.c, vbe.c

# Include your lib folder so the compiler finds your .h files
CFLAGS += -pedantic -D__LCOM_OPTIMIZED__ -I${.CURDIR}/lib

# list of library dependencies
DPADD += ${LIBLCF}
LDADD += -llcf

# include LCOM's makefile that does all the "heavy lifting"
.include <minix.lcom.mk>