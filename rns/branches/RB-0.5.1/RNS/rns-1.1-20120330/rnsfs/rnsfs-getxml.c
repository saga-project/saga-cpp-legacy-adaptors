/*
 * Copyright (C) 2008-2012 Osaka University.
 * Copyright (C) 2008-2012 National Institute of Informatics in Japan.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <stdlib.h>

static void
usage(const char *name)
{
	fprintf(stdout, "usage: %s path\n", name);
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

	size = getxattr(path, "rns.xml", NULL, 0);
	l = malloc(size + 1);
	if (l == NULL) {
		fprintf(stderr, "malloc failed\n");
		return 2;
	}
	if (getxattr(path, "rns.xml", l, size) == -1) {
		perror("getxattr");
		return 3;
	}
	l[size] = '\0';
	printf("%s", l);

	free(l);

	return 0;
}
