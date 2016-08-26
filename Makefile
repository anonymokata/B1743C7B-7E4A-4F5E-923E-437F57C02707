CFLAGS=-g -O2 -Wall -Wextra -std=c99 -Isrc -rdynamic -DNDEBUG $(OPTFLAGS)
LIBS=-ldl $(OPTLIBS) # Library linking options
PREFIX?=/usr/local
CHECK_LIBRARY_DIR=/usr/local/lib

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst src/%.c,build/%.o,$(SOURCES))

TEST_SRC=$(wildcard tests/check_*.c)
TEST_OBJ=$(patsubst %.c,%,$(TEST_SRC))

TARGET=build/libroman_calculator.a
SO_TARGET=$(patsubst %.a,%.so,$(TARGET))

# The Target Build
all: $(TARGET) $(SO_TARGET)

# The Development Build
dev: CFLAGS=-g -Isrc -Wall -Wextra $(OPTFLAGS)
dev: all check

# Recipe for Object Files
$(OBJECTS): build/%.o: src/%.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

# Recipe for libroman_calculator
$(TARGET): CFLAGS += -fPIC
$(TARGET): build $(OBJECTS)
	ar rcs $@ $(OBJECTS)
	ranlib $@

# Recipe for shared objects. Links each SO_TARGET to all objects.
$(SO_TARGET): $(TARGET) $(OBJECTS)
	$(CC) -shared -o $@ $(OBJECTS)

# Create build and bin subdirectories for object/library files and binaries.
build:
	@mkdir -p build
	@mkdir -p bin

# The Unit Tests
tests/check_roman_calculator: $(TARGET)
	cc tests/check_roman_calculator.c \
	-o tests/check_roman_calculator build/libroman_calculator.a \
	-L$(CHECK_LIBRARY_DIR) -Wl,-rpath=$(CHECK_LIBRARY_DIR) $(shell pkg-config --libs --cflags check)
.PHONY: check
check: tests/check_roman_calculator
	./tests/check_roman_calculator

# Valgrind
valgrind:
	VALGRIND="valgrind --log-file=/tmp/valgrind-%p.log" $(MAKE)

# The Cleaner
clean:
	@rm -rf build $(OBJECTS) $(TESTS)
	@rm -f tests/tests.log tests/check_roman_calculator
	@find . -name "*.gc*" -exec rm {} \;
	@rm -rf `find . -name "*.dSYM" -print`

# Library Installer
install: all
	install -d $(DESTDIR)/$(PREFIX)/lib/
	install $(TARGET) $(DESTDIR)/$(PREFIX)/lib/
