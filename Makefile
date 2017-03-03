
OS := $(shell uname)

CC=g++

CFLAGS = -Ofast -Wall -Wextra -Wfatal-errors -std=c++14
LFLAGS = -L$(NANOGUI_GIT_PATH)/build/ -lnanogui -framework OpenGL
INCLUDES = -I./src/ -I$(NANOGUI_GIT_PATH)/ext/eigen/ -I$(NANOGUI_GIT_PATH)/ext/nanovg/src/
NANOGUI_GIT_PATH=/Users/kamilrocki/git/nanogui_forked

# BLAS
CFLAGS := -DUSE_BLAS $(CFLAGS)

ifeq ($(OS),Linux)
	INCLUDES := -I/opt/OpenBLAS/include $(INCLUDES)
	LFLAGS := -lopenblas -L/opt/OpenBLAS/lib $(LFLAGS)
else
	#OSX
	INCLUDES := -I/usr/local/opt/openblas/include $(INCLUDES)
	LFLAGS := -lopenblas -L/usr/local/opt/openblas/lib $(LFLAGS)
endif

all:
	$(CC) -o nntest ./src/nntest.cc $(INCLUDES) $(CFLAGS) $(LFLAGS)
	$(CC) -o manifold ./src/main.cc $(INCLUDES) $(CFLAGS) $(LFLAGS)
