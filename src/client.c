#include "hauva.h"

/*
 * Connect to the hauva daemon via unix domain socket
 */
int connect_daemon()
{
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	struct sockaddr_un addr = { .sun_family = AF_UNIX };
	strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		fprintf(stderr,
			"Failed to connect to hauva daemon. Is it running?\n");
		return -1;
	}
	return fd;
}

/*
 * Read all data from stdin into a dynamically allocated buf
 */
char *read_stdin(size_t *len)
{
	char *buffer = NULL;
	size_t capacity = 0;
	size_t length = 0;
	char chunk[1024];
	size_t n;

	while ((n = fread(chunk, 1, sizeof(chunk), stdin)) > 0) {
		if (length + n > capacity) {
			capacity = length + n + 1024;
			buffer = realloc(buffer, capacity);
		}
		memcpy(buffer + length, chunk, n);
		length += n;
	}
	if (buffer)
		buffer[length] = '\0';
	*len = length;
	return buffer;
}

/*
 * Hauva client
 * Usage:
 *   hauva add   - adds clipboard entry from stdin
 *   hauva list  - lists clipboard entries
 *   hauva copy  - copies selected entry from stdin (e.g. wmenu output)
 */
int main(int argc, char *argv[])
{
	if (argc < 2)
		return 1;

	int fd = connect_daemon();
	if (fd < 0)
		return 1;

	struct msg_header header;

	if (strcmp(argv[1], "add") == 0) {
		size_t len;
		char *data = read_stdin(&len);
		if (data && len > 0) {
			header.op = OP_ADD;
			header.data_len = len;
			send(fd, &header, sizeof(header), 0);
			send(fd, data, len, 0);
		}
		free(data);
	} else if (strcmp(argv[1], "list") == 0) {
		header.op = OP_LIST;
		header.data_len = 0;
		send(fd, &header, sizeof(header), 0);

		char buf[1024];
		ssize_t n;
		while ((n = recv(fd, buf, sizeof(buf), 0)) > 0)
			write(STDOUT_FILENO, buf, n);
	} else if (strcmp(argv[1], "copy") == 0) {
		// Reads the sellected line from wmenu (e.g. "0: text...")
		size_t len;
		char *data = read_stdin(&len);
		if (data && len > 0) {
			header.op = OP_COPY;
			header.data_len = len;
			send(fd, &header, sizeof(header), 0);
			send(fd, data, len, 0);
		}
		free(data);
	}

	close(fd);
	return 0;
}
