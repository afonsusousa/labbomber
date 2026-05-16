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
SRCS = keyboard.c mouse.c rtc.c timer.c bitwise.c utils.c vbe.c board_generator.c debug.c draw.c assets.c glyphs.c event_handlers.c game.c game_player.c game_map.c gui_game.c gui_menus.c gui_utils.c gui_widget_elements.c gui_widget_text_input.c gui_widget_utils.c main.c	
# Compilation flags. 
# We include all the lib folders so your #include "timer.h" statements work seamlessly.
CFLAGS += -Wall -Wextra -pedantic -D__LCOM_OPTIMIZED__ 
CFLAGS += -I${.CURDIR}/include
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