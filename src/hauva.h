#ifndef HAUVA_H
#define HAUVA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/hauva.sock"
#define MAX_ENTRIES 50

/*
 * Op codes for IPC messages
 * OP_ADD  - Add a new clipboard entry
 * OP_LIST - List all clipboard entries
 * OP_COPY - Copy a specific clipboard entry to clipboard
 */
enum op_code { OP_ADD = 1, OP_LIST, OP_COPY };

struct msg_header {
	enum op_code op;
	size_t data_len;
};

struct clip_entry {
	char *content;
	size_t length;
};

extern void add_entry(const char *data, size_t len);
extern void load_history();
extern void save_history();
extern void get_cache_path();
extern int connect_daemon();

#endif /* HAUVA_H */
