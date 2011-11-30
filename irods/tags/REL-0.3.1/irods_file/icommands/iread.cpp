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
 * iread.cpp
 *
 *  Created on: Feb 23, 2010
 *      Author: kawai
 */
#include "irods_icommands.hpp"

namespace irods_file_adaptor {
	namespace api {
	int icommands::iread (char *path, char *buf, size_t size, off_t offset, int seek_mode)
	{
		// Connection
		int status;
		rodsEnv myEnv;
		rErrMsg_t errMsg;
		rcComm_t *conn;
		rodsArguments_t myRodsArgs;
		int reconnFlag;
		myRodsArgs.reconnect = True;

		status = getRodsEnv (&myEnv);

		if (status < 0) {
			rodsLogError(LOG_ERROR, status, "main: getRodsEnv error. ");
			std::cout << "connection error 1" << std::endl;
			exit (1);
		}

		if (myRodsArgs.reconnect == True) {
			reconnFlag = RECONN_TIMEOUT;
		} else {
			reconnFlag = NO_RECONN;
		}

//		int cnt=0;
//		do{
//			conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
//			  myEnv.rodsZone, reconnFlag, &errMsg);
//
//			if (conn == NULL) {
//				std::cout << "connection error 2" << std::endl;
//				std::cout << "sleeping..." << std::endl;
//				sleep(1);
//			}
//
//			cnt++;
//			if (cnt>10) {
//				exit (2);
//			}
//		}while(conn == NULL);

		conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
		  myEnv.rodsZone, reconnFlag, &errMsg);

		if (conn == NULL) {
			std::cout << "connection error 2" << std::endl;
			exit(2);
		}

		if (strcmp (myEnv.rodsUserName, PUBLIC_USER_NAME) != 0) {
			status = clientLogin(conn);
			if (status != 0) {
				rcDisconnect(conn);
				std::cout << "connection error 7" << std::endl;
				exit (7);
			}
		}


		// Open file
		int l1descInx2;
		dataObjInp_t dataObjOpenInp;
	    memset (&dataObjOpenInp, 0, sizeof (dataObjOpenInp));

	    strcpy(dataObjOpenInp.objPath, path);
//	    std::cout << "dataObjOpenInp.objPath=" << dataObjOpenInp.objPath << std::endl;
	    dataObjOpenInp.openFlags = O_RDONLY;

	    l1descInx2 = rcDataObjOpen (conn, &dataObjOpenInp);


		// Seek file
        openedDataObjInp_t dataObjLseekInp;
        fileLseekOut_t *dataObjLseekOut = NULL;

        bzero (&dataObjLseekInp, sizeof (dataObjLseekInp));
        dataObjLseekInp.l1descInx = l1descInx2;
        dataObjLseekInp.offset = offset;
        dataObjLseekInp.whence = seek_mode;

//		std::cout << "inside of seek()" << std::endl;
        status = rcDataObjLseek (conn, &dataObjLseekInp, &dataObjLseekOut);
        if (dataObjLseekOut != NULL) free (dataObjLseekOut);


	    // Read file
		openedDataObjInp_t dataObjReadInp;
		bytesBuf_t dataObjReadOutBBuf;

		bzero (&dataObjReadInp, sizeof (dataObjReadInp));
		dataObjReadOutBBuf.buf = buf;
		dataObjReadOutBBuf.len = size;
		dataObjReadInp.l1descInx = l1descInx2;
		dataObjReadInp.len = size;

//		std::cout << "inside of ifuseread()" << std::endl;
		status = rcDataObjRead (conn, &dataObjReadInp, &dataObjReadOutBBuf);


	    /* close the files */
		openedDataObjInp_t dataObjCloseInp;
	    memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
	    dataObjCloseInp.l1descInx = l1descInx2;

	    status = rcDataObjClose (conn, &dataObjCloseInp);

	    return status;
	}
}
}
