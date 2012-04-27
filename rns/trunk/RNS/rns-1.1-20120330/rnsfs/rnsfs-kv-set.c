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

static void
usage(const char *name)
{
	fprintf(stdout, "usage: %s path key value\n", name);
}

int
main(int argc, char **argv)
{
	char *path, *key, *value;
	int res = 0;
	char k[1024];

	if (argc != 4) {
		usage(argv[0]);
		return 1;
	}
	path = argv[1];
	key = argv[2];
	value = argv[3];

	snprintf(k, sizeof(k), "rnskv.%s", key);

	res = setxattr(path, k, value, strlen(value), XATTR_REPLACE);
	if (res == -1) {
		perror("setxattr");
		return 2;
	}
	return 0;
}
