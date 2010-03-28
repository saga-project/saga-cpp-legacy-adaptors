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
static char rcsid[] = "$RCSfile: chkvoidp.c,v $ $Revision: 1.3 $ $Date: 2003/10/28 05:33:36 $";
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "confdefs.h"


int
main(argc, argv)
     int argc;
     char *argv[];
{
    char *s = "unknown";
    if (SIZEOF_VOID_P == SIZEOF_UNSIGNED_SHORT) {
	s = "short";
    } else if (SIZEOF_VOID_P == SIZEOF_UNSIGNED_INT) {
	s = "unsigned int";
#ifdef HAS_LONGLONG
    } else if (SIZEOF_VOID_P == SIZEOF_UNSIGNED_LONG_LONG) {
	s = "unsigned long long";
#endif /* HAS_LONGLONG */
    } else if (SIZEOF_VOID_P == SIZEOF_UNSIGNED_LONG) {
	s = "unsigned long";
    }

    printf("%s\n", s);
    return 0;
}
