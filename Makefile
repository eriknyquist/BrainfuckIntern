OUTPUT_DIR := build
SRC_ROOT := source
SRC_DIRS := \
	$(SRC_ROOT) \
	$(SRC_ROOT)/evolution \
	$(SRC_ROOT)/population \
	$(SRC_ROOT)/bf_utils \
	$(SRC_ROOT)/portable_getopt \
	$(SRC_ROOT)/common

VPATH := $(SRC_DIRS)
SRC_FILES := $(foreach DIR,$(SRC_DIRS),$(wildcard $(DIR)/*.c))
OBJ_FILES := $(patsubst %.c,%.o,$(addprefix $(OUTPUT_DIR)/,$(notdir $(SRC_FILES))))
PROGNAME=$(OUTPUT_DIR)/bfintern

INCLUDES := $(addprefix -I, $(SRC_DIRS))
FLAGS := -Wall -O3
DEBUG_FLAGS := -Wall -O0 -g3
CFLAGS := $(FLAGS) $(INCLUDES)
.PHONY: clean

all: $(PROGNAME)
debug: CFLAGS := $(DEBUG_FLAGS) $(INCLUDES)
debug: $(PROGNAME)

$(PROGNAME): output_dir $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $@

$(OUTPUT_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

output_dir:
	[ -d $(OUTPUT_DIR) ] || mkdir -p $(OUTPUT_DIR)

clean:
	[ -d $(OUTPUT_DIR) ] && rm -rf $(OUTPUT_DIR)
