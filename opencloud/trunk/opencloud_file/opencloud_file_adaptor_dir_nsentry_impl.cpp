//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>

#include "opencloud_file_adaptor_dir.hpp"

namespace opencloud_file_adaptor
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_get_url (saga::url & url)
  {
     url = location ; 
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_get_cwd (saga::url & cwd)
  {
      cwd = dir_ ; 
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_get_name (saga::url & name)
  {
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
    name =  dir_ ; 
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_dir (bool & is_dir)
  {
      sync_is_dir( is_dir, location) ; 
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_entry (bool & is_entry)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_link (bool & is_link)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented in Sector/Sphere", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_read_link (saga::url & target)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented in Sector/Sphere", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_link (saga::impl::void_t & ret, 
                                saga::url            dest, 
                                int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented in Sector/Sphere", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_copy (saga::impl::void_t & ret, 
                                saga::url            dest, 
                                int                  flags)
  {    
     return ( sync_copy( ret, location, dest, flags )) ; 
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_move (saga::impl::void_t & ret, 
                                saga::url            dest, 
                                int                  flags)
  {
     return ( sync_move( ret, location, dest, flags )) ; 
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_remove (saga::impl::void_t & ret, 
                                  int                  flags)
  {
     return( sync_remove( ret, location, flags )) ; 
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_close (saga::impl::void_t & ret, 
                                 double               timeout)
  {
     dserv_.close() ; 
  }

} // namespace

