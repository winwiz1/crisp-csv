BUILD_DIR := build
SOURCE_DIR := src
BUILD_CONFIG_DIR := ${BUILD_DIR}/config
BUILD_HANDLERS_DIR := ${BUILD_DIR}/handlers
BUILD_TEST_DIR := ${BUILD_DIR}/test
MKDIR_P := mkdir -p
TEST_MONIKER_SRC := $(SOURCE_DIR)/test/moniker.test.txt
TEST_MONIKER_OBJ := $(BUILD_TEST_DIR)/moniker.test.o
TEST_MONIKER_EMBED := objcopy
CSV_VERSION := 1.1.3

ifdef CSV_TEST
  SRCS := $(shell find ./${SOURCE_DIR} \( -type f -a -name *.cpp -o -type f -a -name *.c \) -print )
else
  SRCS := $(shell find ./${SOURCE_DIR} -path ./${SOURCE_DIR}/test -prune -o \( -type f -a -name *.cpp -o -type f -a -name *.c \) -print )
endif

# % expands to filename with extension stripped
__OBJS := $(SRCS:%.cpp=%.o)
_OBJS := $(__OBJS:%.c=%.o)
OBJS := $(shell echo ${_OBJS} | sed 's/${SOURCE_DIR}/${BUILD_DIR}/g' )
ifdef CSV_TEST
  OBJS += $(TEST_MONIKER_OBJ)
endif

_LINK_TARGET := $(shell basename "${PWD}")
LINK_TARGET := $(patsubst %,./${BUILD_DIR}/%,$(_LINK_TARGET))

REBUILDABLES := $(OBJS) $(LINK_TARGET)

LIBS := -lpthread
CC := g++
CFLAGS := -DNDEBUG -DAPP_TITLE="\"$(strip $(_LINK_TARGET))\"" -DCSV_VERSION=$(CSV_VERSION) -DSKIP_LOCALITIES=0 -Wall -Wextra -O2 -std=c++17 -pthread
ifdef CSV_TEST
  CFLAGS += -DCSV_TEST
endif
#####################################

.PHONY: default depend all clean directories

default: depend $(LINK_TARGET)
	$(info All done)

all: default

clean : 
	@rm -f $(REBUILDABLES) $(TEST_MONIKER_OBJ)
	@rm -f ./.depend
	@test -d ${BUILD_CONFIG_DIR} && rmdir --ignore-fail-on-non-empty ${BUILD_CONFIG_DIR} || :
	@test -d ${BUILD_HANDLERS_DIR} && rmdir --ignore-fail-on-non-empty ${BUILD_HANDLERS_DIR} || :
	@test -d ${BUILD_TEST_DIR} && rmdir --ignore-fail-on-non-empty ${BUILD_TEST_DIR} || :
	@test -d ${BUILD_DIR} && rmdir --ignore-fail-on-non-empty ${BUILD_DIR} || :
	$(info Clean done)

directories: ${BUILD_DIR} ${BUILD_CONFIG_DIR} ${BUILD_HANDLERS_DIR} ${BUILD_TEST_DIR}

${BUILD_DIR}:
	${MKDIR_P} ${BUILD_DIR}

${BUILD_CONFIG_DIR}:
	${MKDIR_P} ${BUILD_CONFIG_DIR}

${BUILD_HANDLERS_DIR}:
	${MKDIR_P} ${BUILD_HANDLERS_DIR}

${BUILD_TEST_DIR}:
	${MKDIR_P} ${BUILD_TEST_DIR}

#####################################

# Print a variable: make print-OBJS
# stackoverflow.com/a/25817631
print-%  : ; @echo $* = $($*)

#####################################

# Based on stackoverflow.com/a/2394651 with sed added
depend: directories ./.depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend;
	sed -i '/\bo\b/s/^/build\//' ./.depend

include ./.depend

#####################################

# $@ expands to the rule's target, in this case the name
# of the executable (named after the project top directory).
# $^ expands to the rule's dependencies e.g. object files.
$(LINK_TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

# $@ expands to the pattern-matched target
# $< expands to the pattern-matched dependency
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

$(TEST_MONIKER_OBJ): $(TEST_MONIKER_SRC)
	$(TEST_MONIKER_EMBED) --input binary --output-target pe-x86-64 --binary-architecture i386:x86-64 $^ $@
