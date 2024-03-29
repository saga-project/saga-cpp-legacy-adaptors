/*
 *    Copyright (c) <2002-2006> <Jean-Philippe Barrette-LaPierre>
 *    
 *    Permission is hereby granted, free of charge, to any person obtaining
 *    a copy of this software and associated documentation files 
 *    (cURLpp), to deal in the Software without restriction, 
 *    including without limitation the rights to use, copy, modify, merge,
 *    publish, distribute, sublicense, and/or sell copies of the Software,
 *    and to permit persons to whom the Software is furnished to do so, 
 *    subject to the following conditions:
 *    
 *    The above copyright notice and this permission notice shall be included
 *    in all copies or substantial portions of the Software.
 *    
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 *    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
 *    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
 *    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <iostream>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Multi.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

int main(int argc, char *argv[])
{
  if(argc < 3) {
    std::cerr << "Example 13: Wrong number of arguments" << std::endl 
	      << "Example 13: Usage: example13 url1 url2" 
	      << std::endl;
    return EXIT_FAILURE;
  }
  
  char *url1 = argv[1];
  char *url2 = argv[2];
  
  try {
    cURLpp::Cleanup cleaner;
    
    cURLpp::Easy request1;
    cURLpp::Easy request2;
    
    request1.setOpt(new cURLpp::Options::Url(url1)); 
    request1.setOpt(new cURLpp::Options::Verbose(true)); 
    
    request2.setOpt(new cURLpp::Options::Url(url2)); 
    request2.setOpt(new cURLpp::Options::Verbose(true)); 
    
    int nbLeft;
    cURLpp::Multi requests;
    requests.add(&request1);
    requests.add(&request2);
    
    /* we start some action by calling perform right away */
    while(!requests.perform(&nbLeft));
    
    while(nbLeft) {
      struct timeval timeout;
      int rc; /* select() return code */
      
      fd_set fdread;
      fd_set fdwrite;
      fd_set fdexcep;
      int maxfd;
      
      FD_ZERO(&fdread);
      FD_ZERO(&fdwrite);
      FD_ZERO(&fdexcep);
      
      /* set a suitable timeout to play around with */
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;
      
      /* get file descriptors from the transfers */
      requests.fdset(&fdread, &fdwrite, &fdexcep, &maxfd);
      
      rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
      
      switch(rc) {
      case -1:
	/* select error */
	nbLeft = 0;
	printf("select() returns error, this is badness\n");
	break;
      case 0:
      default:
	/* timeout or readable/writable sockets */
	while(!requests.perform(&nbLeft));
	break;
      }
    }
    
    std::cout << "NB lefts: " << nbLeft << std::endl;
  }
  catch ( cURLpp::LogicError & e ) {
    std::cout << e.what() << std::endl;
  }
  catch ( cURLpp::RuntimeError & e ) {
    std::cout << e.what() << std::endl;
  }

  return EXIT_SUCCESS;
}

  
