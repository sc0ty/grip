GENERAL_SOURCES = \
				  general/compressedids.cpp \
				  general/dbreader.cpp \
				  general/dir-boost.cpp \
				  general/dir.cpp \
				  general/dir-posix.cpp \
				  general/error.cpp \
				  general/file.cpp \
				  general/fileline.cpp \
				  general/filelist.cpp \
				  general/ids.cpp \
				  general/node.cpp \
				  general/print.cpp \

GENERAL_HEADERS = \
				  general/case.h \
				  general/compressedids.h \
				  general/config.h \
				  general/dbreader.h \
				  general/dir.h \
				  general/error.h \
				  general/file.h \
				  general/fileline.h \
				  general/filelist.h \
				  general/ids.h \
				  general/index.h \
				  general/node.h \
				  general/print.h \
				  general/queue.h \
				  general/sem.h \
				  external/catch.hpp \
				  external/fnmatch.c \
				  external/fnmatch.h \
				  external/ansidecl.h \
				  external/getopt.c \
				  external/getopt.h \
				  external/getopt1.c \

CXXFLAGS += -Igeneral

