ARCH = armel
CROSSTOOLS = arm-none-linux-gnueabi-
CROSSDEVDIR = /opt/arm-dev/sysroots/armv7a-none-linux-gnueabi
BUILD_DEBUG = no

MTDEVD = ../mtdev-1.1.3
#EVEMUD = ../utouch-evemu-1.0.9
#FRAMED = ../utouch-frame-1.1.4
#GRAILD = ../utouch-grail-1.0.20

MTDEV_SRCS = core.c match.c caps.c match_four.c iobuf.c
#EVEMU_SRCS = evemu.c
#FRAME_SRCS = frame.c frame-mtdev.c
#GRAIL_SRCS = gestures-drag.c gestures-pinch.c gestures-rotate.c \
			 gestures-tapping.c grail-api.c grail-bits.c grail-event.c \
			 grail-gestures.c grail-inserter.c grail-recognizer.c



OPTCADD = -DJPANEL_TOUCHSCREEN -DMELFAS_TOUCHSCREEN -DMELFAS_XRES=2048.0 -DMELFAS_YRES=2048.0
OPTLADD =

TARGET = jgestured
#SRCS = ${MTDEV_SRCS} ${EVEMU_SRCS} ${FRAME_SRCS} ${GRAIL_SRCS}
SRCS = ${MTDEV_SRCS}
#SRCS+= gesture.c uinput_api.c
SRCS+= main.c uinput_api.c frame.c
SRCS+= gesture_param.c gesture_touch.c gesture_flick.c gesture_pinch.c
OBJS = ${SRCS:%.c=%.o}

#VPATH = ../src:${MTDEVD}/src:${EVEMUD}/src:${FRAMED}/src:${GRAILD}/src
VPATH = ../src:${MTDEVD}/src

CC = ${CROSSTOOLS}gcc
LD = ${CROSSTOOLS}gcc
AR = ${CROSSTOOLS}ar
STRIP = ${CROSSTOOLS}strip
STRIP_OPT = --remove-section=.comment --remove-section=.note

CFLAGS  = -g -O2 -Wall
CFLAGS += -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mhard-float
CFLAGS += -mfloat-abi=softfp -ftree-vectorize -mvectorize-with-neon-quad
CFLAGS += -mthumb-interwork -mno-thumb
#CFLAGS += -I${MTDEVD}/include -I${EVEMUD}/include
CFLAGS += -I../include -I${MTDEVD}/include
#CFLAGS += -I${FRAMED}/include -I${GRAILD}/include -I${GRAILD}/src
CFLAGS += -I${CROSSDEVDIR}/usr/include
CFLAGS += ${OPTCADD}

LDFLAGS += -L${CROSSDEVDIR}/usr/lib
LDFLAGS += ${OPTLADD}
LDFLAGS += -lm




all: depend.inc $(TARGET)

$(TARGET): $(OBJS) $(DEPLIBS)
	@echo "=== linking " ${CC} " : " $@
	$(CC) -o $@ $(OBJS) $(LDFLAGS)
ifneq ($(BUILD_DEBUG),yes)
	@echo "=== striping " ${CC} " : " $@
	$(STRIP) $(STRIP_OPT) $@
endif

%.o: %.c
	@echo "=== compiling " ${CC} " : " $@
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	@echo "=== cleaning ==="
	-rm -f $(TARGET) depend.inc $(OBJS)

# depend header file
depend.inc: $(SRCS)
	@echo "=== header file dependency resolv ==="
	$(CC) -MM $(CFLAGS) $^ > depend.inc

-include depend.inc
