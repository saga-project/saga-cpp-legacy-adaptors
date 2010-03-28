//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>
#include <fcntl.h>
#include <iostream>

#include "kfs_file_adaptor_file.hpp"

using namespace KFS;

namespace kfs_file_adaptor {
  file_cpi_impl::file_cpi_impl (proxy                * p, 
                                cpi_info       const & info,
                                saga::ini::ini const & glob_ini,
                                saga::ini::ini const & adap_ini,
                                boost::shared_ptr <saga::adaptor> adaptor)
                                : file_cpi (p, info, adaptor, cpi::Noflags) {        
     adaptor_data_t       adata (this);
     file_instance_data_t idata (this);
     
     saga::url file_url(idata->location_);
     std::string url = file_url.get_path();
     
     file_ = 0;
     std::string host(file_url.get_host());
     if(host.empty()) {
        SAGA_ADAPTOR_THROW("No host supplied", saga::IncorrectURL);
     }
     if(file_url.get_port() < 0) {
        SAGA_ADAPTOR_THROW("Negative port not recognized", saga::IncorrectURL);
     }
     std::string scheme(file_url.get_scheme());
     if(!scheme.empty() && scheme != "kfs" && scheme != "any") {
        SAGA_ADAPTOR_THROW("Cannot handle scheme other than \"kfs\" or \"any\"", saga::IncorrectURL);
     }
     fs_ = getKfsClientFactory()->GetClient(file_url.get_host(), file_url.get_port());
     if (!fs_) {
           SAGA_OSSTREAM strm;
           strm << "Could not connect to host : " << file_url.get_host();
           strm << " on port : " << file_url.get_port();
           SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::NoSuccess);
     }
     saga::filesystem::flags mode = (saga::filesystem::flags)idata->mode_;
     
     //Create file if saga::filesystem::Create flag is given and file does not exist
     if( !(mode & saga::filesystem::Create) && fs_->Exists(url.c_str()) == 0 ) {
        SAGA_ADAPTOR_THROW("File does not exists and Create flag not given", saga::BadParameter);
     }
     if (mode & saga::filesystem::Read && mode & saga::filesystem::Write) {
        SAGA_ADAPTOR_THROW("KFS FS does not support ReadWrite mode", saga::NoSuccess);
     }
     if (mode & saga::filesystem::Append) {
        SAGA_ADAPTOR_THROW("KFS does not support append mode yet", saga::NoSuccess);
     }
     if(idata->mode_ & saga::filesystem::Create) {
        file_ = fs_->Create(url.c_str());
        idata->mode_ &= ~saga::filesystem::Create; // no need to create this file twice
     }
     if (idata->mode_ & saga::filesystem::Read && !(idata->mode_ & saga::filesystem::Write)) {
        file_ = fs_->Open(url.c_str(), O_RDWR);
        if(file_ < 0) {
           SAGA_OSSTREAM strm;
           strm << "Could not open file: " << idata->location_;
           SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::DoesNotExist);
        }
     }
     else if (idata->mode_ & saga::filesystem::Write && !(idata->mode_ & saga::filesystem::Read)) {
        file_ = fs_->Open(url.c_str(), O_WRONLY);
        if(file_ < 0) {
           SAGA_OSSTREAM strm;
           strm << "Could not open file: " << idata->location_;
           SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::DoesNotExist);
        }
     }
     if (0 != idata->pointer_) {
        fs_->Seek(file_, idata->pointer_);
        if (fs_->Tell(file_) != idata->pointer_) {
           SAGA_OSSTREAM strm;
           strm << "Could not seek in file: " << idata->location_;
           SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::NoSuccess);
        }
     }
     //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

   file_cpi_impl::~file_cpi_impl (void) {
      /*fs_->Sync(file_);
      fs_->Close(file_);*/
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }


   void file_cpi_impl::sync_get_size (saga::off_t & size_out) {
      struct stat statInfo;
      instance_data idata(this);
      if(fs_->Exists((idata->location_).get_path().c_str()) == 0 ) {
         // Doesn\'t exist
         SAGA_ADAPTOR_THROW ("File Does not exist", saga::BadParameter);
      }
      else {
         fs_->Stat((idata->location_).get_path().c_str(), statInfo);
         size_out = (saga::off_t)statInfo.st_size;
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void file_cpi_impl::sync_read (saga::ssize_t        & len_out,
                                 saga::mutable_buffer   data,
                                 saga::ssize_t          len_in)
   {
      if (len_in < 0) {
         SAGA_ADAPTOR_THROW(
            "file_cpi_impl::sync_read: 'len_in' should not be negative",
             saga::BadParameter);
      }
      if (data.get_size() != -1 && len_in > data.get_size()) {
         SAGA_ADAPTOR_THROW(
            "file_cpi_impl::sync_read: buffer too small for data to read",
             saga::BadParameter);
      }
      instance_data idata(this);
      fs_->Seek(file_, idata->pointer_);
      if(fs_->Tell(file_) != idata->pointer_) {
         SAGA_ADAPTOR_THROW(
            "file_cpi_impl::sync_read: unable to seek to position",
             saga::BadParameter);
      }
      if (-1 == data.get_size()) {
          data.set_size(len_in);    // use implementation managed memory
      }
      len_out = fs_->Read(file_, (char *)data.get_data(), data.get_size());
      idata->pointer_ += (saga::off_t) len_out;
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

   void file_cpi_impl::sync_write (saga::ssize_t      & len_out, 
                                  saga::const_buffer   data,
                                  saga::ssize_t        len_in)
   {
      instance_data idata(this);
      if (len_in < 0) {
         SAGA_ADAPTOR_THROW(
            "file_cpi_impl::sync_write: 'len_in' should not be negative",
            saga::BadParameter);
      }
      // write data 
      mutex_type::scoped_lock lock(mtx_);
    
      fs_->Seek(file_, idata->pointer_);
      int res = fs_->Write(file_, (const char*)data.get_data(), data.get_size());
      if(res < 0) {
         SAGA_OSSTREAM strm;
         instance_data data (this);
         strm << "file_cpi_impl::write: could not write to file: "
              << data->location_.get_url();
         SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm),
            saga::adaptors::Unexpected);
      }
      len_out = data.get_size();
      idata->pointer_ += (saga::off_t)   len_out;
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void file_cpi_impl::sync_seek (saga::off_t                 & out, 
                                 saga::off_t                   offset, 
                                 saga::filesystem::seek_mode   whence)
   {
      instance_data idata(this);
      if(fs_->Exists((idata->location_).get_path().c_str()) == 0 ) {
         SAGA_OSSTREAM strm;
         strm << "File not open file";
         SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::DoesNotExist);
      }
      mutex_type::scoped_lock lock(mtx_);
 
      switch ( (saga::filesystem::seek_mode) whence ) {
         case saga::filesystem::Start:
            fs_->Seek(file_, offset);
            if(fs_->Tell(file_) == 0) {
               SAGA_OSSTREAM strm;
               strm << "Could not seek in file: " << idata->location_;
               SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::NoSuccess);
            }
            break;
         case saga::filesystem::Current:
            fs_->Seek(file_, idata->pointer_ + offset);
            if(fs_->Tell(file_) == 0) {
               SAGA_OSSTREAM strm;
               strm << "Could not seek in file: " << idata->location_;
               SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::NoSuccess);
            }
            break;
         case saga::filesystem::End:
            SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
         default:
            // wasn't able to seek 
            SAGA_OSSTREAM strm;
            strm << "file_cpi_impl::seek: bogus seek mode: " << (int) whence;
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::adaptors::Unexpected);
      }
      idata->pointer_ = (saga::off_t)fs_->Tell(file_);
      out = idata->pointer_;
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }
} // namespace

