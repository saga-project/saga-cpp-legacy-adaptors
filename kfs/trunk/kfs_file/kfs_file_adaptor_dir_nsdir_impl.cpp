//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>

#include "kfs_file_adaptor_dir.hpp"

namespace kfs_file_adaptor {

   void dir_cpi_impl::sync_change_dir (saga::impl::void_t & ret, 
                                      saga::url            name) {
      
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void dir_cpi_impl::sync_list (std::vector <saga::url> & list, 
                                std::string               pattern, 
                                int                       flags) {
      instance_data idata(this);
      std::vector<std::string> list_string;
      if(fs_->Readdir(idata->location_.get_path().c_str(), list_string) != 0) {
         SAGA_ADAPTOR_THROW ("List error", saga::NoSuccess);
      }
      for(std::vector<std::string>::size_type counter = 0; counter < list_string.size(); counter++) {
         list.push_back(saga::url(list_string[counter]));
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void dir_cpi_impl::sync_find (std::vector <saga::url> & list, 
                                std::string               entry, 
                                int                       flags) {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void dir_cpi_impl::sync_exists (bool    & exists, 
                                  saga::url url) {
      
      instance_data idata (this);
      saga::url dir_url(idata->location_);
      boost::filesystem::path name (url.get_path(), boost::filesystem::native);
      boost::filesystem::path path (idata->location_.get_path(), boost::filesystem::native);
      
      if ( ! name.has_root_path () )
        path /= name;
      else
        path = name;
      exists = false;
      if(fs_->Exists(path.string().c_str())) {
         exists = true;
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void dir_cpi_impl::sync_is_dir (bool    & is_dir, 
                                  saga::url url) {
      instance_data idata (this);
      is_dir = false;
     
      saga::url dir_url(idata->location_);
      boost::filesystem::path name (url.get_path(), boost::filesystem::native);
      boost::filesystem::path path (idata->location_.get_path(), boost::filesystem::native);
     
      if ( ! name.has_root_path () )
         path /= name;
      else
         path = name;
     
      if(fs_->Exists(path.string().c_str())) {
         //Check to see if it is a directory
         if(fs_->IsDirectory(path.string().c_str()))
            is_dir = true;
       }
       //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void dir_cpi_impl::sync_is_entry (bool    & is_entry, 
                                    saga::url url) {
      instance_data idata (this);
      saga::url dir_url(idata->location_);
      is_entry = false;
     
      boost::filesystem::path name (url.get_path(), boost::filesystem::native);
      boost::filesystem::path path (idata->location_.get_path(), boost::filesystem::native);
     
      if ( ! name.has_root_path() )
        path /= name;
      else
        path = name;
      
      if(fs_->Exists(path.string().c_str())) {
         //Check to see if it is a directory
         if(fs_->IsFile(path.string().c_str()))
            is_entry = true;
       }
       //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void dir_cpi_impl::sync_is_link (bool    & is_link, 
                                   saga::url url) {
      is_link = false;
   }

   void dir_cpi_impl::sync_read_link (saga::url & ret, 
                                     saga::url   source) {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void dir_cpi_impl::sync_get_num_entries (std::size_t & num) {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void dir_cpi_impl::sync_get_entry (saga::url & ret, 
                                     std::size_t entry ) {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void dir_cpi_impl::sync_link (saga::impl::void_t & ret, 
                                saga::url            source, 
                                saga::url            url, 
                                int                  flags) {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void dir_cpi_impl::sync_copy (saga::impl::void_t & ret, 
                                saga::url            src, 
                                saga::url            dst, 
                                int                  flags) {
      instance_data idata (this);
      saga::url url(idata->location_);
      
      // handle the files
      boost::filesystem::path src_location (idata->location_.get_path(), boost::filesystem::native);
      boost::filesystem::path dst_location (src_location);
      
      // complete paths
      boost::filesystem::path src_path  (src.get_path(), boost::filesystem::native);
      boost::filesystem::path dest_path (dst.get_path(), boost::filesystem::native);
      
      if ( ! src_path.has_root_path () )
          src_location /= src_path;
      else
          src_location = src_path;
      
      if ( ! dest_path.has_root_path () )
          dst_location /= dest_path;
      else
          dst_location = dest_path;
      
      bool is_src_dir = false;
      if(fs_->Exists(src_location.string().c_str())) {
         //Check to see if it is a directory
         if(fs_->IsDirectory(src_location.string().c_str()))
            is_src_dir = true;
      }
      // src location refers to a is a directory
      if (is_src_dir) {
         SAGA_ADAPTOR_THROW("Cannot copy directory at moment.", saga::NotImplemented);
      }
      else {
          //Check to see if dst_location is a directory
         bool is_dst_dir = false;
         bool dst_exists = false;
         if(fs_->Exists(dst_location.string().c_str())) {
            dst_exists = true;
            //Check to see if it is a directory
            if(fs_->Exists(dst_location.string().c_str()))
               is_dst_dir = true;
         }
         else {
            SAGA_ADAPTOR_THROW("Path does not exists!", saga::NoSuccess);
         }
      
         if (is_dst_dir)
             dst_location /= src_location.leaf();
     
         // remove the file/directory if it exists and we should overwrite
         if ((flags & saga::name_space::Overwrite) && dst_exists) {
/*             if (is_dst_dir)
                 fs::remove_all(dst_location);*/
            if(fs_->Remove(dst_location.string().c_str()) != 0) {
               SAGA_ADAPTOR_THROW("Could not delete", saga::NoSuccess);
            }
         }
     
         if(fs_->Exists(dst_location.string().c_str())) {
             SAGA_OSSTREAM strm;
             strm << "namespace_dir_cpi_impl<Base>::sync_copy: "
                 "target file already exists: " << dst.get_url();
             SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::AlreadyExists);
         }
         /*if(hdfsCopy(fs_, src_location.string().c_str(), fs_, dst_location.string().c_str()) != 0)
         {
             SAGA_OSSTREAM strm;
             strm << "namespace_dir_cpi_impl<Base>::sync_copy: "
                 "target file did not copy successfully: " << dst.get_url();
             SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::NoSuccess);
         }*/
      }
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void dir_cpi_impl::sync_move (saga::impl::void_t & ret, 
                                saga::url            src, 
                                saga::url            dest, 
                                int                  flags) {
      instance_data idata (this);
      
      // verify current working directory is local
      saga::url url(idata->location_);
      
      // handle the files
      boost::filesystem::path src_location (idata->location_.get_path(), boost::filesystem::native);
      boost::filesystem::path dst_location (src_location);
      // complete paths
      boost::filesystem::path src_path  (src.get_path(), boost::filesystem::native);
      boost::filesystem::path dest_path (dest.get_path(), boost::filesystem::native);
      
      if ( ! src_path.has_root_path () )
          src_location /= src_path;
      else
          src_location = src_path;
      
      if ( ! dest_path.has_root_path () )
          dst_location /= dest_path;
      else
          dst_location = dest_path;
      
      bool is_src_dir;
      bool is_dst_dir;
      if(fs_->Exists(src_location.string().c_str())) {
         //Check to see if it is a directory
         if(fs_->IsDirectory(src_location.string().c_str()))
            is_src_dir = true;
         else
            is_src_dir = false;
      }
      bool dst_exists = false;
      if(fs_->Exists(dst_location.string().c_str())) {
         dst_exists = true;
         //Check to see if it is a directory
         if(fs_->IsDirectory(dst_location.string().c_str()))
            is_dst_dir = true;
         else
            is_dst_dir = false;
      }
      
      if (!is_src_dir && is_dst_dir)
          dst_location /= src_location.leaf();
      
      if ((flags & saga::name_space::Overwrite) && dst_exists) {
         /*if (is_dst_dir)
              fs::remove_all(dst_location);*/
         if(fs_->Remove(dst_location.string().c_str()) != 0) {
            SAGA_ADAPTOR_THROW("Could not delete", saga::NoSuccess);
         }
      }
      // if destination still exists raise an error
      dst_exists = false;
      if(fs_->Exists(dst_location.string().c_str())) {
         SAGA_OSSTREAM strm;
         strm << "namespace_dir_cpi_impl<Base>::sync_move: "
             "target file already exists: " << dest.get_url();
         SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::AlreadyExists);
      }
      if(fs_->Rename(src_location.string().c_str(), dst_location.string().c_str(), false) != 0) {
         SAGA_ADAPTOR_THROW("Unable to move file", saga::NoSuccess);
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void dir_cpi_impl::sync_remove (saga::impl::void_t & ret, 
                                  saga::url            url, 
                                  int                  flags) {
      instance_data idata (this);
      saga::url dir_url(idata->location_);
      boost::filesystem::path src_location (idata->location_.get_path(), boost::filesystem::native);
      // complete paths
      boost::filesystem::path src_path (url.get_path(), boost::filesystem::native);
      if ( ! src_path.has_root_path () )
          src_location /= src_path;
      else
          src_location = src_path;
      
      bool is_src_dir = false;
      if(fs_->Exists(src_location.string().c_str())) {
          SAGA_ADAPTOR_THROW("directory::remove: Can't remove directory: "
            "Does not exist", saga::DoesNotExist);
      }
      else {
         if(fs_->IsDirectory(src_location.string().c_str()))
            is_src_dir = true;
         else
            is_src_dir = false;
      }
      if (is_src_dir) {
          if (saga::name_space::Recursive != flags) {
              SAGA_ADAPTOR_THROW("directory::remove: Can't remove directory. "
                  "Please use recursive mode!", saga::BadParameter);
          }
          else {
             if(fs_->Remove(src_location.string().c_str()) != 0) {
                SAGA_ADAPTOR_THROW("Could not delete", saga::NoSuccess);
             }
          }
      }
      else {
         if(fs_->Remove(src_location.string().c_str()) != 0) {
            SAGA_ADAPTOR_THROW("Could not delete", saga::NoSuccess);
         }
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void dir_cpi_impl::sync_copy_wildcard (saga::impl::void_t & ret, 
                                         std::string          source, 
                                         saga::url            dest, 
                                         int                  flags) {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void dir_cpi_impl::sync_link_wildcard (saga::impl::void_t & ret, 
                                         std::string          source, 
                                         saga::url            dest, 
                                         int                  flags) {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void dir_cpi_impl::sync_move_wildcard (saga::impl::void_t & ret, 
                                         std::string          source, 
                                         saga::url            dest, 
                                         int                  flags) {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void dir_cpi_impl::sync_remove_wildcard (saga::impl::void_t & ret, 
                                           std::string          url, 
                                           int                  flags) {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void dir_cpi_impl::sync_make_dir (saga::impl::void_t & ret, 
                                    saga::url            url, 
                                    int                  flags) {
      instance_data idata (this);
     
      // verify current working directory is local
      saga::url dir_url(idata->location_);
     
      boost::filesystem::path src_location (idata->location_.get_path(), boost::filesystem::native);
      // complete paths
      boost::filesystem::path src_path (url.get_path(), boost::filesystem::native);
     
      if ( ! src_path.has_root_path () )
          src_location /= src_path;
      else
          src_location = src_path;
      
      if(fs_->Mkdir(src_location.string().c_str()) != 0) {
         SAGA_ADAPTOR_THROW("Could not create directory", saga::NoSuccess);
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

} // namespace

