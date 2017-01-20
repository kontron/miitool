TOPDIR := $(shell pwd)

VERSION := 1.0

# install directories
PREFIX ?= /usr
SBINDIR ?= $(PREFIX)/sbin
DESTDIR ?=

ifeq ($(shell test -d .git && echo 1),1)
VERSION := $(shell git describe --abbrev=8 --dirty --always --tags --long)
endif

EXTRA_CFLAGS = -Wall -DVERSION=\"$(VERSION)\"

SOURCES := $(wildcard *.c)
OBJECTS := $(SOURCES:.c=.o)

programs = miitool

.PHONY: all install
all: miitool

clean:
	rm -f $(OBJECTS) miitool

%.o: %.c
	$(CC) -c $< $(CLFAGS) $(EXTRA_CFLAGS)

miitool: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

install: miitool
	install -D -m 0755 miitool $(DESTDIR)$(SBINDIR)/miitool
