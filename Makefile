CC = gcc
CFLAGS = -Wall -O2

BUILD_DIR = build
PREFIX ?= /usr/local

all: $(BUILD_DIR) $(BUILD_DIR)/hauva $(BUILD_DIR)/hauvad

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/hauva: src/client.c src/hauva.h
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/hauva src/client.c

$(BUILD_DIR)/hauvad: src/daemon.c src/hauva.h
	$(CC) $(CFLAGS) -pthread -o $(BUILD_DIR)/hauvad src/daemon.c

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $(BUILD_DIR)/hauva $(BUILD_DIR)/hauvad $(DESTDIR)$(PREFIX)/bin/
	cp hauva-ui.sh $(DESTDIR)$(PREFIX)/bin/hauva-ui
	chmod +x $(DESTDIR)$(PREFIX)/bin/hauva-ui

clean:
	rm -rf $(BUILD_DIR)
