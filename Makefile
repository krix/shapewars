# Makefile for ShapeWars game

# Options:
# RENDER   = GL|SOFTWARE|METAL (default GL; Metal only for macOS)
# DEBUG    = true|false (default false)
# USE_GL   = true|false (default false; only relevant on linux)

CC ?= gcc
EMCC ?= emcc
UNAME_S := $(shell uname -s)
RENDER ?= GL
USE_GLX ?= false
DEBUG ?= false
USER_CFLAGS ?=

L_FLAGS ?= -lm
C_FLAGS ?= -std=gnu99 -Wall -Wstrict-prototypes -Wno-unused-variable $(USER_CFLAGS)

# Source files
GAME_SRC := $(shell find src/ -type f -name '*.c')
HIGH_IMPACT_SRC := $(shell find high_impact/src/ -type f -name '*.c')
ALL_SRC := $(GAME_SRC) $(HIGH_IMPACT_SRC)

BUILD_DIR ?= build
OBJ_DIR = build/obj

TARGET_SDL ?= $(BUILD_DIR)/shapewars_sdl
TARGET_SOKOL ?= $(BUILD_DIR)/shapewars_sokol

# Use sdl as the default target when just calling `make`
.DEFAULT_GOAL := sokol

ifeq ($(DEBUG), true)
	C_FLAGS := $(C_FLAGS) -g
else
	C_FLAGS := $(C_FLAGS) -O3
endif

# Renderer selection
ifeq ($(RENDER), GL)
	C_FLAGS := $(C_FLAGS) -DRENDER_GL
else ifeq ($(RENDER), METAL)
	C_FLAGS := $(C_FLAGS) -DRENDER_METAL
else ifeq ($(RENDER), SOFTWARE)
	C_FLAGS := $(C_FLAGS) -DRENDER_SOFTWARE
else
	$(error Unknown RENDER backend)
endif

ifeq ($(GL_VERSION), GLES2)
	C_FLAGS := $(C_FLAGS) -DUSE_GLES2
endif

# Platform-specific settings
ifeq ($(UNAME_S), Darwin)
	BREW_HOME := $(shell brew --prefix)
	C_FLAGS := $(C_FLAGS) -x objective-c -I/opt/homebrew/include -D_THREAD_SAFE -w
	L_FLAGS := $(L_FLAGS) -L$(BREW_HOME)/lib -framework Foundation

	L_FLAGS_SOKOL = -framework Cocoa -framework QuartzCore -framework AudioToolbox

	ifeq ($(RENDER), GL)
		L_FLAGS := $(L_FLAGS) -GLU -framework OpenGL
	else ifeq ($(RENDER), METAL)
		C_FLAGS := $(C_FLAGS) -fobjc-arc
		L_FLAGS := $(L_FLAGS) -framework Metal
		L_FLAGS_SOKOL := $(L_FLAGS_SOKOL) -framework MetalKit
	endif

else ifeq ($(UNAME_S), Linux)
	ifeq ($(RENDER), GL)
		ifeq ($(USE_GLX), true)
			L_FLAGS := $(L_FLAGS) -lGL
		else
			L_FLAGS := $(L_FLAGS) -lOpenGL
		endif
	endif

	L_FLAGS_SOKOL = -lX11 -lXcursor -pthread -lXi -ldl -lasound

else ifeq ($(shell uname -o), Msys)
	ifeq ($(RENDER), GL)
		L_FLAGS := $(L_FLAGS) -lopengl32
	endif

	L_FLAGS_SOKOL = --pthread -lkernel32 -luser32 -lshell32 -lgdi32 -lole32

else ifeq ($(OS), Windows_NT)
	# Windows-specific flags
	C_FLAGS := $(C_FLAGS) -DWIN32
	L_FLAGS := $(L_FLAGS) -lmingw32 -lSDL2main -lSDL2 -mwindows -lopengl32 -lglu32
	L_FLAGS_SOKOL = -lgdi32 -luser32 -lshell32 -lole32

else
	$(error Unknown environment)
endif

# Object files
ALL_OBJ := $(patsubst %.c, $(OBJ_DIR)/%.o, $(ALL_SRC))

# Build targets
all: sokol

# Platform: Native/SOKOL
sokol: $(ALL_OBJ)
	mkdir -p $(BUILD_DIR)
	$(CC) $^ -o $(TARGET_SOKOL) $(L_FLAGS) $(L_FLAGS_SOKOL)

$(OBJ_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) -DPLATFORM_SOKOL -MMD -MP -c $< -o $@

# Clean target
clean:
	$(RM) -rf $(OBJ_DIR) $(BUILD_DIR)

.PHONY: all clean sokol