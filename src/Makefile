PREFIX   ?= /usr/local

CXXFLAGS ?= \
			--std=c++11 \
			-Wall \
			-Wextra \
			-pedantic \

LDFLAGS ?= \

TARGETS = \
		  grip \
		  gripgen \

SOURCES = \
		  indexer.cpp \
		  finder.cpp \
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
		  error.cpp \

HEADERS = \
		  indexer.h \
		  finder.h \
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
		  config.h \
		  error.h \

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
	$(CXX) $(LDFLAGS) -o $@ $^

grip: grip.o

gripgen: gripgen.o

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<


install: $(TARGETS)
	install -m 0755 $^ $(PREFIX)/bin

uninstall:
	$(RM) $(addprefix $(PREFIX)/bin/, $(TARGETS))


.PHONY: all clean debug release install uninstall

