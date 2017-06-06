PREFIX   ?= /usr/local

ifeq ($(OS),Windows_NT)
	SUFFIX  ?= .exe
	RM      = cmd /C del /Q
endif

CXXFLAGS ?= \
			--std=c++11 \
			-Wall \
			-Wextra \
			-pedantic \
			-pthread \

LDFLAGS ?= \
		   --std=c++11 \
		   -pthread \

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

ifeq ($(USE_BOOST),mt)
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


TARGETS = \
		  grip \
		  egrip \
		  fgrip \
		  gripgen \

SOURCES = \
		  indexer.cpp \
		  dbreader.cpp \
		  sortdb.cpp \
		  filelist.cpp \
		  ids.cpp \
		  compressedids.cpp \
		  grep.cpp \
		  file.cpp \
		  fileline.cpp \
		  print.cpp \
		  dir.cpp \
		  dir-posix.cpp \
		  dir-boost.cpp \
		  glob.cpp \
		  pattern.cpp \
		  node.cpp \
		  error.cpp \

HEADERS = \
		  indexer.h \
		  dbreader.h \
		  filelist.h \
		  ids.h \
		  compressedids.h \
		  grep.h \
		  index.h \
		  file.h \
		  fileline.h \
		  print.h \
		  dir.h \
		  glob.h \
		  pattern.h \
		  globfilters.def \
		  node.h \
		  case.h \
		  queue.h \
		  sem.h \
		  config.h \
		  error.h \

EXTERNAL = \
		  external/fnmatch.c \
		  external/fnmatch.h \
		  external/ansidecl.h \
		  external/getopt.c \
		  external/getopt.h \
		  external/getopt1.c \

OBJECTS = $(SOURCES:.cpp=.o)

TOBJECTS = $(TARGETS:=.o)

TFILES = $(TARGETS:=$(SUFFIX))

all: release

debug: CXXFLAGS += -O0 -g
debug: $(TFILES)

release: CXXFLAGS += -O3 -DNDEBUG
release: $(TFILES)

clean:
	$(RM) $(TFILES)
	$(RM) $(TOBJECTS)
	$(RM) $(OBJECTS)


$(TFILES): %: $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

grip$(SUFFIX): grip.o

egrip$(SUFFIX): egrip.o
egrip.o: grip.cpp

fgrip$(SUFFIX): fgrip.o
fgrip.o: grip.cpp

gripgen$(SUFFIX): gripgen.o

%.o: %.cpp $(HEADERS) $(EXTERNAL)
	$(CXX) -c -o $@ $< $(CXXFLAGS)


install: $(TFILES)
	install -m 0755 $^ $(PREFIX)/bin

uninstall:
	$(RM) $(addprefix $(PREFIX)/bin/, $(TFILES))


.PHONY: all clean debug release install uninstall

