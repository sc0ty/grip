PREFIX   ?= /usr/local

CXXFLAGS ?= \
			--std=c++11 \
			-Wall \
			-Wextra \
			-pedantic \

LDFLAGS ?= \

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
		  config.h \
		  error.h \

EXTERNAL = \
		  external/fnmatch.c \
		  external/fnmatch.h \

OBJECTS = $(SOURCES:.cpp=.o)

TOBJECTS = $(TARGETS:=.o)


all: release

debug: CXXFLAGS += -O0 -g
debug: $(TARGETS)

release: CXXFLAGS += -O2 -DNDEBUG
release: $(TARGETS)

clean:
	$(RM) $(TARGETS)
	$(RM) $(TOBJECTS)
	$(RM) $(OBJECTS)


$(TARGETS): %: $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

grip: grip.o

egrip: egrip.o
egrip.o: grip.cpp

fgrip: fgrip.o
fgrip.o: grip.cpp

gripgen: gripgen.o

%.o: %.cpp $(HEADERS) $(EXTERNAL)
	$(CXX) -c -o $@ $< $(CXXFLAGS)


install: $(TARGETS)
	install -m 0755 $^ $(PREFIX)/bin

uninstall:
	$(RM) $(addprefix $(PREFIX)/bin/, $(TARGETS))


.PHONY: all clean debug release install uninstall

