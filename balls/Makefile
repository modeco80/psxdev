# Balls makefile
# This is also a sort of template for the common.mk buildsystem.

PROJ = balls
TARGET_TYPE = exe
OBJS = obj/main.o

# Directories holding projects that need to be built before this target is built go here.
DEPENDENT_PROJECTS = testlib/

# Libraries we need to link to. SN/Runtime Library ones should probably be listed first.
LIBS = libsn.lib \
	testlib/lib/libtest.lib \
	libpad.lib \
	libapi.lib \
	libetc.lib \
	libgpu.lib \
	libc.lib
	
# Applicable source directories for this target go here.
VPATH = src/


# Some generic Make boilerplate - this is really just here
# to let you customize build flow or add your own pre/post-build functionality
# for say, if you're building a game and want to pack assets/build a CD image/etc

all: all-dependent-projects bin/$(PROJ).exe
clean: clean-common

run: bin/$(PROJ).exe
	.\run.cmd $<

include $(PSYQ_ROOT)/make/common.mk