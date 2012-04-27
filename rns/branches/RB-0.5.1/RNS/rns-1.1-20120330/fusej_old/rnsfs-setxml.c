#include <stdio.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static void
usage(const char *name)
{
	fprintf(stdout, "usage: %s xmlfilename pathname\n", name);
}

int
main(int argc, char **argv)
{
	char *path, *xml;
	FILE *input;
	int res = 0;
	int fd;
	struct stat stbuf;
	char *b;
	int size;

	if (argc != 3) {
		usage(argv[0]);
		return 1;
	}
	xml = argv[1];
	path = argv[2];
	if (strcmp("-", xml) == 0) {
		/* not implemented */
		input = stdin;
		usage(argv[0]);
		return 1;
	} else {
		input = fopen(xml, "r");
	}
	if (input == NULL) {
		perror("fopen");
		return 2;
	}
	fd = fileno(input);
	fstat(fd, &stbuf);
	if (stbuf.st_size <= 0) {
		fprintf(stderr, "input file is empty\n");
		res = 3;
		goto end_close;
	}
	b = malloc(stbuf.st_size);
	if (b == NULL) {
		fprintf(stderr, "malloc failed\n");
		res = 4;
		goto end_close;
	}
	size = fread(b, 1, stbuf.st_size, input);
	if (ferror(input) != 0) {
		fprintf(stderr, "fread failed\n");
		res = 5;
		goto end_free;
	}

	res = setxattr(path, "xml", b, size, XATTR_REPLACE);
	if (res == -1) {
		perror("setxattr");
		res = 6;
		goto end_free;
	}
end_free:
	free(b);
end_close:
	if (input != stdin) {
		fclose(input);
	}

	return res;
}
