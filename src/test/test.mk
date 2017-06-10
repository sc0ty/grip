TEST_DIR    = test
TEST_TARGET = $(TEST_DIR)/test$(SUFFIX)

TEST_SOURCES = \
			   $(TEST_DIR)/test.cpp \
			   $(TEST_DIR)/ids.cpp \
			   $(TEST_DIR)/compressedids.cpp \
			   $(GENERAL_SOURCES) \

TEST_HEADERS = \
			   $(TEST_DIR)/ids.h \
			   $(GENERAL_HEADERS) \


TEST_OBJDIR   = $(OBJDIR)/$(TEST_DIR)
TEST_OBJDIRS  = $(sort $(dir $(TEST_OBJECTS)))
TEST_OBJECTS  = $(addprefix $(TEST_OBJDIR)/, $(TEST_SOURCES:.cpp=.o))


test: $(TEST_TARGET) FORCE
	$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(TEST_OBJECTS): $(TEST_OBJDIR)/%.o: %.cpp $(TEST_HEADERS) | $(TEST_OBJDIRS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(TEST_OBJDIRS):
	$(MKDIR) $@

test-clean:
	$(RM) $(TEST_TARGET)
	$(RM) $(TEST_OBJECTS)

clean: test-clean

FORCE:

.PHONY: test test-clean FORCE

