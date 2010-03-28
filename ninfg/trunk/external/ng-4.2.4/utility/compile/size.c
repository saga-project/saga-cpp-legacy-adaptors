/* 
 * $AIST_Release: 4.2.4 $
 * $AIST_Copyright:
 *  Copyright 2003, 2004, 2005, 2006 Grid Technology Research Center,
 *  National Institute of Advanced Industrial Science and Technology
 *  Copyright 2003, 2004, 2005, 2006 National Institute of Informatics
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *  $
 */
#if 0
static char rcsid[] = "$RCSfile: size.c,v $ $Revision: 1.3 $ $Date: 2003/10/28 05:33:36 $";
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *
chkIntSize(sz)
     int sz;
{
    if (sizeof(char) == sz) {
	return "char";
    } else if (sizeof(int) == sz) {
	return "int";
    } else if (sizeof(short) == sz) {
	return "short";
#ifdef HAS_LONGLONG
    } else if (sizeof(long long int) == sz) {
	return "long long int";
#endif
    } else if (sizeof(long int) == sz) {
	return "long int";
    } else {
	return "unknown";
    }
}


int
main(argc, argv)
     int argc;
     char *argv[];
{
    int sz;

    if (argc < 2) {
	return 1;
    }
    sz = atoi(argv[1]) / 8;

    printf("%s\n", chkIntSize(sz));
    return 0;
}
