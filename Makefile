NAME	=	snaker

#compiler
CC	=	gcc
LFLAGS	= -Wall -Wextra -o2 #-Werror -O2

#sources & object files
SRC_DIR =	./src/
OBJ_DIR =	./build/
SRC_LIST =	main.c \
		draw_pixel.c \
		draw_shapes.c \
		utils.c
SRCS = $(addprefix $(SRC_DIR),$(SRC_LIST))
OBJS = $(addprefix $(OBJ_DIR),$(SRC_LIST:.c=.o))

#SDL2
SDL_SRC_DIR = SDL2-2.0.14
SDL_MIX_SRC_DIR = SDL2_mixer-2.0.4
SDL_DIR = libSDL2
SDL_BUILD_DIR_PATH = $(CURDIR)/$(SDL_DIR)
SDL_CFLAGS = `$(SDL_DIR)/bin/sdl2-config --cflags --libs` -lSDL2_mixer
SDL_BIN = $(SDL_DIR)/lib/libSDL2.a

.PHONY: all clean fclean re

all: $(SDL_BIN) $(NAME)

$(OBJ_DIR)%.o:$(SRC_DIR)%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(LFLAGS) -Iinclude/ -I$(SDL_DIR)/include/SDL2 -o $@ -c $<

$(NAME): $(OBJS)
	$(CC) $(OBJS) $(SDL_CFLAGS) -lm -o $(NAME)

$(SDL_BIN) :
	echo "Extracting SDL archives..."
	tar xzf SDL2-2.0.14.tar.gz
	tar xzf SDL2_mixer-2.0.4.tar.gz
	mkdir -p $(SDL_DIR)
	cd $(SDL_SRC_DIR); ./configure --prefix=$(SDL_BUILD_DIR_PATH); make -j6; make install
	cd $(SDL_MIX_SRC_DIR); ./configure --prefix=$(SDL_BUILD_DIR_PATH); make -j6; make install

clean:
	rm -rf $(OBJ_DIR)
	rm -rf DATA

fclean: clean
	rm $(NAME)
	rm -rf $(SDL_SRC_DIR)
	rm -rf $(SDL_MIX_SRC_DIR)

re: fclean all
