# Balls makefile

BINDIR=bin/

TARGET = balls
TYPE = ps-exe

SRCS = src/main.cpp \
	../third_party/nugget/common/crt0/crt0.s \

CPPFLAGS += -I../third_party/nugget/psyq/include
LDFLAGS += -L../third_party/nugget/psyq/lib

LDFLAGS += -Wl,--start-group
LDFLAGS += -lapi
LDFLAGS += -lc
LDFLAGS += -letc
LDFLAGS += -lgpu
LDFLAGS += -lgte
LDFLAGS += -lpad
LDFLAGS += -Wl,--end-group

include ../third_party/nugget/common.mk
