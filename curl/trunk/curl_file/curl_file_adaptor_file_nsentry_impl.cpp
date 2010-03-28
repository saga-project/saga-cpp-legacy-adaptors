//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <stdio.h>
#include <errno.h>

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>

#include "curl_file_adaptor_file.hpp"

namespace curl_file_adaptor
{
//  ///////////////////////////////////////////////////////////////////////////////
//  //
//  void file_cpi_impl::sync_get_url (saga::url & url)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
// 
//  void file_cpi_impl::sync_get_cwd  (saga::url & cwd)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
// 
//  void file_cpi_impl::sync_get_name (saga::url & name)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
// 
//  void file_cpi_impl::sync_is_dir (bool & is_dir)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
// 
//  void file_cpi_impl::sync_is_entry  (bool & is_file)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
// 
//  void file_cpi_impl::sync_is_link   (bool & is_link)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
// 
//  void file_cpi_impl::sync_read_link (saga::url & target)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }

  void file_cpi_impl::sync_copy (saga::impl::void_t & ret, 
                                 saga::url            dest, 
                                 int                  flags)
  {
    adaptor_data_t       adata (this);
    file_instance_data_t idata (this);

    // check preconditions
    // We can only check coditions for local targets
    
    if ( saga::adaptors::utils::is_local_address (dest) )
    {
      saga::filesystem::directory loc ("/");

      if ( loc.exists (dest.get_path ()) )
      {
        if ( ! (flags & saga::name_space::Overwrite) )
        {
          std::stringstream ss;
          ss << "Target exists: " << dest;
          SAGA_ADAPTOR_THROW (ss.str (), saga::AlreadyExists);
        }
      }
    }

    std::string src (u_.get_string ());
    std::string tgt (dest.get_string ());


    CURLcode code;

    // get handle for input and output curl ops (see README)
    CURL * in  = adata->get_curl_handle_in  ();
    CURL * out = adata->get_curl_handle_out ();

    // create buffer file
    FILE * buf = ::fopen ("/tmp/curl-cache.dat", "w+");
    ensure (NULL != buf, "fopen() failed: ", ::strerror (errno));

    // read data into buffer
    code = curl_easy_setopt (in, CURLOPT_URL, src.c_str ());
    ensure (0 == code, "Curl Error: ", curl_easy_strerror (code));

    code = curl_easy_setopt (in, CURLOPT_WRITEDATA, buf);
    ensure (0 == code, "Curl Error: ", curl_easy_strerror (code));

    code = curl_easy_perform (in);
    ensure (0 == code, "Curl Error: ", curl_easy_strerror (code));


    // data are in buffer - now rewind buffer, and copy data to target location
    ::rewind (buf);

    code = curl_easy_setopt (out, CURLOPT_URL, tgt.c_str ());
    ensure (0 == code, "Curl Error: ", curl_easy_strerror (code));

    code = curl_easy_setopt (out, CURLOPT_VERBOSE, "true");
    ensure (0 == code, "Curl Error: ", curl_easy_strerror (code));

    code = curl_easy_setopt (out, CURLOPT_USERPWD, "merzky:Z(I)nfandel");
    ensure (0 == code, "Curl Error: ", curl_easy_strerror (code));

    code = curl_easy_setopt (out, CURLOPT_UPLOAD, "true");
    ensure (0 == code, "Curl Error: ", curl_easy_strerror (code));

    code = curl_easy_setopt (out, CURLOPT_READDATA, buf);
    ensure (0 == code, "Curl Error: ", curl_easy_strerror (code));

    code = curl_easy_perform (out);
    ensure (0 == code, "Curl Error: ", curl_easy_strerror (code));

    ::fclose (buf);

    // done :-)
  }

  //  void file_cpi_impl::sync_link (saga::impl::void_t & ret,    
  //                                 saga::url            dest, 
  //                                 int                  flags)
  //  {
  //    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  //  }
  // 
  //  void file_cpi_impl::sync_move (saga::impl::void_t & ret,   
  //                                 saga::url            dest, 
  //                                 int                  flags)
  //  {
  //    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  //  }
  // 
  //  void file_cpi_impl::sync_remove (saga::impl::void_t & ret,
  //                                   int                  flags)
  //  {
  //    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  //  }
  // 
  // 
  //  void file_cpi_impl::sync_close (saga::impl::void_t & ret, 
  //                                  double               timeout)
  //  {
  //    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  //  }

} // namespace

