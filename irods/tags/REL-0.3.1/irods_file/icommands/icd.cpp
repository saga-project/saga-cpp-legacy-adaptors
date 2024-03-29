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
//#include <fcntl.h>
//#include "rods.h"
//#include "parseCommandLine.h"
//#include "rodsPath.h"
//#include "miscUtil.h"
//#include "rcMisc.h"
//#include "genQuery.h"
//
//void usage (char *prog);

#include "irods_icommands.hpp"

namespace irods_file_adaptor {
	namespace api {
	int icommands::icd(int argc, char **argv)
	{
		int status, ix, i, fd, len;
		rodsArguments_t myRodsArgs;
		rodsEnv myEnv;
		char *envFile;
		char buffer[500];
		rodsPath_t rodsPath;
		rcComm_t *Conn;
		rErrMsg_t errMsg;

		status = parseCmdLineOpt(argc, argv, "vVh", 0, &myRodsArgs);
		if (status) {
		   printf("Use -h for help.\n");
		   exit(1);
		}
		if (myRodsArgs.help==True) {
//		   usage(argv[0]);
		   exit(0);
		}
		ix = myRodsArgs.optind;

		status = getRodsEnv (&myEnv);
		envFile= getRodsEnvFileName();

		/* Just "icd", so cd to home, so just remove the session file */
		/* (can do this for now, since session has only the cwd). */
		if (ix >= argc) {
		   status = unlink(envFile);
		   if (myRodsArgs.verbose==True) {
		  printf("Deleting (if it exists) session envFile:%s\n",envFile);
		  printf("unlink status = %d\n",status);
		   }
		   exit(0);
		}

		/* call parseRodsPath to handle the .. cases, etc. */
		if (strcmp(argv[ix], "/")==0) {  /* allow cd'ing to root */
		   strcpy(rodsPath.outPath, "/");
		}
		else {
		   memset((char*)&rodsPath, 0, sizeof(rodsPath));
		   rstrcpy(rodsPath.inPath, argv[ix], MAX_NAME_LEN);
		   parseRodsPath (&rodsPath, &myEnv);
		}

		/* Connect and check that the path exists */
		Conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
		  myEnv.rodsZone, 0, &errMsg);
		if (Conn == NULL) {
		   exit (2);
		}

		status = clientLogin(Conn);
		if (status != 0) {
		   rcDisconnect(Conn);
		   exit (7);
		}

		status = getRodsObjType (Conn, &rodsPath);
		rcDisconnect(Conn);

		if (status < 0) {
		   printError(Conn, status, "getRodsObjType");
		   exit(4);
		}

		if (rodsPath.objType != COLL_OBJ_T || rodsPath.objState != EXIST_ST) {
		   printf("No such directory (collection): %s\n", rodsPath.outPath);
		   exit(3);
		}


		/* open the sessionfile and write or update it */
		if ((fd = open(envFile, O_CREAT|O_RDWR, 0644)) < 0) {
			fprintf(stderr, "Unable to open envFile %s\n", envFile);
			exit(5);
		}
		sprintf(buffer,"irodsCwd=%s\n", rodsPath.outPath);
		len = strlen(buffer);
		i = write(fd, buffer, len);
		close(fd);
		if (i != len) {
		   fprintf(stderr, "Unable to write envFile %s\n", envFile);
		   exit(6);
		}

		return(0);
	}
	}
}

///* Check to see if a collection exists */
//int
//checkColl(rcComm_t *Conn, char *path) {
//   genQueryInp_t genQueryInp;
//   genQueryOut_t *genQueryOut;
//   int i1a[10];
//   int i1b[10];
//   int i2a[10];
//   char *condVal[2];
//   char v1[MAX_NAME_LEN];
//   int status;
//
//   memset (&genQueryInp, 0, sizeof (genQueryInp_t));
//
//   i1a[0]=COL_COLL_ID;
//   i1b[0]=0; /* currently unused */
//   genQueryInp.selectInp.inx = i1a;
//   genQueryInp.selectInp.value = i1b;
//   genQueryInp.selectInp.len = 1;
//
//   i2a[0]=COL_COLL_NAME;
//   genQueryInp.sqlCondInp.inx = i2a;
//   sprintf(v1,"='%s'",path);
//   condVal[0]=v1;
//   genQueryInp.sqlCondInp.value = condVal;
//   genQueryInp.sqlCondInp.len=1;
//
//   genQueryInp.maxRows=10;
//   genQueryInp.continueInx=0;
//   status = rcGenQuery(Conn, &genQueryInp, &genQueryOut);
//   return(status);
//}
//
//void usage (char *prog)
//{
//   printf("Changes iRODS the current working directory (collection)\n");
//   printf("Usage: %s [-vh] [directory]\n", prog);
//   printf("If no directory is entered, the cwd is set back to your home\n");
//   printf("directory as defined in your .rodsEnv file.\n");
//   printf("Like the unix 'cd', '..' will move up one level and \n");
//   printf("'/name' starts at the root.\n");
//   printf(" -v  verbose\n");
//   printf(" -V  very verbose\n");
//   printf(" -h  this help\n");
//   printReleaseInfo("icd");
//}
