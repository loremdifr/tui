CC = clang

EXEC_NAME = build/main
SRC_NAME  = main.c

LIBS_DIR = libs/

CFLAGS = -fdiagnostics-color=always -W -Werror -Wextra -pedantic -g -std=c23
CFLAGS+= -I$(LIBS_DIR)

LD_LIBS = -lm


valgrind:
	rm -f $(EXEC_NAME)
	mkdir -p build
	#-O0 is for valgrind debugging, remove it later
	$(CC) $(CFLAGS) -O0 $(SRC_NAME) -o $(EXEC_NAME) $(LD_LIBS)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(EXEC_NAME) $(ARGS)

run:
	rm -f $(EXEC_NAME)
	mkdir -p build
	bear -- $(CC) $(CFLAGS) $(SRC_NAME) -o $(EXEC_NAME) $(LD_LIBS)
	./$(EXEC_NAME) $(ARGS)

check:
	$(CC) -fsyntax-only $(CFLAGS) $(SRC_NAME)

gf2:
	rm -f $(EXEC_NAME)
	mkdir -p build
	$(CC) $(CFLAGS) $(SRC_NAME) -o $(EXEC_NAME) $(LD_LIBS)
	gf2 --args ./$(EXEC_NAME) $(ARGS)

# phony is so that it skips checking the commands as if they
# were files to avoid recompilation. none of those commands are files
# and since we're using a unity build we don't really need separate translation
# units
.PHONY: run valgrind gf2 check