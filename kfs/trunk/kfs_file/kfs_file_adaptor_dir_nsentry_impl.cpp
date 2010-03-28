//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>

#include "kfs_file_adaptor_dir.hpp"

namespace kfs_file_adaptor
{
  void dir_cpi_impl::sync_get_url (saga::url & url) {
       saga::url u;
       instance_data data (this);
       u = data->location_;
       if (u.get_scheme().empty())
           u.set_scheme("hdfs");
       if (u.get_host().empty())
           u.set_host("localhost");
       url = u.get_url();
       //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void dir_cpi_impl::sync_get_cwd (saga::url & cwd) {
     instance_data idata (this);
     saga::url result(idata->location_);
     boost::filesystem::path name (result.get_path(), boost::filesystem::native);
     result.set_path(name.branch_path().string());
     // complete url
     if (result.get_scheme().empty())
         result.set_scheme("hdfs");
     if (result.get_host().empty())
         result.set_host("localhost");
     cwd = result;
     //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void dir_cpi_impl::sync_get_name (saga::url & name) {
     instance_data idata (this);
     saga::url current_url(idata->location_);
     saga::url u (idata->location_.get_path());
     boost::filesystem::path myname (u.get_path(), boost::filesystem::native);
     name = myname.leaf();
     //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void dir_cpi_impl::sync_is_dir (bool & is_dir) {
      instance_data idata (this);
      is_dir = false;
      
      saga::url url(idata->location_);
      boost::filesystem::path path (idata->location_.get_path(), boost::filesystem::native);
      if(fs_->Exists(path.string().c_str())) {
         //Check to see if it is a directory
         instance_data idata(this);
         if(fs_->IsDirectory(path.string().c_str()))
            is_dir = true;
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void dir_cpi_impl::sync_is_entry (bool & is_entry) {
      instance_data idata (this);
      is_entry = false;
      
      saga::url url(idata->location_);
      boost::filesystem::path path (idata->location_.get_path(), boost::filesystem::native);
      if(fs_->Exists(path.string().c_str())) {
         //Check to see if it is a directory
         instance_data idata(this);
         if(fs_->IsFile(path.string().c_str()))
            is_entry = true;
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_link (bool & is_link)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_read_link (saga::url & target)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_link (saga::impl::void_t & ret, 
                                saga::url            dest, 
                                int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_copy (saga::impl::void_t & ret, 
                                saga::url            dest, 
                                int                  flags)
  {    
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_move (saga::impl::void_t & ret, 
                                saga::url            dest, 
                                int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_remove (saga::impl::void_t & ret, 
                                  int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_close (saga::impl::void_t & ret, 
                                 double               timeout)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

} // namespace

