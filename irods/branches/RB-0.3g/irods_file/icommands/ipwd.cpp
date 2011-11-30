/*
 * Copyright (C) 2008-2011 High Energy Accelerator Research Organization (KEK)
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

/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
//#include "rods.h"
//#include "parseCommandLine.h"
//#include "rcMisc.h"
//
//void usage (char *prog);

#include "irods_icommands.hpp"

namespace irods_file_adaptor {
	namespace api {

	int icommands::ipwd(int argc, char **argv)
	{
		int status;
		rodsArguments_t myRodsArgs;
		rodsEnv myEnv;

		status = parseCmdLineOpt(argc, argv, "vVh", 0, &myRodsArgs);
		if (status) {
		   printf("Use -h for help\n");
		   exit(1);
		}

		if (myRodsArgs.help==True) {
//		   usage(argv[0]);
		   exit(0);
		}

		status = getRodsEnv (&myEnv);
		if (status != 0) {
		  printf("Failed with error %d\n", status);
		  exit(2);
		}

		printf ("%s\n", myEnv.rodsCwd);

		exit (0);
	}
	}
}

//void usage (char *prog)
//{
//  printf("Shows your iRODS Current Working Directory.\n");
//  printf("Usage: %s [-vVh]\n", prog);
//  printf(" -v  verbose\n");
//  printf(" -V  very verbose\n");
//  printf(" -h  this help\n");
//  printReleaseInfo("ipwd");
//}
