# WARNING: this makefile is recursive!
#
# by default it runs commands on all example apps
# if we want to run it on a single app it's used like this:
# make <command> APP=basic_app
# where basic_app is the name of the folder inside the examples folder
#
# commands:
# make check <- runs "make check" on all the example apps in examples/
# make clean <- cleans all the example apps
# make build <- builds all the example apps
# make run   <- builds all the example apps, runs the first one it finds or the one matching the argument
# make gf2   <- same, but with gf2 debugger
# make val   <- same, but with valgrind

EXAMPLES_DIR := $(wildcard examples/*)

APP_NAME := $(firstword $(subst examples/,,$(EXAMPLES_DIR)))
ifdef APP
    APP_NAME := $(APP)
endif
TARGET_APP := examples/$(APP_NAME)

.PHONY: all build clean check run gf2 val

all: run

build:
	@for dir in $(EXAMPLES_DIR); do \
		$(MAKE) -C $$dir build; \
	done

clean:
	@for dir in $(EXAMPLES_DIR); do \
		$(MAKE) -C $$dir clean; \
	done

check:
	@for dir in $(EXAMPLES_DIR); do \
		$(MAKE) -C $$dir check; \
	done

run: build
	$(MAKE) -C $(TARGET_APP) run

gf2: build
	$(MAKE) -C $(TARGET_APP) gf2

val: build
	$(MAKE) -C $(TARGET_APP) valgrind
