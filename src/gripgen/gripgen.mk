GEN_DIR    = gripgen
GEN_TARGET = $(GEN_DIR)/gripgen$(SUFFIX)
TARGETS   += $(GEN_TARGET)

GEN_SOURCES = \
			  $(GEN_DIR)/gripgen.cpp \
			  $(GEN_DIR)/indexer.cpp \
			  $(GEN_DIR)/sortdb.cpp \
			  $(GENERAL_SOURCES) \

GEN_HEADERS = \
			  $(GEN_DIR)/indexer.h \
			  $(GENERAL_HEADERS) \

GEN_CXXFLAGS = \
			   $(CXXFLAGS) \
			   -pthread \

GEN_LDFLAGS  = \
			   $(LDFLAGS) \
			   -pthread \


GEN_OBJDIR   = $(OBJDIR)/$(GEN_DIR)
GEN_OBJDIRS  = $(sort $(dir $(GEN_OBJECTS)))
GEN_OBJECTS  = $(addprefix $(GEN_OBJDIR)/, $(GEN_SOURCES:.cpp=.o))


$(GEN_TARGET): $(GEN_OBJECTS)
	$(CXX) -o $@ $^ $(GEN_LDFLAGS)

$(GEN_OBJECTS): $(GEN_OBJDIR)/%.o: %.cpp $(GEN_HEADERS) | $(GEN_OBJDIRS)
	$(CXX) -c -o $@ $< $(GEN_CXXFLAGS)

$(GEN_OBJDIRS):
	$(MKDIR) $@

gripgen-clean:
	$(RM) $(GEN_TARGET)
	$(RM) $(GEN_OBJECTS)

clean: gripgen-clean

.PHONY: gripgen-clean

