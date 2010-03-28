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
static char rcsid[] = "$RCSfile: align.c,v $ $Revision: 1.3 $ $Date: 2003/10/28 05:33:36 $";
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAS_LONGLONG
typedef long long int longlongint;
#endif /* HAS_LONGLONG */
#ifdef HAS_LONGDOUBLE
typedef long double longdouble;
#endif /* HAS_LONGDOUBLE */
#ifndef CHECK_TYPE
#define CHECK_TYPE	double
#endif

typedef struct {
    char a0;
    CHECK_TYPE c;
} checkStruct;


static int
maxAlign(addr)
     char *addr;
{
    unsigned long int val = (unsigned long int)addr;
    int ret = 1;

    if (val % sizeof(char) == 0) {
	ret = sizeof(char);
    }
    if (val % sizeof(short) == 0) {
	ret = sizeof(short);
    }
    if (val % sizeof(int) == 0) {
	ret = sizeof(int);
    }
    if (val % sizeof(long) == 0) {
	ret = sizeof(long);
    }
#ifdef HAS_LONGLONG
    if (val % sizeof(longlongint) == 0) {
	ret = sizeof(longlongint);
    }
#endif /* HAS_LONGLONG */
    if (val % sizeof(float) == 0) {
	ret = sizeof(float);
    }
    if (val % sizeof(double) == 0) {
	ret = sizeof(double);
    }
#ifdef HAS_LONGDOUBLE
    if (val % sizeof(longdouble) == 0) {
	ret = sizeof(longdouble);
    }
#endif /* HAS_LONGDOUBLE */

    return ret;
}


int
main(argc, argv)
     int argc;
     char *argv[];
{
    checkStruct *s;
    int align = -1;
    int mxAlign;

    s = (checkStruct *)malloc(sizeof(checkStruct));

    mxAlign = maxAlign((char *)&(s->c));
    if (mxAlign == sizeof(CHECK_TYPE)) {
	align = sizeof(CHECK_TYPE);
    } else if (mxAlign > sizeof(CHECK_TYPE)) {
	int offset = (char *)&(s->c) - (char *)s;
	if (offset <= sizeof(CHECK_TYPE)) {
	    align = offset;
	} else {
	    align = sizeof(CHECK_TYPE);
	}
    } else {
	align = mxAlign;
    }

    printf("%d\n", align);
    return 0;
}
