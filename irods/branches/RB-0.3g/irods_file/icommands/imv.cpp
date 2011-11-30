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
/*
 * imv - The irods mv utility
*/

//#include "rodsClient.h"
//#include "parseCommandLine.h"
//#include "rodsPath.h"
//#include "mvUtil.h"
//void usage (char *program);

#include "irods_icommands.hpp"

namespace irods_file_adaptor {
	namespace api {

	int icommands::imv(int argc, char **argv) {
		int status;
		rodsEnv myEnv;
		rErrMsg_t errMsg;
		rcComm_t *conn;
		rodsArguments_t myRodsArgs;
		char *optStr;
		rodsPathInp_t rodsPathInp;


		optStr = "hvV";

		status = parseCmdLineOpt (argc, argv, optStr, 0, &myRodsArgs);
		if (status) {
		   printf("Use -h for help.\n");
		   exit(1);
		}
		if (myRodsArgs.help==True) {
//		   usage(argv[0]);
		   exit(0);
		}

		if (argc - optind <= 1) {
			rodsLog (LOG_ERROR, "imv: no input");
		printf("Use -h for help.\n");
			exit (2);
		}

		status = getRodsEnv (&myEnv);

		if (status < 0) {
			rodsLogError (LOG_ERROR, status, "main: getRodsEnv error. ");
			exit (1);
		}

		status = parseCmdLinePath (argc, argv, optind, &myEnv,
		  UNKNOWN_OBJ_T, UNKNOWN_OBJ_T, 0, &rodsPathInp);

		if (status < 0) {
			rodsLogError (LOG_ERROR, status, "main: parseCmdLinePath error. ");
		printf("Use -h for help.\n");
			exit (1);
		}

		conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
		  myEnv.rodsZone, 1, &errMsg);

		if (conn == NULL) {
			exit (2);
		}

		status = clientLogin(conn);
		if (status != 0) {
		   rcDisconnect(conn);
			exit (7);
		}

		status = mvUtil (conn, &myEnv, &myRodsArgs, &rodsPathInp);

		rcDisconnect(conn);

		if (status < 0) {
		exit (3);
		}
		else {
//			exit(0);
			return 0;
		}

	}
	}
}

//void
//usage (char *program)
//{
//   int i;
//   char *msgs[]={
//"imv moves/renames an irods data-object (file) or collection (directory) to "
//"another, data-object or collection.  Options are:",
//"-v verbose - display various messages while processing",
//"-V very verbose",
//"-h help - this help",
//""};
//    printf ("Usage : %s [-hvV] srcDataObj|srcColl ...  destDataObj|destColl\n", program);
//    for (i=0;;i++) {
//       if (strlen(msgs[i])==0) break;
//       printf("%s\n",msgs[i]);
//    }
//    printReleaseInfo("imv");
//}
