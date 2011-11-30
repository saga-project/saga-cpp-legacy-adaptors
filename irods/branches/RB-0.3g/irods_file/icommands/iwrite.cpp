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
 * iwrite.cpp
 *
 *  Created on: Feb 23, 2010
 *      Author: kawai
 */
#include "irods_icommands.hpp"

namespace irods_file_adaptor {
	namespace api {
	int icommands::iwrite (char *path, char *buf, size_t size, off_t offset, int seek_mode)
	{
		// Connection
		int status;
		rodsEnv myEnv;
		rErrMsg_t errMsg;
		rcComm_t *conn;
		rodsArguments_t myRodsArgs;
		int reconnFlag;


		status = getRodsEnv (&myEnv);

		if (status < 0) {
			rodsLogError(LOG_ERROR, status, "main: getRodsEnv error. ");
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

		if (strcmp (myEnv.rodsUserName, PUBLIC_USER_NAME) != 0) {
			status = clientLogin(conn);
			if (status != 0) {
				rcDisconnect(conn);
				exit (7);
			}
		}

		// Open file
		int l1descInx2;
		dataObjInp_t dataObjOpenInp;
	    memset (&dataObjOpenInp, 0, sizeof (dataObjOpenInp));

	    strcpy(dataObjOpenInp.objPath, path);
	    std::cout << "dataObjOpenInp.objPath=" << dataObjOpenInp.objPath << std::endl;
	    dataObjOpenInp.openFlags = O_WRONLY;

	    l1descInx2 = rcDataObjOpen (conn, &dataObjOpenInp);


		// Seek file
        openedDataObjInp_t dataObjLseekInp;
        fileLseekOut_t *dataObjLseekOut = NULL;

        bzero (&dataObjLseekInp, sizeof (dataObjLseekInp));
        dataObjLseekInp.l1descInx = l1descInx2;
        dataObjLseekInp.offset = offset;
        dataObjLseekInp.whence = seek_mode;

		std::cout << "inside of seek()" << std::endl;
        status = rcDataObjLseek (conn, &dataObjLseekInp, &dataObjLseekOut);
        if (dataObjLseekOut != NULL) free (dataObjLseekOut);
        if (status < 0) {
			rodsLogError (LOG_ERROR, status, "rcDataObjLseek: lseek of %s error", path);
			return status;
		}


		// Write
	    openedDataObjInp_t dataObjWriteInp;
	    bytesBuf_t dataObjWriteInpBBuf;

	    bzero (&dataObjWriteInp, sizeof (dataObjWriteInp));
		dataObjWriteInpBBuf.buf = (void *) buf;
		dataObjWriteInpBBuf.len = size;
		dataObjWriteInp.l1descInx = l1descInx2;
		dataObjWriteInp.len = size;

//	    std::cout << "dataObjWriteInpBBuf.buf=" << (char*)dataObjWriteInpBBuf.buf << std::endl;
		status = rcDataObjWrite (conn, &dataObjWriteInp, &dataObjWriteInpBBuf);
        if (status < 0) {
			rodsLogError (LOG_ERROR, status, "rrcDataObjWrite of %s error, wrote %d, toWrite %d", path, status, size);
			return status;
        }


	    /* close the files */
		openedDataObjInp_t dataObjCloseInp;
	    memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
	    dataObjCloseInp.l1descInx = l1descInx2;

	    status = rcDataObjClose (conn, &dataObjCloseInp);
        if (status < 0) {
			rodsLogError (LOG_ERROR, status, "rcDataObjClose: close of %s error", path);
		}

	    return status;
	}
}
}
