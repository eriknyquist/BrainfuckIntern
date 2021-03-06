OUTPUT_DIR := build
SRC_ROOT := source
SRC_DIRS := \
	$(SRC_ROOT) \
	$(SRC_ROOT)/evolution \
	$(SRC_ROOT)/population \
	$(SRC_ROOT)/bf_utils \
	$(SRC_ROOT)/portable_getopt \
	$(SRC_ROOT)/common

X64_CC := x86_64-w64-mingw32-gcc
X86_CC := i686-w64-mingw32-gcc

WIN_BUILD := windows_build
X64_DIR := $(WIN_BUILD)/x86_64
X86_DIR := $(WIN_BUILD)/i686

VPATH := $(SRC_DIRS)
SRC_FILES := $(foreach DIR,$(SRC_DIRS),$(wildcard $(DIR)/*.c))
OBJ_FILES := $(patsubst %.c,%.o,$(addprefix $(OUTPUT_DIR)/,$(notdir $(SRC_FILES))))
PROGNAME := bfintern
BUILD_OUTPUT := $(OUTPUT_DIR)/$(PROGNAME)

INCLUDES := $(addprefix -I, $(SRC_DIRS))

PROFILE_ENABLE_FLAGS := -pg -no-pie

FLAGS := -Wall -O3
DEBUG_FLAGS := -Wall -O0 -g3
PROFILE_FLAGS := $(DEBUG_FLAGS) $(PROFILE_ENABLE_FLAGS)

CFLAGS := $(FLAGS) $(INCLUDES)
.PHONY: clean

all: $(BUILD_OUTPUT)

debug: CFLAGS := $(DEBUG_FLAGS) $(INCLUDES)
debug: $(BUILD_OUTPUT)

profile: CFLAGS := $(PROFILE_FLAGS) $(INCLUDES)
profile: LFLAGS := $(PROFILE_ENABLE_FLAGS)
profile: $(BUILD_OUTPUT)

$(BUILD_OUTPUT): output_dir $(OBJ_FILES)
	$(CC) $(LFLAGS) $(OBJ_FILES) -o $@

$(OUTPUT_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

output_dir:
	[ -d $(OUTPUT_DIR) ] || mkdir -p $(OUTPUT_DIR)

windows_x64: CC := $(X64_CC)
windows_x64: $(BUILD_OUTPUT)
	cp $(BUILD_OUTPUT) $(X64_DIR)/$(PROGNAME).exe

windows_x86: CC := $(X86_CC)
windows_x86: $(BUILD_OUTPUT)
	cp $(BUILD_OUTPUT) $(X86_DIR)/$(PROGNAME).exe

clean:
	[ -d $(OUTPUT_DIR) ] && rm -rf $(OUTPUT_DIR)
