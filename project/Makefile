# Name of the executable program
PROG = proj

# Build directory (BSD make)
BUILD_DIR ?= build
BUILD_DIR_PATH != mkdir -p ${.CURDIR}/${BUILD_DIR} && echo ${.CURDIR}/${BUILD_DIR}
.OBJDIR: ${BUILD_DIR_PATH}

.PATH: ${.CURDIR}/src
.PATH: ${.CURDIR}/lib/keyboard
.PATH: ${.CURDIR}/lib/mouse
.PATH: ${.CURDIR}/lib/rtc
.PATH: ${.CURDIR}/lib/timer
.PATH: ${.CURDIR}/lib/utils
.PATH: ${.CURDIR}/lib/vbe
.PATH: ${.CURDIR}/lib/serialPort

SRCS = keyboard.c mouse.c rtc.c timer.c bitwise.c utils.c vbe.c board_generator.c debug.c draw.c assets.c glyphs.c event_handlers.c multiplayer.c app_time.c game.c game_entity.c game_player.c game_enemy.c game_bombs.c game_bombs_explosion.c game_map.c gui_game.c gui_menus.c gui_utils.c gui_widget_elements.c gui_widget_text_input.c gui_widget_utils.c main.c serial_port.c

CFLAGS += -Wall -Wextra -pedantic -D__LCOM_OPTIMIZED__
CFLAGS += -I${.CURDIR}/include
CFLAGS += -I${.CURDIR}/lib/serialPort
CFLAGS += -I${.CURDIR}/src
CFLAGS += -I${.CURDIR}/lib/keyboard
CFLAGS += -I${.CURDIR}/lib/mouse
CFLAGS += -I${.CURDIR}/lib/rtc
CFLAGS += -I${.CURDIR}/lib/timer
CFLAGS += -I${.CURDIR}/lib/utils
CFLAGS += -I${.CURDIR}/lib/vbe

DPADD += ${LIBLCF}
LDADD += -llcf

re: clean all

.END:
	cp ${.OBJDIR}/${PROG} ${.CURDIR}/${PROG}; 

.include <minix.lcom.mk>
