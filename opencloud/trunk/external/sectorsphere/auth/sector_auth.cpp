//  Copyright (c) 2005-2007 Saurabh Sehgal (saurabh.r.s@gmail.com)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)



#include "sector_auth.hpp"
#include <saga/saga/object.hpp>
#include <saga/impl/exception.hpp>

namespace saga_sectorsphere
{
  authenticator::authenticator(void)
    : user_name(""),
      password(""),
      server_address(""),
      server_port(-1), 
      client_conf(""), 
      connected(false)
  {
  }


  authenticator::~authenticator()
  {
     //logout() ; 
  }

  authenticator::authenticator( std::string client_conf )
  {
  }

  authenticator::authenticator (std::string username, std::string passwd, std::string server_address, int port )
    : user_name( username ),
      password( passwd ), 
      server_address( server_address), 
      server_port( port ) 
  {
  }

  void authenticator::clear (void)
  {
  }

  void authenticator::set_user    (std::string user) 
  {
     user_name = user ; 
  }

  void authenticator::set_passwd  (std::string passwd ) 
  {
     password = passwd ; 
  }

  void authenticator::set_server  (std::string server ) 
  {
      server_address = server ; 
  }
  
  void authenticator:: set_port   ( int port  ) 
  {
     server_port = port;
  }


  void authenticator:: set_sec_cert   (std::string cert )
  {
     sec_cert = cert ;  
  }

  void authenticator::set_client_conf    (std::string conf ) 
  {
     client_conf = conf  ; 
  }

  void authenticator::get_client_conf    (std::string &conf ) 
  {
     conf = client_conf ; 
  }

  int authenticator::authenticate (void)
  {
     /* Check if we have all the information required to authenticate
      */
     if( !(server_address.empty() || 
           server_port == -1 ||
           user_name.empty() ||
           password.empty() ||
           sec_cert.empty() ) )
    {
       /* Great ! We have everything .. no need to do proceed further
        */
       return do_auth() ; 
    }

    /* We are missing some things .. 
     * The next strategy would be to see if a client conf file path is supplied ..
     * The adaptor has already checked the ini file for information
     */

    if( client_conf.empty() )  return error::SAGA_SECSP_E_NO_CONF_FILE ; 

    Session s ; 
    s.loadInfo( client_conf ) ; 
    
    if( (server_address.empty()) )
    {
        if( !(s.m_ClientConf.m_strMasterIP.empty() ))
        {
           server_address = s.m_ClientConf.m_strMasterIP ; 
        }
        else
        {
           return error::SAGA_SECSP_E_NO_SERVER ; 
        }
    }
    

    if( (server_port < 0 ) )
    {
        if(  !(s.m_ClientConf.m_iMasterPort < 0 ) )
        {
          server_port = s.m_ClientConf.m_iMasterPort ; 
        }
        else
        {
           return error::SAGA_SECSP_E_NO_PORT ; 
        }
    }

    if( (user_name.empty()) )
    {
        if (!(s.m_ClientConf.m_strUserName.empty()) )
        {
           user_name = s.m_ClientConf.m_strUserName ; 
        }
        else
        {
          return error::SAGA_SECSP_E_NO_USERNAME ; 
        }
    }

    if( (password.empty() ))
    {
       if ( !(s.m_ClientConf.m_strPassword.empty() ))
       {
          password = s.m_ClientConf.m_strPassword ; 
       }
       else
       {
          return error::SAGA_SECSP_E_NO_PASSWORD ; 
       }
    }
    

    if( (sec_cert.empty() ))
    {
       if ( !(s.m_ClientConf.m_strCertificate.empty() ))
       {
           sec_cert = s.m_ClientConf.m_strCertificate ; 
       }
       else
       {
          return error::SAGA_SECSP_E_NO_SECURITY_CERT ; 
       }
    }
    

    return ( do_auth()) ; 

  }

  int authenticator::do_auth(void)
  {

     int err = error::SAGA_SECSP_E_SUCCESS ; 

     if( server_address.empty() || 
         server_port == -1 ||
         user_name.empty() ||
         password.empty() ||
         sec_cert.empty() )
     {
        return error::SAGA_SECSP_E_BAD_ARGS ; 
     }


     const char * sec = sec_cert.c_str() ; 

     /* Connect to the server
      */
     if( err = Sector::init ( server_address, server_port ) < 0 )
     {
        return err ; 
     }
     
     /* Login with the username/password 
      */
     if( err = Sector::login( user_name, password, sec ) < 0 )
     {
         return err ; 
     }

     connected = true ; 
     return err ; 
  }


  void authenticator::logout (void)
  {
     if( connected )
     {
        Sector::logout();
        Sector::close();
     }
  }

  void authenticator::get_error( std::string &err , int const &code )
  {
     error::get_err_msg( err, code ) ; 
  }

}
