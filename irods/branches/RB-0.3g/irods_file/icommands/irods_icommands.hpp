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
 * irods_icommands.hpp
 *
 *  Created on: Feb 8, 2010
 *      Author: kawai
 */

#ifndef IRODS_ICOMMANDS_HPP_
#define IRODS_ICOMMANDS_HPP_

#include <iostream>
#include <vector>
#include <fcntl.h>
#include "rods.h"
#include "rodsClient.h"
#include "parseCommandLine.h"
#include "rodsPath.h"
#include "lsUtil.h"
#include "miscUtil.h"
#include "rcMisc.h"
#include "genQuery.h"
#include "cpUtil.h"
#include "getUtil.h"
#include "mkdirUtil.h"
#include "mvUtil.h"
#include "putUtil.h"
#include "rmUtil.h"
#include <boost/filesystem/path.hpp>

typedef struct {
  std::string collName;
  std::string dataName;
  std::string rescLoc;
  std::string dataPath;
  std::string metaDataAttr;
  std::string metaDataVal;
  std::string metaCollAttr;
  std::string metaCollVal;
//  objType_t objType;
//  int replNum;
//  int replStatus;
//  uint dataMode;
  unsigned long dataSize;
//  char *collName;		/* valid for dataObj and collection */
//  char *dataName;
//  char *dataId;
//  char *createTime;
//  char *modifyTime;
//  char *chksum;
//  char *resource;
//  char *rescGrp;
//  char *phyPath;
//  char *ownerName;    	 /* valid for dataObj and collection */
//  specColl_t specColl;	 /* valid only for collection */
} irdEnt_t;

////////////////////////////////////////////////////////////////////////////////
//
namespace irods_file_adaptor {
	namespace api {

	  class icommands
	  {
		private:

		public:

		  /*! constructor  */
			icommands (){}

		  /*! destructor  */
		  ~icommands (){}

		  int ils (int argc, char **argv);
		  int lsUtil_sg (rcComm_t *conn, rodsEnv *myRodsEnv, rodsArguments_t *myRodsArgs,	rodsPathInp_t *rodsPathInp);
		  int lsDataObjUtil_sg (rcComm_t *conn, rodsPath_t *srcPath, rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, genQueryInp_t *genQueryInp);
		  int lsCollUtil_sg (rcComm_t *conn, rodsPath_t *srcPath, rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs);

		  int icd(int argc, char **argv);
		  int icp (int argc, char **argv);

		  int imkdir(int argc, char **argv);
		  int imv(int argc, char **argv);
		  int ipwd(int argc, char **argv);
		  int irm(int argc, char **argv);

//		  std::vector <irdEnt_t*> iquest(int argc, char **argv);
//		  std::vector <irdEnt_t*> queryAndShowStrCond(rcComm_t *conn, char *hint, char *format, char *selectConditionString);
//		  std::vector <irdEnt_t*> printGenQueryOut_sg(FILE *fd, char *format, char *hint, genQueryOut_t *genQueryOut);
		  std::vector <irdEnt_t> iquest(int argc, char **argv);
		  std::vector <irdEnt_t> queryAndShowStrCond(rcComm_t *conn, char *hint, char *format, char *selectConditionString);
		  std::vector <irdEnt_t> printGenQueryOut_sg(FILE *fd, char *format, char *hint, genQueryOut_t *genQueryOut);

		  int iget(int argc, char **argv) ;
		  int iread (char *path, char *buf, size_t size, off_t offset, int seek_mode);

		  int iput(int argc, char **argv);
		  int iwrite (char *path, char *buf, size_t size, off_t offset, int seek_mode);

		  //// imeta functions
		  int imeta(int argc, char **argv);
		  void printGenQueryResults(rcComm_t *Conn, int status, genQueryOut_t *genQueryOut, char *descriptions[]);
		  int showDataObj(rcComm_t *Conn, char *name, char *attrName, int wild);
		  int showColl(rcComm_t *Conn, char *name, char *attrName, int wild);
		  int showResc(rcComm_t *Conn, char *name, char *attrName, int wild);
		  int showRescGroup(rcComm_t *Conn, char *name, char *attrName, int wild);
		  int showUser(rcComm_t *Conn, rodsEnv myEnv, char *name, char *attrName, int wild);
		  int queryDataObj(rcComm_t *Conn, char *cmdToken[]);
		  int queryCollection(rcComm_t *Conn, char *attribute, char *op, char *value);
		  int queryResc(rcComm_t *Conn, char *attribute, char *op, char *value);
		  int queryRescGroup(rcComm_t *Conn, char *attribute, char *op, char *value);
		  int queryUser(rcComm_t *Conn, rodsEnv myEnv, char *attribute, char *op, char *value);
		  int modCopyAVUMetadata(rcComm_t *Conn, rodsEnv myEnv, char *arg0, char *arg1, char *arg2, char *arg3, char *arg4, char *arg5, char *arg6, char *arg7);
		  int modAVUMetadata(rcComm_t *Conn, char *arg0, char *arg1, char *arg2, char *arg3, char *arg4, char *arg5, char *arg6, char *arg7, char *arg8);
		  void getInput(rcComm_t *Conn, char *cmdToken[], int maxTokens);
		  int doCommand(rcComm_t *Conn, rodsEnv myEnv, char *cmdToken[]);

		  void test(void){
			  printf("icommand::test test\n");
		  }
	  };

	} // namespace api

} // namespace irods_file_adaptor


#endif /* IRODS_ICOMMANDS_HPP_ */

