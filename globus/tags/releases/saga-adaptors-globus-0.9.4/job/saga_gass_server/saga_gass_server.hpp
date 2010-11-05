//  Copyright (c) 2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef SAGA_GASS_SERVER_HPP
#define SAGA_GASS_SERVER_HPP

extern "C" {
    #include <gssapi.h>
    #include <globus_gram_client.h>
    #include "./globus_gram_job_adaptor_gass_server_ez.h"
}

#include <string> 

namespace adaptors {
  namespace globus_preWS { 
    namespace common {
                
      class saga_gass_server 
      {

      private:
        
        globus_gass_transfer_listener_t Listener_;
        
        int stdout_fd_ ; 
        //int server_port_; // we don't use that.
        std::string server_url_;
        
        bool is_running;

      public:
              
        /**
          * DESCRIPTION
          */
        saga_gass_server ();
          
        /**
          * DESCRIPTION
          */
        ~saga_gass_server ();
          
        /**
          * DESCRIPTION
          */
        std::string start (int pipe[2]/*, int port=0*/);
          
        /**
          * DESCRIPTION
          */
        void stop ();
        
        /**
          * DESCRIPTION
          */
        std::string get_url () { return server_url_; };
      };
    
    } // common
  } // globus_preWS
} // adaptors

#endif //SAGA_GASS_SERVER_HPP

