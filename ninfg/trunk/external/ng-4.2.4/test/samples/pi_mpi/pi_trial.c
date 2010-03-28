static const char rcsid[] = "$RCSfile: pi_trial.c,v $ $Revision: 1.1 $ $Date: 2004/11/05 06:14:44 $";
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

#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__
#define RANDOM_MAX RAND_MAX
#else
#define RANDOM_MAX 2147483647 /* 2**31 - 1 */
#endif

long pi_trial(int seed, long times)
{
    long l, counter = 0;

    srandom(seed);

    for (l = 0; l < times; l++) {
        double x = (double)random() / RANDOM_MAX;
        double y = (double)random() / RANDOM_MAX;

        if (x * x + y * y < 1.0) {
            counter++;
        }
     }

     printf("counter = %ld, times = %ld\n", counter, times);
     return counter;
}
