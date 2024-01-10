CC = cc
CFLAGS = -Wall -Wextra
BUILD_DIR = build
PROGRAM = shell-picker
SOURCES = shell-picker.c
X86_64_OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%_x86_64.o,$(SOURCES))
ARM64_OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%_arm64.o,$(SOURCES))
EXECUTABLES = $(BUILD_DIR)/$(PROGRAM)_x86_64 $(BUILD_DIR)/$(PROGRAM)_arm64

.PHONY: all clean

all: $(BUILD_DIR)/$(PROGRAM)

$(BUILD_DIR)/$(PROGRAM): $(EXECUTABLES)
	lipo -create -output $@ $^

$(BUILD_DIR)/$(PROGRAM)_x86_64: $(X86_64_OBJECTS)
	$(CC) $(CFLAGS) -arch x86_64 -o $@ $^

$(BUILD_DIR)/$(PROGRAM)_arm64: $(ARM64_OBJECTS)
	$(CC) $(CFLAGS) -arch arm64 -o $@ $^

$(BUILD_DIR)/%_x86_64.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -arch x86_64 -c -o $@ $<

$(BUILD_DIR)/%_arm64.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -arch arm64 -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(PROGRAM)
