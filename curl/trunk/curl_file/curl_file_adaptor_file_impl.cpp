//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>

#include "curl_file_adaptor_file.hpp"

namespace curl_file_adaptor
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  file_cpi_impl::file_cpi_impl (proxy                * p, 
                                cpi_info       const & info,
                                saga::ini::ini const & glob_ini,
                                saga::ini::ini const & adap_ini,
                                boost::shared_ptr <saga::adaptor> adaptor)

      : file_cpi (p, info, adaptor, cpi::Noflags)
  {        
    adaptor_data_t       adata (this);
    file_instance_data_t idata (this);

    i_ = adap_ini.get_section ("preferences").get_entries ();
    s_ = p->get_session ();
    m_ = idata->mode_;
    u_ = idata->location_;

    // FIXME: if possible, dynamically check which schemes can be accepted 
    //        by inspecting curl
    if ( u_.get_scheme () != "http"  &&
         u_.get_scheme () != "https" &&
         u_.get_scheme () != "ftp"   &&
         u_.get_scheme () != "file"  &&
         u_.get_scheme () != "scp"   &&
         u_.get_scheme () != "ssh" ) 
    {
      SAGA_ADAPTOR_THROW ("Cannot handle URL", saga::BadParameter);
    }

    SAGA_LOG_DEBUG (u_.get_string ().c_str ());
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  file_cpi_impl::~file_cpi_impl (void)
  {
  }


//  ///////////////////////////////////////////////////////////////////////////////
//  //
//  void file_cpi_impl::sync_get_size (saga::off_t & size_out)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
// 
//  ///////////////////////////////////////////////////////////////////////////////
//  //
//  void file_cpi_impl::sync_read (saga::ssize_t        & len_out,
//                                 saga::mutable_buffer   data,
//                                 saga::ssize_t          len_in)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
// 
//  ///////////////////////////////////////////////////////////////////////////////
//  //
//  void file_cpi_impl::sync_write (saga::ssize_t      & len_out, 
//                                  saga::const_buffer   data,
//                                  saga::ssize_t        len_in)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
// 
//  ///////////////////////////////////////////////////////////////////////////////
//  //
//  void file_cpi_impl::sync_seek (saga::off_t                 & out, 
//                                 saga::off_t                   offset, 
//                                 saga::filesystem::seek_mode   whence)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }

} // namespace

