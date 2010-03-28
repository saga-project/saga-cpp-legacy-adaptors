//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>
#include <iostream>

#include "kfs_file_adaptor_file.hpp"

namespace kfs_file_adaptor
{
   void file_cpi_impl::sync_get_url (saga::url & url) {
      saga::url u;
      {
         instance_data idata (this);
         u = idata->location_;
      }
      // complete url
      if (u.get_scheme().empty())
         u.set_scheme("kfs");
      if (u.get_host().empty())
         u.set_host("localhost");
      url = u.get_url();
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void file_cpi_impl::sync_get_cwd  (saga::url & cwd) {
      instance_data idata (this);
      saga::url result(idata->location_);
      boost::filesystem::path name (result.get_path(), boost::filesystem::native);
      result.set_path(name.branch_path().string());
      
      // complete url
      if (result.get_scheme().empty())
          result.set_scheme("kfs");
      if (result.get_host().empty())
          result.set_host("localhost");
     
      std::string s(result.get_url());
      cwd = result;
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void file_cpi_impl::sync_get_name (saga::url & name) {
      instance_data idata (this);
      saga::url u (idata->location_.get_path());
      
      boost::filesystem::path path (u.get_path(), boost::filesystem::native);
      name = path.leaf();
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

   void file_cpi_impl::sync_is_dir (bool & is_dir) {
      instance_data idata (this);
      is_dir = false;
     
      // verify current working directory is local
      saga::url url(idata->location_);
      boost::filesystem::path path (idata->location_.get_path(), boost::filesystem::native);
      
      if( fs_->Exists(path.string().c_str()) == 0 ) {
         // Doesn't exist
         SAGA_ADAPTOR_THROW("Path doesn\'t exist", saga::NoSuccess);
      }
      else if (fs_->IsDirectory(path.string().c_str())) {
         is_dir = true;
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void file_cpi_impl::sync_is_entry  (bool & is_file) {
      instance_data idata (this);
      is_file = false;
      
      saga::url url(idata->location_);
      boost::filesystem::path path (idata->location_.get_path(), boost::filesystem::native);
      if(fs_->Exists(path.string().c_str()) == 0) {
         //Check to see if it is a directory
         instance_data idata(this);
         if(fs_->IsFile(path.string().c_str()))
            is_file = true;
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

   void file_cpi_impl::sync_is_link   (bool & is_link) {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void file_cpi_impl::sync_read_link (saga::url & target) {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void file_cpi_impl::sync_copy (saga::impl::void_t & ret, saga::url dest, int flags) {
     instance_data idata (this);
  
     saga::url url(idata->location_);
     boost::filesystem::path src_location (idata->location_.get_path(), boost::filesystem::native);
     boost::filesystem::path dst_location (src_location);
  
     // complete paths
     boost::filesystem::path dest_path (dest.get_path(), boost::filesystem::native);
     if ( ! dest_path.has_root_path () )
         dst_location = dest_path;
     else
         dst_location = dest_path;
  
     bool is_src_dir = false;
     if(fs_->Exists(src_location.string().c_str()) == 0) {
        if(fs_->IsDirectory(src_location.string().c_str())) 
           is_src_dir = true;
     }
     bool is_dst_dir = false;
     if(fs_->Exists(dst_location.string().c_str()) == 0) {
        if(fs_->IsDirectory(dst_location.string().c_str())) 
           is_dst_dir = true;
     } 
     if (!is_src_dir && is_dst_dir)
         dst_location /= src_location.leaf();
  
     // remove the file/directory if it exists and we should overwrite
     bool dst_location_dir = false;
     if(fs_->Exists(dst_location.string().c_str()) == 0) {
        if(fs_->IsDirectory(dst_location.string().c_str()) == 0)
           dst_location_dir = true;
     } 
     if ((flags & saga::name_space::Overwrite) && dst_location_dir) {
         if (is_dst_dir) {
            SAGA_ADAPTOR_THROW("Can not recursive delete", saga::NoSuccess);
         }
         else {
            if(fs_->Remove(dst_location.string().c_str()) == -1)
               SAGA_ADAPTOR_THROW("Can not delete", saga::NoSuccess);
         }
     }
  
     // if destination still exists raise an error
     if( fs_->Exists(dst_location.string().c_str())) {
         SAGA_OSSTREAM strm;
         strm << "sync_copy: " << "target file already exists: " << dest.get_url();
         SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::AlreadyExists);
     }
     //Copy
/*     if(hdfsCopy(fs_, src_location.string().c_str(), fs_, dst_location.string().c_str()) != 0)
     {
         SAGA_ADAPTOR_THROW("Failed to copy", saga::NoSuccess);
     }*/
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void file_cpi_impl::sync_link (saga::impl::void_t & ret,    
                                 saga::url            dest, 
                                 int                  flags) {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void file_cpi_impl::sync_move (saga::impl::void_t & ret,   
                                 saga::url            dest, 
                                 int                  flags) {
      instance_data idata (this);
  
      // verify current working directory is local
      saga::url url(idata->location_);
      boost::filesystem::path src_location (idata->location_.get_path(), boost::filesystem::native);
      boost::filesystem::path dst_location (src_location);
      
      // complete paths
      boost::filesystem::path dest_path (dest.get_path(), boost::filesystem::native);
      if ( ! dest_path.has_root_path () )
          dst_location = dest_path;
      else
          dst_location = dest_path;
      
      bool is_src_dir = false;
      if(fs_->Exists(src_location.string().c_str()) == 0) {
         if(fs_->IsDirectory(src_location.string().c_str()))
            is_src_dir = true;
      }
      bool is_dst_dir = false;
      if(fs_->Exists(dst_location.string().c_str()) == 0) {
         if(fs_->IsDirectory(dst_location.string().c_str()))
            is_dst_dir = true;
      } 
      if (!is_src_dir && is_dst_dir)
          dst_location /= src_location.leaf();
      
      // remove the file/directory if it exists and we should overwrite
      bool dst_location_dir = false;
      if(fs_->Exists(dst_location.string().c_str()) == 0) {
         if(fs_->IsDirectory(dst_location.string().c_str()))
            dst_location_dir = true;
      } 
      if ((flags & saga::name_space::Overwrite) && dst_location_dir) {
          if (is_dst_dir) {
             SAGA_ADAPTOR_THROW("Can not recursive delete", saga::NoSuccess);
          }
          else {
             if(fs_->Remove(dst_location.string().c_str()) == -1)
                SAGA_ADAPTOR_THROW("Can not delete", saga::NoSuccess);
          }
      }
      
      // if destination still exists raise an error
      if( fs_->Exists(dst_location.string().c_str()) == 0 ) {
          SAGA_OSSTREAM strm;
          strm << "sync_copy: " << "target file already exists: " << dest.get_url();
          SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::AlreadyExists);
      }
      //Copy
/*      if(hdfsCopy(fs_, src_location.string().c_str(), fs_, dst_location.string().c_str()) != 0) {
          SAGA_ADAPTOR_THROW("Failed to copy in move", saga::NoSuccess);
      }*/
      //Remove old
      if(fs_->Remove(src_location.string().c_str()) == -1) {
         SAGA_ADAPTOR_THROW("Failed to delete in move", saga::NoSuccess);
      }
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void file_cpi_impl::sync_remove (saga::impl::void_t & ret,
                                   int                  flags) {
      instance_data idata (this);
      // verify current working directory is local
      saga::url url(idata->location_);
      if(fs_->Remove(url.get_path().c_str()) == -1) {
         SAGA_ADAPTOR_THROW("Failed to remove", saga::NoSuccess);
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void file_cpi_impl::sync_close (saga::impl::void_t & ret, 
                                  double               timeout) {
      if(file_)
         fs_->Close(file_);
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }
} // namespace

