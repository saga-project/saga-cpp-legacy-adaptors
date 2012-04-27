#include <stdio.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <stdlib.h>

static void
usage(const char *name)
{
	fprintf(stdout, "usage: %s pathname\n", name);
}

int
main(int argc, char **argv)
{
	char *path;
	int size;
	char *l;

	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}
	path = argv[1];

	size = getxattr(path, "xml", NULL, 0);
	l = malloc(size + 1);
	if (l == NULL) {
		fprintf(stderr, "malloc failed\n");
		return 2;
	}
	if (getxattr(path, "xml", l, size) == -1) {
		perror("getxattr");
		return 3;
	}
	l[size] = '\0';
	printf("%s", l);

	return 0;
}
