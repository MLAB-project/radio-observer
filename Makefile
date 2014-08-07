#
# C++ Makefile template
#


BIN_NAME     = radio-observer
VERSION      = 0.4dev
# yes / no
IS_LIBRARY   = no

SRC_DIR      = src
CPP_FILES    = $(shell ls $(SRC_DIR)/*.cpp)
H_FILES      = $(shell ls $(SRC_DIR)/*.h)
OBJECT_FILES = $(foreach CPP_FILE, $(CPP_FILES), $(patsubst %.cpp,%.o,$(CPP_FILE)))
DEP_FILES    = $(foreach CPP_FILE, $(CPP_FILES), $(patsubst %.cpp,%.d,$(CPP_FILE)))

TESTS_DIR    = tests
TEST_BIN     = $(TESTS_DIR)/tests

DOCS_ARCH    = $(BIN_NAME)-$(VERSION)-docs.html.tar.gz

UNAME       := $(shell uname)
CXX          = clang++
CXXFLAGS     = -ggdb -O0 -Wall -Icppapp -rdynamic
LDFLAGS      = -Lcppapp -lcppapp -lfftw3 -lcfitsio -lpthread
ifeq ($(UNAME),Darwin)
	LDFLAGS += -framework jackmp
else
	LDFLAGS += $(shell pkg-config --libs jack)
endif


ECHO         = $(shell which echo)


build: $(BIN_NAME)


-include $(DEP_FILES)


clean:
	@echo "========= CLEANING =================================================="
	rm -f $(OBJECT_FILES) $(BIN_NAME)
	$(MAKE) -C $(TESTS_DIR) clean
	@echo


rebuild:
	@$(MAKE) clean
	@$(MAKE) build


deps: $(DEP_FILES)


test: build
	$(MAKE) -C $(TESTS_DIR)
	$(TEST_BIN)


clean-deps:
	rm -f $(DEP_FILES)


docs:
	@echo "========= GENERATING DOCS ==========================================="
	doxygen
	cd docs/html; tar -czf ../../$(DOCS_ARCH) ./*


clean-docs:
	@echo "========= CLEANING DOCS ============================================="
	rm -fR docs/html


upload-docs:
	@$(MAKE) docs
	scp $(DOCS_ARCH) jan@milik.cz:public_html/waterfall
	ssh -t jan@milik.cz "cd ~/public_html/waterfall/docs; tar -xzf ../$(DOCS_ARCH)"


$(BIN_NAME): $(OBJECT_FILES)
ifeq ($(IS_LIBRARY),yes)
	@echo "========= LINKING LIBRARY $@ ========================================"
	$(AR) -r $@ $^
else
	@echo "========= LINKING EXECUTABLE $@ ====================================="
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
endif
	@echo


.PHONY: all build clean rebuild deps test clean-deps docs clean-docs


%.d: %.cpp $(H_FILES)
	@$(ECHO) "Generating \"$@\"..."
	@$(ECHO) -n "$(SRC_DIR)/" > $@
	@$(CXX) $(CXXFLAGS) -MM $< >> $@


