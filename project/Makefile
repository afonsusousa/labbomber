# Name of the executable program
PROG = proj

# Build directory (BSD make)
BUILD_DIR ?= build
BUILD_DIR_PATH != mkdir -p ${.CURDIR}/${BUILD_DIR} && echo ${.CURDIR}/${BUILD_DIR}
.OBJDIR: ${BUILD_DIR_PATH}

# Source search paths
.PATH: ${.CURDIR}/src/core
.PATH: ${.CURDIR}/src/multiplayer
.PATH: ${.CURDIR}/src/scoreboard
.PATH: ${.CURDIR}/src/time
.PATH: ${.CURDIR}/src/game
.PATH: ${.CURDIR}/src/gui
.PATH: ${.CURDIR}/src/view
.PATH: ${.CURDIR}/src/view/game
.PATH: ${.CURDIR}/lib/keyboard
.PATH: ${.CURDIR}/lib/mouse
.PATH: ${.CURDIR}/lib/rtc
.PATH: ${.CURDIR}/lib/timer
.PATH: ${.CURDIR}/lib/utils
.PATH: ${.CURDIR}/lib/vbe
.PATH: ${.CURDIR}/lib/serialPort

SRCS += keyboard.c mouse.c rtc.c timer.c bitwise.c utils.c vbe.c serial_port.c
SRCS += main.c event_handlers.c debug.c
SRCS += app_time.c
SRCS += mp_protocol.c mp_send.c mp_receive.c mp_session.c
SRCS += game.c entity_controller.c enemy_spawn.c player_controller.c enemy_controller.c
SRCS += game_bombs.c game_bombs_explosion.c game_map.c board_generator.c
SRCS += scoreboard_controller.c
SRCS += gui_dialogs.c gui_game.c gui_menus.c gui_scoreboard.c gui_utils.c
SRCS += gui_widget_elements.c gui_widget_text_input.c gui_widget_utils.c
SRCS += draw.c assets.c glyphs.c
SRCS += draw_player.c draw_enemy.c draw_map.c draw_bombs.c draw_hud.c draw_game.c 

CFLAGS += -Wall -Wextra -pedantic -D__LCOM_OPTIMIZED__
# Project headers
CFLAGS += -I${.CURDIR}/include
CFLAGS += -I${.CURDIR}/include/core
CFLAGS += -I${.CURDIR}/include/multiplayer
CFLAGS += -I${.CURDIR}/include/scoreboard
CFLAGS += -I${.CURDIR}/include/time
CFLAGS += -I${.CURDIR}/include/game
CFLAGS += -I${.CURDIR}/include/gui
CFLAGS += -I${.CURDIR}/include/view
# Lib headers
CFLAGS += -I${.CURDIR}/lib/keyboard
CFLAGS += -I${.CURDIR}/lib/mouse
CFLAGS += -I${.CURDIR}/lib/rtc
CFLAGS += -I${.CURDIR}/lib/timer
CFLAGS += -I${.CURDIR}/lib/utils
CFLAGS += -I${.CURDIR}/lib/vbe
CFLAGS += -I${.CURDIR}/lib/serialPort 

DPADD += ${LIBLCF}
LDADD += -llcf

re: clean all

.END:
	cp ${.OBJDIR}/${PROG} ${.CURDIR}/${PROG}; 

.include <minix.lcom.mk>
