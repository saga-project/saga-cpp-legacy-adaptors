#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: dadd.c,v $ $Revision: 1.5 $ $Date: 2004/03/11 08:15:27 $";
#endif /* NG_OS_IRIX */
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

void dadd(int n, double *A, double *B, double *C);

#include <unistd.h>
void
dadd(int n, double *A, double *B, double *C)
{
    int i, j;

    sleep(10);
    for (i = 0; i < n; i++) {
         for (j = 0; j < n; j++) {
             *(C + (i * n + j)) = *(A + (i * n + j)) + *(B + (i * n + j));
         }
    }
}
