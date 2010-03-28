//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>

#include "opencloud_file_adaptor_file.hpp"

namespace opencloud_file_adaptor
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_get_url (saga::url & url)
  {
    url = location ; 
  }

  void file_cpi_impl::sync_get_cwd  (saga::url & cwd)
  {
    SAGA_ADAPTOR_THROW ("Not yet implemented in Sector/Sphere", saga::NotImplemented);
  }

  void file_cpi_impl::sync_get_name (saga::url & name)
  {
    name = location ; 
  }

  void file_cpi_impl::sync_is_dir (bool & is_dir)
  {
     int res  = secf.is_dir( is_dir ) ; 
     if( res < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
       std::string error ; 
       secf.get_error_msg( error, res ) ; 
       SAGA_ADAPTOR_THROW ( error , saga::NoSuccess );
     }

     return ; 
  }

  void file_cpi_impl::sync_is_entry  (bool & is_file)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void file_cpi_impl::sync_is_link   (bool & is_link)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented in Sector/Sphere", saga::NotImplemented);
  }

  void file_cpi_impl::sync_read_link (saga::url & target)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented in Sector/Sphere", saga::NotImplemented);
  }

  void file_cpi_impl::sync_copy (saga::impl::void_t & ret, 
                                 saga::url dest, int flags)
  {

    saga_sector::file_service &secf = serv_.get_sector_file_service() ; 
    std::string err ; 
    std::string s_dest ; 
    resolve( dest, s_dest ) ; 

    int res = secf.copy( s_dest ) ; 

    if( res < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
    {
       secf.get_error_msg( err, res ) ; 
       SAGA_ADAPTOR_THROW ( err.c_str() , saga::NoSuccess ) ; 
    }

    return ; 

  }

  void file_cpi_impl::sync_link (saga::impl::void_t & ret,    
                                 saga::url            dest, 
                                 int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented by Sector/Sphere", saga::NotImplemented);
  }

  void file_cpi_impl::sync_move (saga::impl::void_t & ret,   
                                 saga::url            dest, 
                                 int                  flags)
  {

    std::string err ; 
    std::string dest_s ; 
    resolve( dest, dest_s) ; 

    int res  = secf.move( dest_s ) ; 

    if( res < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
    {
       secf.get_error_msg( err, res ) ; 
       SAGA_ADAPTOR_THROW ( err.c_str() , saga::NoSuccess ) ; 
    }
    
    return ; 
  }

  void file_cpi_impl::sync_remove (saga::impl::void_t & ret,
                                   int                  flags)
  {

    int res  = secf.remove( ) ; 

    if( res < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
    {
       std::string err ; 
       secf.get_error_msg( err, res ) ; 
       SAGA_ADAPTOR_THROW ( err.c_str() , saga::NoSuccess ) ; 
    }

    return ; 
  }


  void file_cpi_impl::sync_close (saga::impl::void_t & ret, 
                                  double               timeout)
  {
     secf.close() ; 
  }

} // namespace

