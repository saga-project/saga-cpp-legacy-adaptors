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
 * imkdir - The irods mkdir utility
*/

//#include "rodsClient.h"
//#include "parseCommandLine.h"
//#include "rodsPath.h"
//#include "mkdirUtil.h"
//void usage ();

#include "irods_icommands.hpp"

namespace irods_file_adaptor {
	namespace api {

	int icommands::imkdir(int argc, char **argv) {
		int status;
		rodsEnv myEnv;
		rErrMsg_t errMsg;
		rcComm_t *conn;
		rodsArguments_t myRodsArgs;
		char *optStr;
		rodsPathInp_t rodsPathInp;

		optStr = "phvV";

		status = parseCmdLineOpt (argc, argv, optStr, 0, &myRodsArgs);

		if (status < 0) {
			printf("use -h for help\n");
			exit (1);
		}
		if (myRodsArgs.help==True) {
//		   usage();
		   exit(0);
		}

		status = getRodsEnv (&myEnv);

		if (status < 0) {
			rodsLogError(LOG_ERROR, status, "main: getRodsEnv error. ");
			exit (1);
		}

		status = parseCmdLinePath (argc, argv, optind, &myEnv,
		  UNKNOWN_OBJ_T, NO_INPUT_T, 0, &rodsPathInp);

		if (status < 0) {
			rodsLogError (LOG_ERROR, status, "main: parseCmdLinePath error. ");
			exit (1);
		}

		conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
		  myEnv.rodsZone, 0, &errMsg);

		if (conn == NULL) {
			exit (2);
		}

		status = clientLogin(conn);
		if (status != 0) {
			rcDisconnect(conn);
			exit (7);
		}

		status = mkdirUtil (conn, &myEnv, &myRodsArgs, &rodsPathInp);

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
//usage () {
//   char *msgs[]={
//"Usage: imkdir [-phvV] collection ...",
//"Create one or more new collections",
//"Options are:",
//" -p  make parent directories as needed",
//" -v  verbose",
//" -V  Very verbose",
//" -h  this help",
//""};
//   int i;
//   for (i=0;;i++) {
//      if (strlen(msgs[i])==0) break;
//      printf("%s\n",msgs[i]);
//   }
//   printReleaseInfo("imkdir");
//}
