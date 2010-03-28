#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: base64encode.c,v $ $Revision: 1.4 $ $Date: 2004/03/11 07:29:52 $";
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
#include <stdio.h>

char template[64];

void init_template(){
  int i = 0;
  for (i = 0; i < 26; i++){
    template[i] = 'A' + i;
    template[i+26] = 'a' + i;    
  }
  for (i = 0; i < 10; i++){
    template[i+52] = '0' + i;
  }
  template[62]='+'; 
  template[63]='/'; 
}

#define CASE0 0
#define CASE1 1
#define CASE2 2

int main(int argc, char ** argv){
  int c1, c2, c3;
  int i;
  int b1, b2, b3, b4;
  int counter = 0;
  int flag = CASE0;
  init_template();

  if (argc > 1)
    printf("%s:: ", argv[1]);

  while (1){
    if ((c1 = getchar()) == EOF)
      break;
    if ((c2 = getchar()) == EOF) {
      c2 = 0;
      c3 = 0;
      flag = CASE1;
    } else {
      if ((c3 = getchar()) == EOF){
	c3 = 0;
	flag = CASE2;
      }
    }
    i = ((c1 & 0xFF) << 16) | ((c2 & 0xFF) << 8) | ((c3 & 0xFF));
    b1 = (i >>18)& 0x3F;
    b2 = (i >>12)& 0x3F;
    b3 = (i >> 6)& 0x3F;
    b4 = (i >> 0)& 0x3F;

    putchar(template[b1]); 
    putchar(template[b2]); 
    if (flag == CASE0){
      putchar(template[b3]); 
      putchar(template[b4]); 
    } else if (flag == CASE1){
      putchar('=');
      putchar('=');
      break;
    } else if (flag == CASE2){
      putchar(template[b3]); 
      putchar('=');
      break;
    }
    if (++counter % 18 == 0){
      putchar('\n');
      putchar(' ');
    }
  }
  putchar('\n');
  return 0;

}
