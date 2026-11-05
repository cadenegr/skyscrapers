NAME    = engine

SRC_DIR = src
OBJ_DIR = obj
SRC     = $(wildcard $(SRC_DIR)/*.c)
OBJ     = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CC      = cc
CFLAGS  = -Wall -Wextra -Werror -std=c99 -O2 -Iinclude

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c include/engine.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
