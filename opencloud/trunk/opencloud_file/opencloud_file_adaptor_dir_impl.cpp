//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/exception.hpp>

#include "opencloud_file_adaptor_dir.hpp"
#include "opencloud_file_adaptor_file.hpp"

namespace opencloud_file_adaptor
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  dir_cpi_impl::dir_cpi_impl (proxy                * p, 
                              cpi_info       const & info,
                              saga::ini::ini const & glob_ini,
                              saga::ini::ini const & adap_ini,
                              boost::shared_ptr<saga::adaptor> adaptor)

      : directory_cpi (p, info, adaptor, cpi::Noflags), 
        dserv_( serv_.get_sector_dir_service())
  {
    std::string error ; 
    adaptor_data_t            adata (this);
    directory_instance_data_t idata (this);

    /* Parse the location and open the directory 
     */
    location =  idata->location_  ; 
    set_dir_name() ; 
    s_ = p->get_session() ; 
    mode = idata->mode_ ; 


    /* 
    if( !adata->get_auth_flag() )
    {
       bool auth = adata->authenticate( s_, location, mode, serv_, glob_ini, adap_ini ) ; 
       if( !auth )
       {
          SAGA_ADAPTOR_THROW ("Could not authenticate with the Sector/Sphere Master Server.", saga::NoSuccess ) ; 
       }
       adata->set_auth_flag() ; 
    }
    */

    bool open = open_dir( error ) ; 
    if( !open )
    {
      SAGA_ADAPTOR_THROW ( error , saga::NoSuccess ) ; 
    }

  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  dir_cpi_impl::~dir_cpi_impl (void)
  {
  }

  void dir_cpi_impl::sync_get_size (saga::off_t & size_out, 
                                    saga::url      name, 
                                    int            flag)
  {
    int res  = dserv_.get_dir_size( size_out )   ; 
    if( res < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
    {
       std::string err ("Sector/Sphere failed to report dir size - ") ; 
       dserv_.get_error_msg( err, res ) ; 
       SAGA_ADAPTOR_THROW ( err , saga::NoSuccess );
    }

    return ; 
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_open (saga::filesystem::file & ret, 
                                saga::url                name, 
                                int                      openmode)
  {
    
    std::string to_open ; 
    
    /* If it is a relative path, then 
     * construct the appropriate URL and
     * create the file in the same session
     */
    if ( name.get_scheme().empty() &&
         name.get_host().empty()   &&
         name.get_path()[0] != '/' )
    {
       to_open += "sector://" ; 
       to_open += dir_ ; 
       to_open += '/' ; 
       to_open += name.get_path() ; 
       ret = saga::filesystem::file (s_, saga::url(to_open), openmode ) ; 
       return ; 
    }

    ret = saga::filesystem::file ( s_, name, openmode ) ; 
    return ; 

  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_open_dir (saga::filesystem::directory & ret, 
                                    saga::url                     name, 
                                    int                           openmode)
  {

    std::string to_open ; 

    if ( name.get_scheme().empty() &&
         name.get_host().empty()   &&
         name.get_path ()[0] != '/' )
    {
       to_open += "sector://" ; 
       to_open += dir_ ; 
       to_open += '/' ; 
       to_open += name.get_path() ; 
       ret = saga::filesystem::directory ( s_, saga::url (to_open), openmode ) ; 
       return ; 
    }

    ret = saga::filesystem::directory (s_,  name, openmode ) ; 
    return ; 

  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_file (bool    & is_file, 
                                   saga::url name)
  {
    std::string to_test ; 
    saga::filesystem::file ret ; 

    if( resolve( name, to_test ) )
    {
       std::string to_open ("sector://") ; 
       to_open += dir_ ; 
       to_open += '/' ; 
       to_open += to_test ; 
       ret = saga::filesystem::file ( s_, saga::url (to_test) ) ; 
       is_file = !ret.is_dir() ; 
       return ; 
    }

    ret = saga::filesystem::file ( s_, name ) ; 
    is_file = !ret.is_dir() ; 
    return ; 
  }

  void dir_cpi_impl::set_dir_name ()
  {

    // can we handle this type of URL ?
    if ( location.get_scheme () != "any" &&
         location.get_scheme () != "sector" &&
         !location.get_scheme().empty() )
    {
      SAGA_LOG_DEBUG  (location.get_string ().c_str ());
      SAGA_ADAPTOR_THROW_NO_CONTEXT ("Cannot handle URL schema", saga::BadParameter);
    }

    resolve( location, dir_ ) ; 

  }

  bool dir_cpi_impl::open_dir( std::string &error )
  {
     int res = dserv_.open( dir_, mode ) ; 
     if( res < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        dserv_.get_error_msg( error, res ) ; 
	return false ; 
     }

     return true ; 
 
  }

  bool dir_cpi_impl::resolve( saga::url const &url_, std::string &ret )
  {
    std::string temp = url_.get_host() ; 

    if( temp.empty() )
    {

       if( (! ( url_.get_path()).empty() ))
       {
           ret = url_.get_path() ; 

           if ( url_.get_scheme().empty() &&
                url_.get_host().empty()   &&
                url_.get_path ()[0] != '/' )
           {
              return true ;      
           }

           return false ;    
      }

    }

    ret = temp ; 

    return false ; 
  }


} // namespace

