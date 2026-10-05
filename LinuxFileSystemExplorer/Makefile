# ============================================================
# Makefile - Linux File System Explorer Using System Calls
# Course : Operating System and System Programming (25CS2104E)
#
# Usage:
#   make            build the program (binary: build/explorer)
#   make run        build (if needed) and start the explorer
#   make clean      remove all build outputs
#   make help       show these targets
# ============================================================

# --- Toolchain and flags --------------------------------------------
CC       = gcc
STD      = -std=c11
WARN     = -Wall -Wextra -Wpedantic
CFLAGS   = $(STD) $(WARN) -O2 -Iinclude -Isrc
LDFLAGS  =

# --- Files ----------------------------------------------------------
SRCDIR   = src
INCDIR   = include
BUILDDIR = build
TARGET   = $(BUILDDIR)/explorer

SOURCES  = $(SRCDIR)/main.c \
           $(SRCDIR)/file_operations.c \
           $(SRCDIR)/directory_operations.c \
           $(SRCDIR)/metadata.c \
           $(SRCDIR)/navigation.c \
           $(SRCDIR)/utils.c

OBJECTS  = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))

# --- Rules ----------------------------------------------------------
.PHONY: all clean run help

all: $(TARGET)

# Link: all objects -> executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Compile: one object per source file (header deps tracked too)
$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(wildcard $(INCDIR)/*.h) | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create the build directory on demand
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILDDIR)

help:
	@echo "Targets:"
	@echo "  make       - compile the explorer into $(TARGET)"
	@echo "  make run   - compile (if needed) and start the explorer"
	@echo "  make clean - remove the $(BUILDDIR) directory"
