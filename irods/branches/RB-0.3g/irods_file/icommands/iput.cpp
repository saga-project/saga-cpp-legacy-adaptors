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
 * iput - The irods put utility
*/

//#include "rodsClient.h"
//#include "parseCommandLine.h"
//#include "rodsPath.h"
//#include "putUtil.h"
//void usage ();

#include "irods_icommands.hpp"

namespace irods_file_adaptor {
	namespace api {

	int icommands::iput(int argc, char **argv) {

		int status;
		rodsEnv myEnv;
		rErrMsg_t errMsg;
		rcComm_t *conn;
		rodsArguments_t myRodsArgs;
		char *optStr;
		rodsPathInp_t rodsPathInp;
		int reconnFlag;

		optind = 1;
		optStr = "aD:fhkKn:N:p:rR:QTvVX:";

		status = parseCmdLineOpt (argc, argv, optStr, 0, &myRodsArgs);

//		for(int i=0; i<argc; i++){
//			printf("argv[%d]=%s\n",i,argv[i]);
//		}

		if (status < 0) {
		printf("use -h for help.\n");
			exit (1);
		}

//		if (myRodsArgs.help==True) {
////		   usage();
//		   exit(0);
//		}

		status = getRodsEnv (&myEnv);
		if (status < 0) {
			rodsLogError (LOG_ERROR, status, "main: getRodsEnv error. ");
			exit (1);
		}

		status = parseCmdLinePath (argc, argv, optind, &myEnv,
		  UNKNOWN_FILE_T, UNKNOWN_OBJ_T, 0, &rodsPathInp);

//		printf("srcPath->inPath:%s \n",(rodsPathInp.srcPath)->inPath);
//		printf("srcPath->outPath:%s \n",(rodsPathInp.srcPath)->outPath);
//		printf("destPath->inPath:%s \n",(rodsPathInp.destPath)->inPath);
//		printf("destPath->outPath:%s \n",(rodsPathInp.destPath)->outPath);
//		printf("targPath->inPath:%s \n",(rodsPathInp.targPath)->inPath);
//		printf("targPath->outPath:%s \n",(rodsPathInp.targPath)->outPath);

		if (status < 0) {
			rodsLogError (LOG_ERROR, status, "main: parseCmdLinePath error. ");
		printf("use -h for help.\n");
			exit (1);
		}

		if (myRodsArgs.reconnect == True) {
			reconnFlag = RECONN_TIMEOUT;
		} else {
			reconnFlag = NO_RECONN;
		}

		conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
		  myEnv.rodsZone, reconnFlag, &errMsg);

		if (conn == NULL) {
			exit (2);
		}

		status = clientLogin(conn);
		if (status != 0) {
		   rcDisconnect(conn);
			exit (7);
		}

//		char buf[1] = "";
//		strcpy((rodsPathInp.srcPath)->inPath, argv[1]);
//		strcpy((rodsPathInp.srcPath)->outPath, argv[1]);
//		strcpy((rodsPathInp.destPath)->inPath, argv[2]);
//		strcpy((rodsPathInp.destPath)->outPath, argv[2]);
//		strcpy((rodsPathInp.targPath)->inPath, buf);
//		strcpy((rodsPathInp.targPath)->outPath, buf);


//	    if (myRodsArgs.progressFlag == True) {
//	        gGuiProgressCB = (irodsGuiProgressCallbak) iCommandProgStat;
//	    }

//		status = putUtil (conn, &myEnv, &myRodsArgs, &rodsPathInp);
		status = putUtil (&conn, &myEnv, &myRodsArgs, &rodsPathInp);

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
//usage ()
//{
//   char *msgs[]={
//"Usage : iput [-fkKQrTUvV] [-D dataType] [-N numThreads] [-n replNum]",
//"             [-p physicalPath] [-R resource] [-X restartFile]",
//"		localSrcFile|localSrcDir ...  destDataObj|destColl",
//"Usage : iput [-fkKQTUvV] [-D dataType] [-N numThreads] [-n replNum] ",
//"             [-p physicalPath] [-R resource] [-X restartFile] localSrcFile",
//" ",
//"Store a file into iRODS.  If the destination data-object or collection are",
//"not provided, the current irods directory and the input file name are used.",
//"The -X option specifies that the restart option is on and the restartFile",
//"input specifies a local file that contains the restart info. If the ",
//"restartFile does not exist, it will be created and used for recording ",
//"subsequent restart info. If it exists and is not empty, the restart info",
//"contained in this file will be used for restarting the operation.",
//"Note that the restart operation only works for uploading directories and",
//"the path input must be identical to the one that generated the restart file",
//" ",
//"If the options -f is used to overwrite an existing data-object, the copy",
//"in the resource specified by the -R option will be picked if it exists.",
//"Otherwise, one of the copy in the other resources will be picked for the",
//"overwrite. Note that a copy will not be made in the specified resource",
//"if a copy in the specified resource does not already exist. The irepl",
//"command should be used to make a replica of an existing copy.",
//" ",
//"The -Q option specifies the use of the RBUDP transfer mechanism which uses",
//"the UDP protocol for data transfer. The UDP protocol is very efficient",
//"if the network is very robust with few packet losses. Two environment",
//"variables - rbudpSendRate and rbudpPackSize are used to tune the RBUDP",
//"data transfer. rbudpSendRate is used to throttle the send rate in ",
//"kbits/sec. The default rbudpSendRate is 600,000. rbudpPackSize is used",
//"to set the packet size. The dafault rbudpPackSize is 8192.",
//" ",
//"The -T option will renew the socket connection between the client and ",
//"server after 10 minutes of connection. This gets around the problem of",
//"sockets getting timed out by the firewall as reported by some users.",
//" ",
//"Options are:",
//" -D  dataType - the data type string",
//" -f  force - write data-object even it exists already; overwrite it",
//" -k  checksum - calculate a checksum on the data",
//" -K  verify checksum - calculate and verify the checksum on the data",
//" -N  numThreads - the number of thread to use for the transfer. A value of",
//"       0 means no threading. By default (-N option not used) the server ",
//"       decides the number of threads to use.",
//" -p physicalPath - the physical path of the uploaded file on the sever ",
//" -Q  use RBUDP (datagram) protocol for the data transfer",
//" -R  resource - specifies the resource to store to. This can also be specified",
//"     in your environment or via a rule set up by the administrator.",
//" -r  recursive - store the whole subdirectory",
//" -T  renew socket connection after 10 minutes",
//" -v  verbose",
//" -V  Very verbose",
//" -X  restartFile - specifies that the restart option is on and the",
//"     restartFile input specifies a local file that contains the restart info.",
//
//" -h  this help",
//""};
//   int i;
//   for (i=0;;i++) {
//      if (strlen(msgs[i])==0) break;
//      printf("%s\n",msgs[i]);
//   }
//   printReleaseInfo("iput");
//}
