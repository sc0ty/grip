PREFIX   ?= /usr/local
OBJDIR   ?= obj

ifeq ($(OS),Windows_NT)
	SUFFIX  ?= .exe
	RM      = scripts\rm.cmd
	MKDIR   = scripts\mkdir.cmd
	INSTALL = scripts\install.cmd
else
	MKDIR   = mkdir -p
	INSTALL = install -m 0755
	CXXFLAGS_EXTRA = -D_POSIX_C_SOURCE
endif

CXXFLAGS ?= \
			--std=c++11 \
			-Wall \
			-Wextra \
			-pedantic \
			-I. \
			$(CXXFLAGS_EXTRA) \

LDFLAGS ?= \
		   --std=c++11 \

ifdef STATIC
	LDFLAGS += \
			   -static \
			   -static-libgcc \
			   -static-libstdc++ \

endif

ifdef USE_BOOST
	CXXFLAGS += -DUSE_BOOST
endif

ifeq ($(USE_BOOST),yes)
	LDFLAGS  += \
				-lboost_regex \
				-lboost_system \
				-lboost_filesystem \

endif

ifeq ($(USE_BOOST),manual)
	CXXFLAGS += $(BOOST_CXXFLAGS)
	LDFLAGS  += $(BOOST_LDFLAGS)
endif

VERSION  ?= $(shell git describe --tags)
ifneq ($(VERSION), )
	CXXFLAGS += -DVERSION=\"$(VERSION)\"
endif


all: release

include general/general.mk
include grip/grip.mk
include gripgen/gripgen.mk
include test/test.mk


release: $(TARGETS)
release: CXXFLAGS += -O3 -DNDEBUG

debug:   $(TARGETS)
debug:   CXXFLAGS += -O0 -g

install: $(TARGETS)
	$(MKDIR) $(PREFIX)/bin
	$(INSTALL) $^ $(PREFIX)/bin

uninstall:
	$(RM) $(addprefix $(PREFIX)/bin/, $(notdir $(TARGETS)))

.PHONY: all release debug clean install uninstall

