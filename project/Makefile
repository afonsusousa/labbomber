# Name of the executable program
PROG = proj

# Tell Make where to look for .c files
.PATH: ${.CURDIR}/src
.PATH: ${.CURDIR}/lib/keyboard
.PATH: ${.CURDIR}/lib/mouse
.PATH: ${.CURDIR}/lib/rtc
.PATH: ${.CURDIR}/lib/timer
.PATH: ${.CURDIR}/lib/utils
.PATH: ${.CURDIR}/lib/vbe

# List EVERY .c file in your project here (no folder paths, just the names)
SRCS = main.c game.c keyboard.c mouse.c rtc.c timer.c bitwise.c utils.c vbe.c draw.c widget.c gui.c board_generator.c

# Compilation flags. 
# We include all the lib folders so your #include "timer.h" statements work seamlessly.
CFLAGS += -Wall -Wextra -pedantic -D__LCOM_OPTIMIZED__ 
CFLAGS += -I${.CURDIR}/src
CFLAGS += -I${.CURDIR}/lib/keyboard
CFLAGS += -I${.CURDIR}/lib/mouse
CFLAGS += -I${.CURDIR}/lib/rtc
CFLAGS += -I${.CURDIR}/lib/timer
CFLAGS += -I${.CURDIR}/lib/utils
CFLAGS += -I${.CURDIR}/lib/vbe

# Minix LCF Library dependencies
DPADD += ${LIBLCF}
LDADD += -llcf

# Include LCOM's framework
.include <minix.lcom.mk>