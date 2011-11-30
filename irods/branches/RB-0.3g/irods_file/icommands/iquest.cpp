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

//#include "rodsClient.h"
//#include "parseCommandLine.h"
//#include "rodsPath.h"
//#include "rcMisc.h"
//#include "lsUtil.h"
//void usage ();
//
//
//void
//usage () {
//   char *msgs[]={
//"Usage : iquest [ [hint] format]  selectConditionString ",
//"format is C format restricted to character strings.",
//"selectConditionString is of the form: SELECT <attribute> [, <attribute>]* [WHERE <condition> [ AND <condition>]*]",
//"attribute can be found using 'iquest attrs' command",
//"condition is of the form: <attribute> <rel-op> <value>",
//"rel-op is a relational operator: eg. =, <>, >,<, like, not like, between, etc.,",
//"value is either a constant or a wild-carded expression.",
//"One can also use a few aggregation operators such as sum,count,min,max and avg.",
//"Use % and _ as wild-cards, and use \\ to escape them",
//"Options are:",
//" -h  this help",
//"Examples:\n",
//" iquest \"SELECT DATA_NAME, DATA_CHECKSUM WHERE DATA_RESC_NAME like 'demo%'\"",
//" iquest \"For %-12.12s size is %s\" \"SELECT DATA_NAME ,  DATA_SIZE  WHERE COLL_NAME = '/tempZone/home/rods'\"",
//" iquest \"SELECT COLL_NAME WHERE COLL_NAME like '/tempZone/home/%'\"",
//" iquest \"User %-6.6s has %-5.5s access to file %s\" \"SELECT USER_NAME,  DATA_ACCESS_NAME, DATA_NAME WHERE COLL_NAME = '/tempZone/home/rods'\"",
//" iquest \" %-5.5s access has been given to user %-6.6s for the file %s\" \"SELECT DATA_ACCESS_NAME, USER_NAME, DATA_NAME WHERE COLL_NAME = '/tempZone/home/rods'\"",
//" iquest \"SELECT RESC_NAME, RESC_LOC, RESC_VAULT_PATH, DATA_PATH WHERE DATA_NAME = 't2' AND COLL_NAME = '/tempZone/home/rods'\"",
//" iquest \"User %-9.9s uses %14.14s bytes in %8.8s files in '%s'\" \"SELECT USER_NAME, sum(DATA_SIZE),count(DATA_NAME),RESC_NAME\"",
//" iquest \"select sum(DATA_SIZE) where COLL_NAME = '/tempZone/home/rods'\"",
//" iquest \"select sum(DATA_SIZE) where COLL_NAME like '/tempZone/home/rods%'\"",
//" iquest \"select sum(DATA_SIZE), RESC_NAME where COLL_NAME like '/tempZone/home/rods%'\"",
//""};
//   int i;
//   for (i=0;;i++) {
//      if (strlen(msgs[i])==0) break;
//      printf("%s\n",msgs[i]);
//   }
//   printReleaseInfo("iquest");
//}

#include "irods_icommands.hpp"

namespace irods_file_adaptor {
	namespace api {
	std::vector <irdEnt_t> icommands::iquest(int argc, char **argv) {
		int status;
		rodsEnv myEnv;
		rErrMsg_t errMsg;
		rcComm_t *conn;
		rodsArguments_t myRodsArgs;
		char *optStr;
		std::vector <irdEnt_t> results;

		optStr = "h";

		status = parseCmdLineOpt (argc, argv, optStr, 0, &myRodsArgs);


		if (status < 0) {
			printf("Use -h for help\n");
//			exit (1);
		}
		if (myRodsArgs.help==True) {
//		   usage();
//		   exit(0);
		}
//		if (myRodsArgs.optind == argc) {
////		  printf("StringCondition needed. myRodsArgs.optind=%d\n", myRodsArgs.optind);
//		  myRodsArgs.optind = 1;
////		  printf("forced change. myRodsArgs.optind=%d\n", myRodsArgs.optind);
////		  usage();
////		  exit(0);
//		}

		status = getRodsEnv (&myEnv);

		if (status < 0) {
			rodsLogError (LOG_ERROR, status, "main: getRodsEnv error. ");
//			exit (1);
		}

		if (myRodsArgs.optind == 1) {
		   if (!strncmp(argv[argc-1], "attrs", 5)) {
		  showAttrNames();
//		  exit(0);
		   }
		}

		conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
		  myEnv.rodsZone, 0, &errMsg);

		if (conn == NULL) {
			exit (2);
		}

		status = clientLogin(conn);
		if (status != 0) {
		   exit (3);
		}

		results = queryAndShowStrCond(conn, NULL, NULL, argv[1]);

		rcDisconnect(conn);

		return results;

	}

	std::vector <irdEnt_t> icommands::queryAndShowStrCond(rcComm_t *conn, char *hint,
			char *format, char *selectConditionString)
	{
	  genQueryInp_t genQueryInp;
	  int i;
	  genQueryOut_t *genQueryOut = NULL;
	  std::vector <irdEnt_t> results;

	  memset (&genQueryInp, 0, sizeof (genQueryInp_t));
	  i = fillGenQueryInpFromStrCond(selectConditionString, &genQueryInp);
	  if (i < 0){
//		return(i);
		  return results;
	  }

	  genQueryInp.maxRows= MAX_SQL_ROWS;
	  genQueryInp.continueInx=0;
	  i = rcGenQuery (conn, &genQueryInp, &genQueryOut);

	  if (i < 0){
//		return(i);
		  return results;
	  }

	  results = printGenQueryOut_sg(stdout, format,hint,  genQueryOut);
//	  if (i < 0)
//		return(i);

	  return results;

	}

	std::vector <irdEnt_t> icommands::printGenQueryOut_sg(FILE *fd, char *format, char *hint, genQueryOut_t *genQueryOut)
	{
	  int i,n,j;
	  sqlResult_t *v[MAX_SQL_ATTR];
	  char * cname[MAX_SQL_ATTR];
	  std::vector <irdEnt_t> irdEnt;
	  irdEnt_t* irdEnt_tmp;
	  n = genQueryOut->attriCnt;

	  for (i = 0; i < n; i++) {
	    v[i] = &genQueryOut->sqlResult[i];
	    cname[i] = getAttrNameFromAttrId(v[i]->attriInx);
	    if (cname[i] == NULL){
//	      return(NO_COLUMN_NAME_FOUND);
	      std::cout << "NO_COLUMN_NAME_FOUND" << std::endl;

//	      return results;
	      return irdEnt;
	    }
	  }

	  for (i = 0;i < genQueryOut->rowCnt; i++) {
		  irdEnt_tmp = new irdEnt_t();

	    if (format == NULL || strlen(format) == 0) {
	      for (j = 0; j < n; j++) {

	    	  if(strcmp(cname[j], "DATA_NAME") == 0){
	    		  irdEnt_tmp->dataName = &v[j]->value[v[j]->len * i];
	    	  }
	    	  else if(strcmp(cname[j], "COLL_NAME") == 0){
	    		  irdEnt_tmp->collName = &v[j]->value[v[j]->len * i];
	    	  }
	    	  else if(strcmp(cname[j], "DATA_SIZE") == 0){
	    		  unsigned long d_size = atol(&v[j]->value[v[j]->len * i]);
	    		  irdEnt_tmp->dataSize = d_size;
	    	  }
	    	  else if(strcmp(cname[j], "RESC_LOC") == 0){
	    		  irdEnt_tmp->rescLoc = &v[j]->value[v[j]->len * i];
	    	  }
	    	  else if(strcmp(cname[j], "DATA_PATH") == 0){
	    		  irdEnt_tmp->dataPath = &v[j]->value[v[j]->len * i];
	    	  }
	    	  else if(strcmp(cname[j], "META_DATA_ATTR_NAME") == 0){
	    		  irdEnt_tmp->metaDataAttr = &v[j]->value[v[j]->len * i];
	    	  }
	    	  else if(strcmp(cname[j], "META_DATA_ATTR_VALUE") == 0){
	    		  irdEnt_tmp->metaDataVal = &v[j]->value[v[j]->len * i];
	    	  }
	    	  else if(strcmp(cname[j], "META_COLL_ATTR_NAME") == 0){
	    		  irdEnt_tmp->metaCollAttr = &v[j]->value[v[j]->len * i];
	    	  }
	    	  else if(strcmp(cname[j], "META_COLL_ATTR_VALUE") == 0){
	    		  irdEnt_tmp->metaCollVal = &v[j]->value[v[j]->len * i];
	    	  }

//	    	  std::cout << "cname[" << j << "]=" << cname[j] << ":" << &v[j]->value[v[j]->len * i] << std::endl;

	      }

		  irdEnt_t irdEnt_buf;
	      irdEnt_buf.collName = irdEnt_tmp->collName;
	      irdEnt_buf.dataName = irdEnt_tmp->dataName;
	      irdEnt_buf.dataSize = irdEnt_tmp->dataSize;
	      irdEnt_buf.rescLoc  = irdEnt_tmp->rescLoc;
	      irdEnt_buf.dataPath = irdEnt_tmp->dataPath;
	      irdEnt_buf.metaDataAttr = irdEnt_tmp->metaDataAttr;
	      irdEnt_buf.metaDataVal = irdEnt_tmp->metaDataVal;
	      irdEnt_buf.metaCollAttr = irdEnt_tmp->metaCollAttr;
	      irdEnt_buf.metaCollVal = irdEnt_tmp->metaCollVal;

	      irdEnt.push_back(irdEnt_buf);
	    }
	  }

//	  return results;
	  return irdEnt;
	}

	}
}
