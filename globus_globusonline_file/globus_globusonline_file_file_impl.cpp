//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>

#include "globus_globusonline_file_file.hpp"

namespace globus_globusonline_file_adaptor
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

    //SAGA_ADAPTOR_THROW ("Not Implemented (yet), but soon will be!", saga::NotImplemented);
  
    saga::url location(idata->location_);
    std::string host(location.get_host());
    std::string scheme(location.get_scheme());

    // make sure that we only allow globusonline:// URLs

    if (scheme != "globusonline")
    {
       SAGA_OSSTREAM strm;
       strm << "Could not initialize file object for [" << idata->location_ << "]. "
            << "Only globusonline:// schemes are supported.";
       SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::adaptors::AdaptorDeclined);
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  file_cpi_impl::~file_cpi_impl (void)
  {
  }


//   ///////////////////////////////////////////////////////////////////////////////
//   //
//   void file_cpi_impl::sync_get_size (saga::off_t & size_out)
//   {
//     SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//   }
// 
//   ///////////////////////////////////////////////////////////////////////////////
//   //
//   void file_cpi_impl::sync_read (saga::ssize_t        & len_out,
//                                  saga::mutable_buffer   data,
//                                  saga::ssize_t          len_in)
//   {
//     SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//   }
// 
//   ///////////////////////////////////////////////////////////////////////////////
//   //
//   void file_cpi_impl::sync_write (saga::ssize_t      & len_out, 
//                                   saga::const_buffer   data,
//                                   saga::ssize_t        len_in)
//   {
//     SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//   }
// 
//   ///////////////////////////////////////////////////////////////////////////////
//   //
//   void file_cpi_impl::sync_seek (saga::off_t                 & out, 
//                                  saga::off_t                   offset, 
//                                  saga::filesystem::seek_mode   whence)
//   {
//     SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//   }

} // namespace

