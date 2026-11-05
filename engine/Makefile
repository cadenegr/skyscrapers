# The engine core is built once as a library (libengine.a);
# each puzzle plug-in is its own program linked against it.

CC       = cc
CFLAGS   = -Wall -Wextra -Werror -std=c99 -O2 -Icore/include
OBJ_DIR  = obj

CORE     = libengine.a
CORE_SRC = $(wildcard core/src/*.c)
CORE_OBJ = $(CORE_SRC:%.c=$(OBJ_DIR)/%.o)

SKY      = skyscrapers
SKY_SRC  = $(wildcard puzzles/skyscrapers/*.c)
SKY_OBJ  = $(SKY_SRC:%.c=$(OBJ_DIR)/%.o)

all: $(SKY)

test: $(SKY)
	./tests/run_tests.sh ./$(SKY)

$(CORE): $(CORE_OBJ)
	ar rcs $@ $^

$(SKY): $(SKY_OBJ) $(CORE)
	$(CC) $(CFLAGS) -o $@ $(SKY_OBJ) $(CORE)

$(OBJ_DIR)/core/%.o: core/%.c core/include/engine.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/puzzles/skyscrapers/%.o: puzzles/skyscrapers/%.c puzzles/skyscrapers/skyscrapers.h core/include/engine.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(CORE) $(SKY)

re: fclean all

.PHONY: all test clean fclean re
