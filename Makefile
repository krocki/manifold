
CFLAGS = -Ofast -Wall -Wextra -Wfatal-errors -std=c++14
NANOGUI_GIT_PATH=/Users/kamilrocki/git/nanogui_forked

all:
	g++ -o manifold ./src/main.cc $(CFLAGS) -I$(NANOGUI_GIT_PATH)/ext/eigen/ -I$(NANOGUI_GIT_PATH)/ext/nanovg/src/ -L$(NANOGUI_GIT_PATH)/build/ -lnanogui -framework OpenGL


