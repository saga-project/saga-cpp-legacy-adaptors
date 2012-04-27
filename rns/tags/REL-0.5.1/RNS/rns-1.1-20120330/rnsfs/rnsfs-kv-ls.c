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
#include <string.h>
#include <stdlib.h>

static void
usage(const char *name)
{
	fprintf(stdout, "usage: %s path\n", name);
}

#define RNSKV_KEY "rnskv."
#define RNSKV_KEYLEN 6

int
main(int argc, char **argv)
{
	char *path;
	ssize_t size, pos;
	char *v, *now;

	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}
	path = argv[1];

	size = listxattr(path, NULL, 0);
	if (size == -1) {
		perror("listxattr");
		return 2;
	}
	v = malloc(size);
	if (v == NULL) {
		fprintf(stderr, "malloc failed\n");
		return 3;
	}
	size = listxattr(path, v, size);
	if (size == -1) {
		perror("listxattr");
		return 4;
	}
	pos = 0;
	now = v;
	while (pos < size) {
		if (v[pos] == '\0') {
			if (strncmp(now, RNSKV_KEY, RNSKV_KEYLEN) == 0) {
				printf("%s\n", now + RNSKV_KEYLEN);
			}
			now = &v[pos + 1]; /* next */
		}
		pos++;
	}
	free(v);

	return 0;
}
