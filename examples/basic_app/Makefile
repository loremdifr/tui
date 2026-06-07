ifeq ($(OS),Windows_NT)
    CC = mingw64\bin\gcc 
    EXT := .exe
    RM := cmd /C del /Q /F
    DS := \\
else
    CC = clang
    EXT :=
    RM := rm -f
    DS := /
endif

SRC_NAME  := $(addprefix $(CURDIR)/, main.c)
BUILD_DIR := $(addprefix $(CURDIR)/, build)
EXEC_NAME  = $(BUILD_DIR)$(DS)main$(EXT)
INCLUDES_DIR := $(addprefix $(CURDIR)/, ../..)

CFLAGS = -fdiagnostics-color=always -W -Werror -Wextra -pedantic -g -std=gnu23
CFLAGS+= -I$(INCLUDES_DIR)

LD_LIBS = -lm

clean:
	-$(RM) "$(EXEC_NAME)"

valgrind: clean
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -O0 $(SRC_NAME) -o $(EXEC_NAME) $(LD_LIBS)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(EXEC_NAME) $(ARGS)

run: clean
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_NAME) -o $(EXEC_NAME) $(LD_LIBS)
ifeq ($(OS),Windows_NT)
	@cmd /C "$(EXEC_NAME) $(ARGS)"
else
	@$(EXEC_NAME) $(ARGS)
endif

#used for syntax checking
check:
	$(CC) -fsyntax-only $(CFLAGS) $(SRC_NAME)

gf2: clean
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_NAME) -o $(EXEC_NAME) $(LD_LIBS)
	gf2 --args ./$(EXEC_NAME) $(ARGS)

# phony is so that it skips checking the commands as if they
# were files to avoid recompilation. none of those commands are files
# and since we're using a unity build we don't really need separate translation
# units
.PHONY: run valgrind gf2 check clean all