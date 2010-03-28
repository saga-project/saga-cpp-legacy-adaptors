//  Copyright (c) 2008 Chris Miceli <cmicel1@cct.lsu.edu>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>

#include "hdfs_file_adaptor_file.hpp"

namespace hdfs_file_adaptor
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_get_url (saga::url & url)
  {
     saga::url u;
     {
         instance_data idata (this);
         u = idata->location_;
     }
     
     // complete url
     if (u.get_scheme().empty())
         u.set_scheme("hdfs");
     if (u.get_host().empty())
         u.set_host("localhost");
     
     url = u.get_url();
  }

  void file_cpi_impl::sync_get_cwd  (saga::url & cwd)
  {
     instance_data idata (this);
     saga::url result(idata->location_);
     boost::filesystem::path name (result.get_path(), boost::filesystem::native);
         result.set_path(name.branch_path().string());

     // complete url
     if (result.get_scheme().empty())
         result.set_scheme("hdfs");
     if (result.get_host().empty())
         result.set_host("localhost");
    
     std::string s(result.get_url());
     cwd = result;
  }

  void file_cpi_impl::sync_get_name (saga::url & name)
  {
     instance_data idata (this);
     saga::url u (idata->location_.get_path());

     boost::filesystem::path path (u.get_path(), boost::filesystem::native);
     name = path.leaf();
  }

  void file_cpi_impl::sync_is_dir (bool & is_dir)
  {
     instance_data idata (this);
     is_dir = false;
    
     // verify current working directory is local
     saga::url url(idata->location_);
     boost::filesystem::path path (idata->location_.get_path(), boost::filesystem::native);

     if( hdfsExists(fs_, path.string().c_str()) == 0 )
     {
        hdfsFileInfo *info;
      
        info = hdfsGetPathInfo(fs_, path.string().c_str());
        if(info == NULL)
        {
           SAGA_ADAPTOR_THROW("Can not get path info", saga::NoSuccess);
        }
        if(info->mKind == kObjectKindDirectory)
           is_dir = true;
        hdfsFreeFileInfo(info, 1);
     }
  }

  void file_cpi_impl::sync_is_entry  (bool & is_file)
  {
     instance_data idata (this);
     is_file = false;
    
     // verify current working directory is local
     saga::url url(idata->location_);
     boost::filesystem::path path (idata->location_.get_path(), boost::filesystem::native);

     if( hdfsExists(fs_, path.string().c_str()) == 0 )
     {
        hdfsFileInfo *info;
      
        info = hdfsGetPathInfo(fs_, path.string().c_str());
        if(info == NULL)
        {
           SAGA_ADAPTOR_THROW("Can not get path info", saga::NoSuccess);
        }
        if(info->mKind == kObjectKindFile)
           is_file = true;
        hdfsFreeFileInfo(info, 1);
     }
  }

  void file_cpi_impl::sync_is_link   (bool & is_link)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void file_cpi_impl::sync_read_link (saga::url & target)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void file_cpi_impl::sync_copy (saga::impl::void_t & ret, 
                                 saga::url dest, int flags)
  {
     // handle the file
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
     if( hdfsExists(fs_, src_location.string().c_str()) == 0 )
     {
        hdfsFileInfo *info;
      
        info = hdfsGetPathInfo(fs_, src_location.string().c_str());
        if(info == NULL)
        {
           SAGA_ADAPTOR_THROW("Can not get path info", saga::NoSuccess);
        }
        if(info->mKind == kObjectKindDirectory)
           is_src_dir = true;
        hdfsFreeFileInfo(info, 1);
     }
     bool is_dst_dir = false;
     if( hdfsExists(fs_, dst_location.string().c_str()) == 0 )
     {
        hdfsFileInfo *info;
       
        info = hdfsGetPathInfo(fs_, dst_location.string().c_str());
        if(info == NULL)
        {
           SAGA_ADAPTOR_THROW("Can not get path info", saga::NoSuccess);
        }
        if(info->mKind == kObjectKindDirectory)
           is_dst_dir = true;
        hdfsFreeFileInfo(info, 1);
     } 
     if (!is_src_dir && is_dst_dir)
         dst_location /= src_location.leaf();
  
     // remove the file/directory if it exists and we should overwrite
     bool dst_location_dir = false;
     if( hdfsExists(fs_, dst_location.string().c_str()) == 0 )
     {
        hdfsFileInfo *info;
       
        info = hdfsGetPathInfo(fs_, dst_location.string().c_str());
        if(info == NULL)
        {
           SAGA_ADAPTOR_THROW("Can not get path info", saga::NoSuccess);
        }
        if(info->mKind == kObjectKindDirectory)
           dst_location_dir = true;
        hdfsFreeFileInfo(info, 1);
     } 
     if ((flags & saga::name_space::Overwrite) && dst_location_dir) {
         if (is_dst_dir)
         {
            SAGA_ADAPTOR_THROW("Can not recursive delete", saga::NoSuccess);
         }
         else
         {
            if(hdfsDelete(fs_, dst_location.string().c_str()) < 0)
               SAGA_ADAPTOR_THROW("Can not delete", saga::NoSuccess);
         }
     }
  
     // if destination still exists raise an error
     if( hdfsExists(fs_, dst_location.string().c_str()) == 0 )
     {
         SAGA_OSSTREAM strm;
         strm << "sync_copy: " << "target file already exists: " << dest.get_url();
         SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::AlreadyExists);
     }
     //Copy
     if(hdfsCopy(fs_, src_location.string().c_str(), fs_, dst_location.string().c_str()) != 0)
     {
         SAGA_ADAPTOR_THROW("Failed to copy", saga::NoSuccess);
     }
  }

  void file_cpi_impl::sync_link (saga::impl::void_t & ret,    
                                 saga::url            dest, 
                                 int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void file_cpi_impl::sync_move (saga::impl::void_t & ret,   
                                 saga::url            dest, 
                                 int                  flags)
  {
     // handle the file
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
     if( hdfsExists(fs_, src_location.string().c_str()) == 0 )
     {
        hdfsFileInfo *info;
      
        info = hdfsGetPathInfo(fs_, src_location.string().c_str());
        if(info == NULL)
        {
           SAGA_ADAPTOR_THROW("Can not get path info", saga::NoSuccess);
        }
        if(info->mKind == kObjectKindDirectory)
           is_src_dir = true;
        hdfsFreeFileInfo(info, 1);
     }
     bool is_dst_dir = false;
     if( hdfsExists(fs_, dst_location.string().c_str()) == 0 )
     {
        hdfsFileInfo *info;
       
        info = hdfsGetPathInfo(fs_, dst_location.string().c_str());
        if(info == NULL)
        {
           SAGA_ADAPTOR_THROW("Can not get path info", saga::NoSuccess);
        }
        if(info->mKind == kObjectKindDirectory)
           is_dst_dir = true;
        hdfsFreeFileInfo(info, 1);
     } 
     if (!is_src_dir && is_dst_dir)
         dst_location /= src_location.leaf();
  
     // remove the file/directory if it exists and we should overwrite
     bool dst_location_dir = false;
     if( hdfsExists(fs_, dst_location.string().c_str()) == 0 )
     {
        hdfsFileInfo *info;
       
        info = hdfsGetPathInfo(fs_, dst_location.string().c_str());
        if(info == NULL)
        {
           SAGA_ADAPTOR_THROW("Can not get path info", saga::NoSuccess);
        }
        if(info->mKind == kObjectKindDirectory)
           dst_location_dir = true;
        hdfsFreeFileInfo(info, 1);
     } 
     if ((flags & saga::name_space::Overwrite) && dst_location_dir) {
         if (is_dst_dir)
         {
            SAGA_ADAPTOR_THROW("Can not recursive delete", saga::NoSuccess);
         }
         else
         {
            if(hdfsDelete(fs_, dst_location.string().c_str()) < 0)
               SAGA_ADAPTOR_THROW("Can not delete", saga::NoSuccess);
         }
     }
  
     // if destination still exists raise an error
     if( hdfsExists(fs_, dst_location.string().c_str()) == 0 )
     {
         SAGA_OSSTREAM strm;
         strm << "sync_copy: " << "target file already exists: " << dest.get_url();
         SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::AlreadyExists);
     }
     //Copy
     if(hdfsCopy(fs_, src_location.string().c_str(), fs_, dst_location.string().c_str()) != 0)
     {
         SAGA_ADAPTOR_THROW("Failed to copy in move", saga::NoSuccess);
     }
     //Remove old
     if(hdfsDelete(fs_, src_location.string().c_str()) != 0)
     {
        SAGA_ADAPTOR_THROW("Failed to delete in move", saga::NoSuccess);
     }
  }

  void file_cpi_impl::sync_remove (saga::impl::void_t & ret,
                                   int                  flags)
  {
     instance_data idata (this);
     
     // verify current working directory is local
     saga::url url(idata->location_);
     if(hdfsDelete(fs_, url.get_path().c_str()) != 0)
     {
        SAGA_ADAPTOR_THROW("Failed to remove", saga::NoSuccess);
     }
  }


  void file_cpi_impl::sync_close (saga::impl::void_t & ret, 
                                  double               timeout)
  {
    // Nothing to close here...
  }

} // namespace

