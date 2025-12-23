CC = gcc
CFLAGS = -Wall -O2

BUILD_DIR = build

all: $(BUILD_DIR) $(BUILD_DIR)/hauva $(BUILD_DIR)/hauvad

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/hauva: src/client.c src/hauva.h
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/hauva src/client.c

$(BUILD_DIR)/hauvad: src/daemon.c src/hauva.h
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/hauvad src/daemon.c

install:
	sudo cp $(BUILD_DIR)/hauva $(BUILD_DIR)/hauvad /usr/local/bin/
	sudo cp hauva-ui.sh /usr/local/bin/hauva-ui
	sudo chmod +x /usr/local/bin/hauva-ui

clean:
	rm -rf $(BUILD_DIR)
