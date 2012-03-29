/*
 * Copyright (C) 2008-2011 Osaka University.
 * Copyright (C) 2008-2011 National Institute of Informatics in Japan.
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
	fprintf(stdout, "usage: %s pathname key\n", name);
}

int
main(int argc, char **argv)
{
	char *path, *key;
	int size;
	char *l;
	char str[1024];

	if (argc != 3) {
		usage(argv[0]);
		return 1;
	}
	path = argv[1];
	key = argv[2];

	snprintf(str, sizeof(str), "rnskv.%s", key);

	size = getxattr(path, str, NULL, 0);
	l = malloc(size + 1);
	if (l == NULL) {
		fprintf(stderr, "malloc failed\n");
		return 2;
	}
	if (getxattr(path, str, l, size) == -1) {
		perror("getxattr");
		return 3;
	}
	l[size] = '\0';
	printf("%s", l);

	free(l);

	return 0;
}
