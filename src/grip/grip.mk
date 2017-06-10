GRIP_DIR     = grip

GRIP_TARGETS = \
			   $(GRIP_DIR)/grip$(SUFFIX) \
			   $(GRIP_DIR)/egrip$(SUFFIX) \
			   $(GRIP_DIR)/fgrip$(SUFFIX) \

TARGETS     += $(GRIP_TARGETS)

GRIP_SOURCES = \
			   $(GRIP_DIR)/glob.cpp \
			   $(GRIP_DIR)/grep.cpp \
			   $(GRIP_DIR)/pattern.cpp \
			   $(GENERAL_SOURCES) \

GRIP_HEADERS = \
			   $(GRIP_DIR)/glob.h \
			   $(GRIP_DIR)/grep.h \
			   $(GRIP_DIR)/pattern.h \
			   $(GENERAL_HEADERS) \

GRIP_CXXFLAGS = \
				$(CXXFLAGS) \

GRIP_OBJDIR   = $(OBJDIR)/$(GRIP_DIR)
GRIP_OBJDIRS  = $(sort $(dir $(GRIP_OBJECTS) $(GRIP_TOBJECTS)))
GRIP_OBJECTS  = $(addprefix $(GRIP_OBJDIR)/, $(GRIP_SOURCES:.cpp=.o))
GRIP_TOBJECTS = $(addprefix $(GRIP_OBJDIR)/, $(GRIP_TARGETS:$(SUFFIX)=.o))


$(GRIP_TARGETS): %$(SUFFIX): $(GRIP_OBJDIR)/%.o $(GRIP_OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(GRIP_OBJECTS) $(GRIP_TOBJECTS): $(GRIP_OBJDIR)/%.o: %.cpp $(GRIP_HEADERS) | $(GRIP_OBJDIRS)
	$(CXX) -c -o $@ $< $(GRIP_CXXFLAGS)

$(GRIP_OBJDIRS):
	$(MKDIR) $@

grip-clean:
	$(RM) $(GRIP_TARGETS)
	$(RM) $(GRIP_TOBJECTS)
	$(RM) $(GRIP_OBJECTS)

clean: grip-clean

.PHONY: grip-clean

