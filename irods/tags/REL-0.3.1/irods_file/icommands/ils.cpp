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

/*
 * ils - The irods ls utility
*/

#include "irods_icommands.hpp"

namespace irods_file_adaptor {
	namespace api {

	int icommands::ils(int argc, char **argv) {
		int status;
		rodsEnv myEnv;
		rErrMsg_t errMsg;
		rcComm_t *conn;
		rodsArguments_t myRodsArgs;
		char *optStr;
		rodsPathInp_t rodsPathInp;

		optStr = "hArlLvV";

		status = parseCmdLineOpt (argc, argv, optStr, 0, &myRodsArgs);

		if (status < 0) {
			printf("Use -h for help\n");
			exit (1);
		}
		if (myRodsArgs.help==True) {
		   //usage();
		   exit(0);
		}

		status = getRodsEnv (&myEnv);

		if (status < 0) {
			rodsLogError (LOG_ERROR, status, "main: getRodsEnv error. ");
			exit (1);
		}

		status = parseCmdLinePath (argc, argv, optind, &myEnv,
		  UNKNOWN_OBJ_T, NO_INPUT_T, ALLOW_NO_SRC_FLAG, &rodsPathInp);

		if (status < 0) {
			rodsLogError (LOG_ERROR, status, "main: parseCmdLinePath error. ");
		//usage ();
			exit (1);
		}

		conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
		  myEnv.rodsZone, 0, &errMsg);

		if (conn == NULL) {
			exit (2);
		}

		if (strcmp (myEnv.rodsUserName, PUBLIC_USER_NAME) != 0) {
			status = clientLogin(conn);
			if (status != 0) {
			   rcDisconnect(conn);
			   exit (7);
		}
		}

//		status = lsUtil (conn, &myEnv, &myRodsArgs, &rodsPathInp);
		status = lsUtil_sg (conn, &myEnv, &myRodsArgs, &rodsPathInp);

		rcDisconnect(conn);

		if (status < 0) {
		exit (4);
		}
		else {
//			exit(0);
			return 0;
		}

	}


	int icommands::lsUtil_sg (rcComm_t *conn, rodsEnv *myRodsEnv,
			rodsArguments_t *myRodsArgs, rodsPathInp_t *rodsPathInp)
	{
	    int i;
	    int status;
	    int savedStatus = 0;
	    genQueryInp_t genQueryInp;


	    if (rodsPathInp == NULL) {
		return (USER__NULL_INPUT_ERR);
	    }

	    initCondForLs (myRodsEnv, myRodsArgs, &genQueryInp);

	    for (i = 0; i < rodsPathInp->numSrc; i++) {
		if (rodsPathInp->srcPath[i].objType == UNKNOWN_OBJ_T ||
		  rodsPathInp->srcPath[i].objState == UNKNOWN_ST) {
		    status = getRodsObjType (conn, &rodsPathInp->srcPath[i]);
		    if (rodsPathInp->srcPath[i].objState == NOT_EXIST_ST) {
	                if (status == NOT_EXIST_ST) {
	                    rodsLog (LOG_ERROR,
	                      "lsUtil: srcPath %s does not exist or user lacks access permission",
	                      rodsPathInp->srcPath[i].outPath);
			}
			savedStatus = USER_INPUT_PATH_ERR;
			continue;
		    }
		}

		if (rodsPathInp->srcPath[i].objType == DATA_OBJ_T) {
		    status = lsDataObjUtil_sg (conn, &rodsPathInp->srcPath[i],
		     myRodsEnv, myRodsArgs, &genQueryInp);
		} else if (rodsPathInp->srcPath[i].objType ==  COLL_OBJ_T) {
		    status = lsCollUtil_sg (conn, &rodsPathInp->srcPath[i],
	              myRodsEnv, myRodsArgs);
		} else {
			printf("ils test3: %s \n", rodsPathInp->srcPath[i].outPath);
		    /* should not be here */
		    rodsLog (LOG_ERROR,
		     "lsUtil: invalid ls objType %d for %s",
		     rodsPathInp->srcPath[i].objType, rodsPathInp->srcPath[i].outPath);
		    return (USER_INPUT_PATH_ERR);
		}
		/* XXXX may need to return a global status */
		if (status < 0 && status != CAT_NO_ROWS_FOUND &&
		  status != SYS_SPEC_COLL_OBJ_NOT_EXIST) {
		    rodsLogError (LOG_ERROR, status,
	             "lsUtil: ls error for %s, status = %d",
		      rodsPathInp->srcPath[i].outPath, status);
		    savedStatus = status;
		}
	    }
	    if (savedStatus < 0) {
	        return (savedStatus);
	    } else if (status == CAT_NO_ROWS_FOUND ||
	      status == SYS_SPEC_COLL_OBJ_NOT_EXIST) {
	        return (0);
	    } else {
	        return (status);
	    }
	}

	int icommands::lsDataObjUtil_sg (rcComm_t *conn, rodsPath_t *srcPath,
	rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs,
	genQueryInp_t *genQueryInp)
	{
	    int status = 0;

	    if (rodsArgs->longOption == True) {
		if (srcPath->rodsObjStat != NULL &&
		  srcPath->rodsObjStat->specColl != NULL) {
		    lsSpecDataObjUtilLong (conn, srcPath,
		      myRodsEnv, rodsArgs);
		} else {
	            lsDataObjUtilLong (conn, srcPath->outPath, myRodsEnv, rodsArgs,
		      genQueryInp);
		}
	    } else {
		printLsStrShort (srcPath->outPath);
	        if (rodsArgs->accessControl == True) {
	            printDataAcl (conn, srcPath->dataId);
	        }
	    }

	    if (srcPath == NULL) {
	       rodsLog (LOG_ERROR,
	          "lsDataObjUtil: NULL srcPath input");
	        return (USER__NULL_INPUT_ERR);
	    }

	    return (status);
	}

	int icommands::lsCollUtil_sg (rcComm_t *conn, rodsPath_t *srcPath, rodsEnv *myRodsEnv,
	rodsArguments_t *rodsArgs)
	{
	    int savedStatus = 0;
	    char *srcColl;
	    int status;
	    int queryFlags;
	    collHandle_t collHandle;
	    collEnt_t collEnt;

	    if (srcPath == NULL) {
	       rodsLog (LOG_ERROR,
	          "lsCollUtil: NULL srcPath input");
	        return (USER__NULL_INPUT_ERR);
	    }

	    srcColl = srcPath->outPath;

	    /* print this collection */
	    printf ("%s:\n", srcColl);

	    if (rodsArgs->accessControl == True) {
	       printCollAcl (conn, srcColl);
	       printCollInheritance (conn, srcColl);
	    }

	    queryFlags = DATA_QUERY_FIRST_FG;
	    if (rodsArgs->veryLongOption == True) {
		/* need to check veryLongOption first since it will have both
		 * veryLongOption and longOption flags on. */
		queryFlags = queryFlags | VERY_LONG_METADATA_FG | NO_TRIM_REPL_FG;
	    } else if (rodsArgs->longOption == True) {
		queryFlags |= LONG_METADATA_FG | NO_TRIM_REPL_FG;;
	    }

	    status = rclOpenCollection (conn, srcColl, queryFlags,
	      &collHandle);

	    if (status < 0) {
	        rodsLog (LOG_ERROR,
	          "lsCollUtil: rclOpenCollection of %s error. status = %d",
	          srcColl, status);
	        return status;
	    }
	    while ((status = rclReadCollection (conn, &collHandle, &collEnt)) >= 0) {
		if (collEnt.objType == DATA_OBJ_T) {
		    printDataCollEnt (&collEnt, queryFlags);
		    std::cout << "collEnt.objType:" << collEnt.objType << std::endl;
		    std::cout << "collEnt.dataName:" << collEnt.dataName << std::endl;
		    if (rodsArgs->accessControl == True) {
			printDataAcl (conn, collEnt.dataId);
		    }
		} else {
		    std::cout << "collEnt.objType:" << collEnt.objType << std::endl;
		    std::cout << "collEnt.collName:" << collEnt.collName << std::endl;
		    printCollCollEnt (&collEnt, queryFlags);
		    /* need to drill down */
	            if (rodsArgs->recursive == True &&
		      strcmp (collEnt.collName, "/") != 0) {
	                rodsPath_t tmpPath;
	                memset (&tmpPath, 0, sizeof (tmpPath));
	                rstrcpy (tmpPath.outPath, collEnt.collName, MAX_NAME_LEN);
	                status = lsCollUtil_sg (conn, &tmpPath, myRodsEnv, rodsArgs);
		        if (status < 0) savedStatus = status;
		    }
		}
	    }
	    rclCloseCollection (&collHandle);
	    if (savedStatus < 0 && savedStatus != CAT_NO_ROWS_FOUND) {
	        return (savedStatus);
	    } else {
	        return (0);
	    }
	}


	}//namespace api
}//namespace irods_file_adaptor

//void
//usage () {
//   char *msgs[]={
//"Usage : ils [-ArlLv] dataObj|collection ... ",
//"Display data Objects and collections stored in irods.",
//"Options are:",
//" -A  ACL (access control list) and inheritance format",
//" -l  long format",
//" -L  very long format",
//" -r  recursive - show subcollections",
//" -v  verbose",
//" -V  Very verbose",
//" -h  this help",
//""};
//   int i;
//   for (i=0;;i++) {
//      if (strlen(msgs[i])==0) break;
//      printf("%s\n",msgs[i]);
//   }
//   printReleaseInfo("ils");
//}
