OS := $(shell uname)

CC=g++

CFLAGS = -Ofast -Wall -Wextra -Wfatal-errors -std=c++14
LFLAGS = -L$(NANOGUI_GIT_PATH)/build/ -lnanogui
INCLUDES = -I./src/ -I$(NANOGUI_GIT_PATH)/include/ -I$(NANOGUI_GIT_PATH)/ext/eigen/ -I$(NANOGUI_GIT_PATH)/ext/glfw/include/ -I$(NANOGUI_GIT_PATH)/ext/nanovg/src/
NANOGUI_GIT_PATH=$(HOME)/git/nanogui_forked

# BLAS
CFLAGS := -DUSE_BLAS $(CFLAGS)

ifeq ($(OS),Linux)
		CFLAGS := $(CFLAGS) -fopenmp
        INCLUDES := -I/opt/OpenBLAS/include $(INCLUDES)
        LFLAGS := -lopenblas -L/opt/OpenBLAS/lib $(LFLAGS) -lGL -lpthread
else
        #OSX
        INCLUDES := -I/usr/local/opt/openblas/include $(INCLUDES)
        LFLAGS := -lopenblas -L/usr/local/opt/openblas/lib $(LFLAGS) -framework OpenGL
endif

all:
	# $(CC) -o nntest ./src/nntest.cc $(INCLUDES) $(CFLAGS) $(LFLAGS)
	# $(CC) -o surftest ./src/surftest.cc $(INCLUDES) $(CFLAGS) $(LFLAGS)
	# $(CC) -o surfnano ./src/surfnano.cc $(INCLUDES) $(CFLAGS) $(LFLAGS)
	# $(CC) -o mlp_mnist ./src/mlp_mnist.cc $(INCLUDES) $(CFLAGS) $(LFLAGS)
	# $(CC) -o graphs ./src/graphs.cc $(INCLUDES) $(CFLAGS) $(LFLAGS)
	# $(CC) -o template ./src/template.cc $(INCLUDES) $(CFLAGS) $(LFLAGS)
	$(CC) -o nbody ./src/nbody.cc  $(INCLUDES) $(CFLAGS) $(LFLAGS)
	$(CC) -o fplot ./src/fplot.cc $(INCLUDES) $(CFLAGS) $(LFLAGS)
	$(CC) -o manifold ./src/manifold.cc $(INCLUDES) $(CFLAGS) $(LFLAGS)
