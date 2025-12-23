#include "hauva.h"

struct clip_entry history[MAX_ENTRIES];
int entry_count = 0;
char cache_path[512];

/*
 * Get the path to history cache file (~/.cache/hauva_history)
 */
void get_cache_path()
{
	const char *home = getenv("HOME");
	snprintf(cache_path, sizeof(cache_path), "%s/.cache/hauva_history",
		 home);
}

/*
 * Convert newlines to until separator (0x1F)
 * This makes storing multiline entries in a line-based file easier
 */
void save_history()
{
	FILE *f = fopen(cache_path, "w");
	if (!f)
		return;
	for (int i = 0; i < entry_count; i++) {
		for (size_t j = 0; j < history[i].length; j++)
			if (history[i].content[j] == '\n')
				fputc(0x1F, f);
			else
				fputc(history[i].content[j], f);
		fputc('\n', f);
	}
	fclose(f);
}

/*
 * Loads the history from the cache file into memory
 * Then restores newlines from separator (0x1F), and removes trailing newlines
 */
void load_history()
{
	FILE *f = fopen(cache_path, "r");
	if (!f)
		return;
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, f) != -1 && entry_count < MAX_ENTRIES) {
		line[strcspn(line, "\n")] = 0;

		char *restore = line;
		while (*restore) {
			if (*restore == 0x1F)
				*restore = '\n';
			restore++;
		}

		history[entry_count].content = strdup(line);
		history[entry_count].length = strlen(line);
		entry_count++;
	}
	free(line);
	fclose(f);
}

/*
 * Add a new clipboard entry to history, de-duplicating and maintaining size
 * limit
 */
void add_entry(const char *data, size_t len)
{
	if (len == 0)
		return;

	// De-duplicate
	if (entry_count > 0 && strcmp(history[0].content, data) == 0)
		return;

	if (entry_count == MAX_ENTRIES) {
		free(history[MAX_ENTRIES - 1].content);
		entry_count--;
	}

	// Shift entries down
	for (int i = entry_count; i > 0; i--)
		history[i] = history[i - 1];

	history[0].content = strdup(data);
	history[0].length = len;
	entry_count++;
	save_history();
}

/*
 * Send squashed list of entries to client
 * index: content... (up to first newline, tabs as spaces)
 * Note: wmenu and dmenu seperate entries by newlines, so each entry must be a
 * single line.
 */
void send_list(int client_fd)
{
	for (int i = 0; i < entry_count; i++) {
		dprintf(client_fd, "%d: ", i);

		int has_newline = 0;
		for (size_t j = 0; j < history[i].length; j++) {
			char c = history[i].content[j];
			if (c == '\n') {
				has_newline = 1;
				break;
			} else if (c == '\t') {
				write(client_fd, " ", 1);
			} else {
				write(client_fd, &c, 1);
			}
		}
		if (has_newline)
			write(client_fd, "...", 3);
		write(client_fd, "\n", 1);
	}
}

/*
 * Copy entry by index parsed from input line (e.g. "0: some text...")
 * This requires wl-copy!
 */
void copy_entry_by_index(const char *input_line)
{
	int index = -1;
	if (sscanf(input_line, "%d:", &index) != 1)
		return;

	if (index >= 0 && index < entry_count) {
		FILE *pipe = popen("wl-copy", "w");
		if (pipe) {
			fwrite(history[index].content, 1, history[index].length,
			       pipe);
			pclose(pipe);
		}
	}
}

/*
 * Handle a connected client
 * For OP_ADD, receives data and adds entry
 * For OP_LIST, sends list of entries
 * For OP_COPY, receives line, parses index, copies entry
 * IPC is doen with unix domain sockets /tmp/hauva.sock
 * Very insecure, and error prone, but since this is local and using unix
 * sockets (AF_UNIX) that are very reliable most of the time unlike something
 * like TCP, it's fine for now
 */
void handle_client(int client_fd)
{
	struct msg_header header;
	if (recv(client_fd, &header, sizeof(header), 0) <= 0)
		return;

	if (header.op == OP_ADD) {
		char *buffer = malloc(header.data_len + 1);
		recv(client_fd, buffer, header.data_len, 0);
		buffer[header.data_len] = '\0';
		add_entry(buffer, header.data_len);
		free(buffer);
	} else if (header.op == OP_LIST) {
		send_list(client_fd);
	} else if (header.op == OP_COPY) {
		char *buffer = malloc(header.data_len + 1);
		recv(client_fd, buffer, header.data_len, 0);
		buffer[header.data_len] = '\0';
		copy_entry_by_index(buffer);
		free(buffer);
	}
	close(client_fd);
}

/*
 * Hauva daemon, using unix domain sockets for IPC
 */
int main()
{
	get_cache_path();
	load_history();
	unlink(SOCKET_PATH);

	int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	struct sockaddr_un addr = { .sun_family = AF_UNIX };
	strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

	bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
	listen(server_fd, 5);

	while (1) {
		int client_fd = accept(server_fd, NULL, NULL);
		if (client_fd >= 0)
			handle_client(client_fd);
	}
	return 0;
}
