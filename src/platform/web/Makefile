PROJECT_ROOT := $(abspath ../../..)
SHORT_NAME := FF
FULL_NAME := Fighting Force
CONFIG ?= Release
APP_SOURCES := $(wildcard $(PROJECT_ROOT)/src/*.c) $(wildcard $(PROJECT_ROOT)/src/audit/*.c)
APP_DEFINES := -DWND_TITLE='"Fighting Force (xport)"' -DWND_WIDTH=960 -DWND_HEIGHT=768 -DFIELD_RATE=60

include ../../../../xport/src/platform/web/Makefile

# Preserve the deep translated call stack across browser yields
COMMON_FLAGS += -sASYNCIFY_STACK_SIZE=65536

ifeq ($(CONFIG),Release)
# Avoid the Binaryen optimized Asyncify failure on the complete FF module
BUILD_FLAGS := -O1 -DNDEBUG
endif
