//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/exception.hpp>

#include "kfs_file_adaptor_dir.hpp"
#include "kfs_file_adaptor_file.hpp"
#include <iostream>

using namespace KFS;

namespace kfs_file_adaptor {

   dir_cpi_impl::dir_cpi_impl (proxy                * p, 
                              cpi_info       const & info,
                              saga::ini::ini const & glob_ini,
                              saga::ini::ini const & adap_ini,
                              boost::shared_ptr<saga::adaptor> adaptor)
      : directory_cpi (p, info, adaptor, cpi::Noflags) {
      adaptor_data_t            adata (this);
      directory_instance_data_t idata (this);
      
      saga::url dir_url(idata->location_);
      
      std::string host(dir_url.get_host());
      if (host.empty()) {
         SAGA_OSSTREAM strm;
         strm << "dir_cpi_impl::init: cannot handle file: " << dir_url.get_url();
         SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
      }
      
      std::string scheme(dir_url.get_scheme());
      if (!scheme.empty() && scheme != "kfs" && scheme != "any") {
         SAGA_OSSTREAM strm;
         strm << "dir_cpi_impl::init: cannot handle file: " << dir_url.get_url();
         SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
      }
      
      fs_ = getKfsClientFactory()->GetClient(dir_url.get_host(), dir_url.get_port());
      if(!fs_) {
            SAGA_OSSTREAM strm;
            strm << "Could not connect to host : " << dir_url.get_host();
            strm << "on port " << dir_url.get_port();
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::NoSuccess);
      }
      
      // check if file exists AND is a dir (not a file)
      bool exists = false;
      bool is_dir = false;
      if(fs_->Exists(dir_url.get_path().c_str())) {
         exists = true;
         //Check to see if it is a directory
         instance_data idata(this);
         if(fs_->IsDirectory(dir_url.get_path().c_str()))
            is_dir = true;
       }
       saga::filesystem::flags OpenMode = (saga::filesystem::flags)idata->mode_;
       if(exists) {
          if(!is_dir) {
             SAGA_ADAPTOR_THROW ("URL does not point to a directory: " +
                                 idata->location_.get_url(), saga::BadParameter);
          }
          else {
             if((OpenMode & saga::filesystem::Create) && (OpenMode & saga::filesystem::Exclusive)) {
                 SAGA_ADAPTOR_THROW ("Directory " + idata->location_.get_url() +
                                     " already exists.", saga::AlreadyExists);
             }
          }
       }
       else {
       // !exists
          if(!(OpenMode & saga::filesystem::Create)) {
             SAGA_ADAPTOR_THROW ("Directory does not exist and saga::filesystem::Create flag not given: " +
                                 idata->location_.get_url(), saga::DoesNotExist);
          }
          else {
             if(fs_->Mkdir(dir_url.get_path().c_str()) != 0) {
                SAGA_OSSTREAM strm;
                strm << "Could not create directory " << idata->location_.get_path();
                SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::DoesNotExist);
             }
          }
       }
       exists = false;
       is_dir = false;
       //Make sure directory exists
       if(fs_->Exists(dir_url.get_path().c_str()))
          exists = true;
       if ((idata->mode_ & saga::filesystem::Create ||
            idata->mode_ & saga::filesystem::CreateParents) &&
            idata->mode_ & saga::filesystem::Exclusive)
       {
          //Check to see if it is a directory
          if(fs_->IsDirectory(dir_url.get_path().c_str()))
             is_dir = true;
          if(is_dir) {
             SAGA_ADAPTOR_THROW(dir_url.get_path().c_str() + ": already exists",
                saga::AlreadyExists);
          }
       }
       if(!exists) {
          //create directory if needed
         if (idata->mode_ & saga::filesystem::Create) {
            if(fs_->Mkdir(dir_url.get_path().c_str()) != 0) {
               SAGA_OSSTREAM strm;
               strm << "Could not create directory " << idata->location_.get_path();
               SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::DoesNotExist);
            }
         }
       }
      
       // we don't need to create the directory twice
       idata->mode_ &= ~(saga::filesystem::Create | saga::filesystem::CreateParents);
      
       if (idata->mode_ & saga::filesystem::Read || idata->mode_ & saga::filesystem::Write ||
           idata->mode_ & saga::filesystem::ReadWrite) {
          //check to see if it exists and is a direcotory
         exists = false;
         is_dir = false;
         if(fs_->Exists(dir_url.get_path().c_str())) {
            exists = true;
            //Check to see if it is a directory
            if(fs_->IsDirectory(dir_url.get_path().c_str()))
               is_dir = true;
         }
         if (!exists || !is_dir) {
              SAGA_OSSTREAM strm;
              strm << idata->location_.get_path() << ": doesn't refer to a directory object";
              SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
         }
      }
      
      if (idata->mode_ & ~(
              saga::filesystem::Create | saga::filesystem::CreateParents |
              saga::filesystem::Exclusive | saga::filesystem::Overwrite |
              saga::filesystem::Read | saga::filesystem::Write | saga::filesystem::ReadWrite
      )) {
         SAGA_ADAPTOR_THROW("Unknown openmode value: " +
            boost::lexical_cast<std::string>(idata->mode_), saga::BadParameter);
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   dir_cpi_impl::~dir_cpi_impl (void) {
   }

   void dir_cpi_impl::sync_get_size (saga::off_t & size_out, 
                                    saga::url      name, int flag) 
   {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_open (saga::filesystem::file & ret, 
                                saga::url                name_to_open, 
                                int                      openmode) {
      instance_data idata (this);
      bool exists = false; 
      bool is_dir = false; 
     
      saga::url file_url(idata->location_);
      boost::filesystem::path name (name_to_open.get_path(), boost::filesystem::native);
      boost::filesystem::path path (file_url.get_path(), boost::filesystem::native);
     
      if ( ! name.has_root_path () ) {
        path /= name;
        file_url.set_path(path.string());
      }
      else {
        path = name;
        file_url = saga::url(name.string());
      }
      
      if(fs_->Exists(path.string().c_str())) {
         exists = true;
         //Check to see if it is a directory
         if(fs_->IsDirectory(path.string().c_str()))
            is_dir = true;
      }
      if ( exists && is_dir ) {
        SAGA_ADAPTOR_THROW(path.string() + ": doesn't refer to a file object",
          saga::DoesNotExist);
      }
     
      ret = saga::filesystem::file (this->get_proxy()->get_session(), file_url.get_url(), openmode);
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_open_dir (saga::filesystem::directory & ret, 
                                    saga::url                     name_to_open,
                                    int                           openmode) {
      instance_data idata (this);
      bool exists = false; 
      bool is_dir = false; 
     
      saga::url file_url(idata->location_);
      boost::filesystem::path name (name_to_open.get_path(), boost::filesystem::native);
      boost::filesystem::path path (file_url.get_path(), boost::filesystem::native);
     
      if ( ! name.has_root_path () ) {
        path /= name;
        file_url.set_path(path.string());
      }
      else {
        path = name;
        file_url = saga::url(name.string());
      }
      if(fs_->Exists(path.string().c_str())) {
         exists = true;
         //Check to see if it is a directory
         if(fs_->IsDirectory(path.string().c_str()))
            is_dir = true;
      }
      if ( exists && !is_dir) {
        SAGA_ADAPTOR_THROW(path.string() + ": doesn't refer to a file object",
          saga::DoesNotExist);
      }
     
      ret = saga::filesystem::directory (this->get_proxy()->get_session(), file_url.get_url(),
                               openmode);
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_file (bool    & is_file, 
                                   saga::url name)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

} // namespace

