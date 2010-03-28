//  Copyright (c) 2006-2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "opencloud_context_adaptor.hpp"

#include <saga/saga/adaptors/config.hpp>
#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/saga/adaptors/attribute.hpp>

using namespace opencloud_context_adaptor;

#define ADAPTORS_OPENCLOUD_CONTEXT_TYPE "opencloud"


SAGA_ADAPTOR_REGISTER (context_adaptor);



///////////////////////////////////////////////////////////////////////////////
//  constructor
context_cpi_impl::context_cpi_impl (proxy                * p, 
                                    cpi_info       const & info,
                                    saga::ini::ini const & glob_ini, 
                                    saga::ini::ini const & adap_ini,
                                    TR1::shared_ptr <saga::adaptor> adaptor)
    : base_cpi (p, info, adaptor, cpi::Noflags), 
      authenticated_flag( false )
{
  saga::adaptors::attribute attr (this);

  if ( attr.attribute_exists (saga::attributes::context_type) )
  {
    if ( ADAPTORS_OPENCLOUD_CONTEXT_TYPE != 
         attr.get_attribute (saga::attributes::context_type) )
    {
      SAGA_ADAPTOR_THROW ("Can't handle context types others than 'opencloud'", 
                          saga::BadParameter);
    }
  }

  s_ = p->get_session() ; 
  a_ini = adap_ini ;  
  g_ini = glob_ini ; 


}

///////////////////////////////////////////////////////////////////////////////
//  destructor
context_cpi_impl::~context_cpi_impl (void)
{
   auth_.logout() ; 
}

///////////////////////////////////////////////////////////////////////////////
//
void context_cpi_impl::sync_set_defaults (saga::impl::void_t &)
{    
  saga::adaptors::attribute attr (this);

  if ( attr.attribute_exists (saga::attributes::context_type) )
  {
    if ( ADAPTORS_OPENCLOUD_CONTEXT_TYPE != 
         attr.get_attribute (saga::attributes::context_type) )
    {
      SAGA_ADAPTOR_THROW ("Can't handle context types others than 'opencloud'", 
                          saga::BadParameter);
    }

    if ( "" == attr.get_attribute (saga::attributes::context_type) )
    {
      SAGA_ADAPTOR_THROW ("Context Type cannot have an empty value", 
                          saga::IncorrectState);
    }


  }
 
  if( !authenticated_flag )
  {
     int rc = authenticate( s_, g_ini, a_ini ) ; 
     if( rc < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ){

      std::string error ("Authentication failed - " ) ;  
      get_authentication_error( error, rc ) ; 
      SAGA_ADAPTOR_THROW (error, saga::AuthenticationFailed ) ; 
     }
     else{
        authenticated_flag = true ; 
     }
  }

}


  int context_cpi_impl::authenticate (const saga::session &s,
                                      saga::ini::ini const & glob_ini,
                                      saga::ini::ini const & adap_ini)
  {
    
    
    std::cout << "Authenticating with Sector/Sphere Master Server ..." << std::endl ; 
    saga::adaptors::attribute c_ (this);
    std::string cf ; 

    /* All the information we need to authenticate
     */
    std::string username ;
    std::string password ; 
    std::string servername ; 
    int port ; 
    std::string sec_cert ; 
    saga::ini::entry_map entries = adap_ini.get_section("preferences").get_entries() ; 

    /* Get the authenticator
     */
    saga_sectorsphere::service serv_ ; 
    saga_sectorsphere::authenticator &auth = serv_.get_authenticator() ; 
    bool client_info_path =  check_ini_conf( cf, entries ) ; 
    if( client_info_path )
       serv_.set_sector_path( cf ) ; 

    /* Load username  
     */
    if( populate_user_name( username, c_, entries ) )
    {
       auth.set_user( username ) ; 
    }

    /* Load password
     */
    if( ( populate_password( password, c_ , entries ) ) )
    {
       auth.set_passwd( password ) ; 
    }

    /* Load server info
     */
    if( populate_server_info ( servername, c_ , entries, port ) )
    {

       auth.set_server( servername ) ; 
       auth.set_port( port ) ; 
    }

    /* Load security certificate 
     */
    if( populate_sec_cert ( sec_cert, c_ , entries ) )
    {
       auth.set_sec_cert( sec_cert ) ; 
    }

    auth_ = auth ; 
    return (auth.authenticate()) ; 

  } 

  bool context_cpi_impl::check_ini_conf( std::string &path, 
                                         saga::ini::entry_map &adap_ini)
  {
     if( adap_ini.find("client.install.path") == adap_ini.end() || 
         adap_ini["client.install.path"] == "" )
     {
        return false ; 
     }

     path = adap_ini["client.install.path"] ; 
     return true ; 
  }


  bool context_cpi_impl::check_ini_for_sec( std::string & sec, saga::ini::entry_map &adap_ini )
  {
     if( !( adap_ini.find("master.security_certificate") == adap_ini.end() || 
            adap_ini["master.security_certificate"] == "" ) )
     {
         sec = adap_ini["master.security_certificate"] ; 
         return true ; 
     }


     return false ; 

  }

  bool context_cpi_impl::check_ini_for_username( std::string & user, saga::ini::entry_map &adap_ini )  
  {
        if( !( adap_ini.find("client.username") == adap_ini.end() || 
               adap_ini["client.username"] == "" ) )
        {
           user = adap_ini["client.username"] ; 
           return true ; 
        }

        return false ; 
  }

  bool context_cpi_impl::check_ini_for_pass( std::string & pass, saga::ini::entry_map  &adap_ini ) 
  {

     if( !( adap_ini.find("client.password") == adap_ini.end() || 
             adap_ini["client.password"] == "" ) )
     {
          pass = adap_ini["client.password"] ; 
          return true ; 
     }
     return false ; 
  }


  bool context_cpi_impl::check_ini_for_server( std::string & serv, int & port, saga::ini::entry_map &adap_ini ) 
  {
      if( !( adap_ini.find("master.server") == adap_ini.end() || 
             adap_ini["master.server"] == "" ) )
      {
         serv = adap_ini["master.server"] ;  
      }
      else
         return false ; 

      if( !( adap_ini.find("master.port") == adap_ini.end() || 
          adap_ini["master.port"] == "" ) )
      {
         std::istringstream iss( adap_ini["master.port"] ) ; 
         if( (iss >> std::dec >> port ).fail() )
            return false ; 
      }
      
      
      return false ; 


  }

  bool context_cpi_impl::populate_user_name( std::string &user, saga::adaptors::attribute const  & c_ , saga::ini::entry_map &adap_ini ) 
  {

     // Check the context
     if( c_.attribute_exists(saga::attributes::context_userid ) )
     {
        user = c_.get_attribute( saga::attributes::context_userid ) ; 
        return true ; 
     }

     // Lastly, check the ini
     if( check_ini_for_username( user, adap_ini ) )
     {
        return true ; 
     }

     // Nothing worked .. Bail out ...
     return false ; 
  }

  bool context_cpi_impl::populate_password( std::string &pass, saga::adaptors::attribute const  &c_ , saga::ini::entry_map &adap_ini ) 
  {

     // Check the context
     if( c_.attribute_exists(saga::attributes::context_userpass ) ) 
     {
        pass = c_.get_attribute( saga::attributes::context_userpass ) ; 
        return true ; 
     }

     // Lastly, check the ini
     if( check_ini_for_pass( pass, adap_ini ) )
     {
        return true ; 
     }

     // Nothing worked .. Bail out ...
     return false ; 
  }

  bool context_cpi_impl::populate_server_info ( std::string &server, saga::adaptors::attribute const  & c_ , saga::ini::entry_map  &adap_ini, int &port )
  {

     // See if a server address and sec certificate is supplied
     // else, load it from the adaptor ini
     if( c_.attribute_exists(saga::attributes::context_server ) )
     {
        saga::url mserver ( c_.get_attribute( saga::attributes::context_server) ) ; 
        port = mserver.get_port() ; 
        if ( port < 0 ) return false ; 
        server = mserver.get_host() ; 
        return true ; 
    }

    // Check ini
    if( check_ini_for_server( server, port, adap_ini ) )
    {
       return true ; 
    }

    return false ; 

  }


  bool context_cpi_impl::populate_sec_cert ( std::string &sec, saga::adaptors::attribute const  & c_ , saga::ini::entry_map &adap_ini )
  {

    // See if supplied in context
    
    if( c_.attribute_exists(saga::attributes::context_usercert ) )
    {
        sec =  c_.get_attribute( saga::attributes::context_server) ; 
        return true ; 
    }

    // Check ini
    
    if( check_ini_for_sec( sec , adap_ini ) )
    {
       return true ; 
    }

    // Everything failed ... bailing out 
    return false ; 

  }

  void context_cpi_impl::get_authentication_error( std::string &err, int const & code )
  {
     auth_.get_error( err , code ) ; 
  }


//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
saga::impl::adaptor_selector::adaptor_info_list_type
context_adaptor::adaptor_register (saga::impl::session * s)
{
  // list of implemented cpi's
  saga::impl::adaptor_selector::adaptor_info_list_type infos;
  preference_type prefs; 

  context_cpi_impl::register_cpi (infos, prefs, adaptor_uuid_);

  // create a default security context if this is a default session
  if ( s->is_default_session () )
  {
    std::vector <std::pair <std::string, std::string> > entries;

    std::pair <std::string, std::string> entry (saga::attributes::context_type, 
                                                "opencloud");

    entries.push_back (entry);

    s->add_proto_context (entries);

  }

  return infos;
}
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

