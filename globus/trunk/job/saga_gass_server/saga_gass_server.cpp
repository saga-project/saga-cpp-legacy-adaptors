//  Copyright (c) 2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "saga_gass_server.hpp"

#include <iostream>

using namespace adaptors::globus_preWS::common;

////////// class: gass_server /////////////////////////////////////////////////
////////// gass_server ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

saga_gass_server::saga_gass_server ()
: stdout_fd_(0), /*server_port_(0),*/ server_url_(""), is_running(false)
{
    // due to heavy leakage we load/unload the modules globally 
    //
    int rc = globus_module_activate(SAGA_GASS_SERVER_EZ_MODULE);
    if(GLOBUS_SUCCESS != rc) 
    {
      throw rc;
    }
}

////////// class: gass_server /////////////////////////////////////////////////
////////// ~gass_server ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

saga_gass_server::~saga_gass_server ()
{
    if(is_running)
    {
        stop();
    }
}

////////// class: gass_server /////////////////////////////////////////////////
////////// std::string start //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

std::string saga_gass_server::start (int pipe[2])
{
    stdout_fd_ = pipe[1];
    
    const char* scheme = "https";
    int   err          = GLOBUS_SUCCESS;
        
    globus_gass_transfer_listenerattr_t attr = GLOBUS_NULL;
    globus_gass_transfer_listenerattr_init (&attr, (char*)scheme);
    
    unsigned long server_ez_opts = 
        GLOBUS_GASS_SERVER_EZ_LINE_BUFFER
        | GLOBUS_GASS_SERVER_EZ_STDOUT_ENABLE
        | GLOBUS_GASS_SERVER_EZ_STDERR_ENABLE
        | GLOBUS_GASS_SERVER_EZ_WRITE_ENABLE
        | GLOBUS_GASS_SERVER_EZ_READ_ENABLE
        | GLOBUS_GASS_SERVER_EZ_CLIENT_SHUTDOWN_ENABLE;
    
    err = globus_gass_server_ez_init (&Listener_, 
                                      &attr, 
                                      (char*)scheme,
                                      NULL, 
                                      server_ez_opts, 
                                      NULL,
                                      stdout_fd_);
        
    if ( err != GLOBUS_SUCCESS )
    {
        throw err;
    }
    else
    {   
        is_running = true;
        server_url_ = std::string (globus_gass_transfer_listener_get_base_url 
                                   (Listener_));
    }
    
    return server_url_;
}

////////// class: gass_server /////////////////////////////////////////////////
////////// void stop //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void saga_gass_server::stop ()
{
    if(is_running)
    {        
        int err = globus_gass_server_ez_shutdown (Listener_);
        if ( err != GLOBUS_SUCCESS )
            throw err;
        else
            is_running = false;
    }
}
