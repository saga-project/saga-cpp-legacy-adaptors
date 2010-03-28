//  Copyright (c) 2008 Chris Miceli <cmicel1@cct.lsu.edu>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/exception.hpp>

#include "hdfs_file_adaptor_dir.hpp"
#include "hdfs_file_adaptor_file.hpp"

namespace hdfs_file_adaptor
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  dir_cpi_impl::dir_cpi_impl (proxy                * p, 
                              cpi_info       const & info,
                              saga::ini::ini const & glob_ini,
                              saga::ini::ini const & adap_ini,
                              boost::shared_ptr<saga::adaptor> adaptor)

      : directory_cpi (p, info, adaptor, cpi::Noflags)
  {
    adaptor_data_t            adata (this);
    directory_instance_data_t idata (this);

    saga::url dir_url(idata->location_);

    std::string host(dir_url.get_host());
    if (host.empty())
    {
        SAGA_OSSTREAM strm;
        strm << "dir_cpi_impl::init: cannot handle file: " << dir_url.get_url();
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
    }

    std::string scheme(dir_url.get_scheme());
    if (!scheme.empty() && scheme != "hdfs" && scheme != "any")
    {
       SAGA_OSSTREAM strm;
       strm << "dir_cpi_impl::init: cannot handle file: " << dir_url.get_url();
       SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
    }

    fs_ = hdfsConnect(dir_url.get_host().c_str(), static_cast<tPort>(dir_url.get_port()));
    if(fs_ == NULL)
    {
          SAGA_OSSTREAM strm;
          strm << "Could not connect to host : " << dir_url.get_host();
          strm << "on port " << dir_url.get_port();
          SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::NoSuccess);
    }
    
    // check if file exists AND is a dir (not a file)
    bool exists = false;
    bool is_dir = false;
    if(hdfsExists(fs_, dir_url.get_path().c_str()) == 0)
    {
       exists = true;
       //Check to see if it is a directory
       hdfsFileInfo *info;
       instance_data idata(this);

       info = hdfsGetPathInfo(fs_, dir_url.get_path().c_str());
       if(info == NULL)
       {
          SAGA_ADAPTOR_THROW("file_cpi_impl::init failed",
                           saga::NoSuccess);
       }
       if(info->mKind == kObjectKindDirectory)
          is_dir = true;
       hdfsFreeFileInfo(info, 1);
     }
     saga::filesystem::flags OpenMode = (saga::filesystem::flags)idata->mode_;
     if(exists)
     {
        if(!is_dir)
        {
           SAGA_ADAPTOR_THROW ("URL does not point to a directory: " +
                               idata->location_.get_url(), saga::BadParameter);
        }
        else
        {
           if((OpenMode & saga::filesystem::Create) && (OpenMode & saga::filesystem::Exclusive))
           {
               SAGA_ADAPTOR_THROW ("Directory " + idata->location_.get_url() +
                                   " already exists.", saga::AlreadyExists);
           }
        }
     }
     else // !exists
     {
        if(!(OpenMode & saga::filesystem::Create))
        {
           SAGA_ADAPTOR_THROW ("Directory does not exist and saga::filesystem::Create flag not given: " +
                               idata->location_.get_url(), saga::DoesNotExist);
        }
        else
        {
           if(hdfsCreateDirectory(fs_, dir_url.get_path().c_str()) != 0)
           {
              SAGA_OSSTREAM strm;
              strm << "Could not create directory " << idata->location_.get_path();
              SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::DoesNotExist);
           }
        }
     }
     exists = false;
     is_dir = false;
     //Make sure directory exists
     if(hdfsExists(fs_, dir_url.get_path().c_str()) == 0)
        exists = true;
     if ((idata->mode_ & saga::filesystem::Create ||
          idata->mode_ & saga::filesystem::CreateParents) &&
          idata->mode_ & saga::filesystem::Exclusive)
     {
        //Check to see if it is a directory
        hdfsFileInfo *info;
      
        info = hdfsGetPathInfo(fs_, dir_url.get_path().c_str());
        if(info == NULL)
        {
           SAGA_ADAPTOR_THROW("file_cpi_impl::init failed",
                            saga::NoSuccess);
        }
        if(info->mKind == kObjectKindDirectory)
           is_dir = true;
        hdfsFreeFileInfo(info, 1);
        if(is_dir)
        {
           SAGA_ADAPTOR_THROW(dir_url.get_path().c_str() + ": already exists",
              saga::AlreadyExists);
        }
     }
     if(!exists)
     {
        //create directory if needed
       if (idata->mode_ & saga::filesystem::Create) {
          if(hdfsCreateDirectory(fs_, dir_url.get_path().c_str()) != 0)
          {
             SAGA_OSSTREAM strm;
             strm << "Could not create directory " << idata->location_.get_path();
             SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::DoesNotExist);
          }
       }
     }
    
     // we don't need to create the directory twice
     idata->mode_ &= ~(saga::filesystem::Create | saga::filesystem::CreateParents);
    
     if (idata->mode_ & saga::filesystem::Read || idata->mode_ & saga::filesystem::Write ||
         idata->mode_ & saga::filesystem::ReadWrite)
     {
        //check to see if it exists and is a direcotory
       exists = false;
       is_dir = false;
       if(hdfsExists(fs_, dir_url.get_path().c_str()) == 0)
       {
          exists = true;
          //Check to see if it is a directory
          hdfsFileInfo *info;
     
          info = hdfsGetPathInfo(fs_, dir_url.get_path().c_str());
          if(info == NULL)
          {
             SAGA_ADAPTOR_THROW("file_cpi_impl::init failed",
                              saga::NoSuccess);
          }
          if(info->mKind == kObjectKindDirectory)
             is_dir = true;
          hdfsFreeFileInfo(info, 1);
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
           ))
     {
       SAGA_ADAPTOR_THROW("Unknown openmode value: " +
           boost::lexical_cast<std::string>(idata->mode_), saga::BadParameter);
     }
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  dir_cpi_impl::~dir_cpi_impl (void)
  {
     hdfsDisconnect(fs_);
  }

  void dir_cpi_impl::sync_get_size (saga::off_t & size_out, 
                                    saga::url      name, int flag)
  {
     SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
    /* hdfsFileInfo *info;
     instance_data idata(this);

     info = hdfsGetPathInfo(fs_, (idata->location).get_path());
     if(info == NULL)
     {
        SAGA_ADAPTOR_THROW("file_cpi_impl::sync_get_size: failed",
                           saga::NoSuccess);
     }
     size_out = (saga::off_t)info->mSize;
     hdfsFreeFileInfo(info, 1);*/
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_open (saga::filesystem::file & ret, 
                                saga::url                name_to_open, 
                                int                      openmode)
  {
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

     if(hdfsExists(fs_, path.string().c_str()) == 0)
     {
        exists = true;
        //Check to see if it is a directory
        hdfsFileInfo *info;
     
        info = hdfsGetPathInfo(fs_, path.string().c_str());
        if(info == NULL)
        {
           SAGA_ADAPTOR_THROW("file_cpi_impl::init failed",
                            saga::NoSuccess);
        }
        if(info->mKind == kObjectKindDirectory)
           is_dir = true;
        hdfsFreeFileInfo(info, 1);
     }
     if ( exists && is_dir ) {
       SAGA_ADAPTOR_THROW(path.string() + ": doesn't refer to a file object",
         saga::DoesNotExist);
     }
    
     // is_entry # FIXME: what? (also below...) -- AM 
     ret = saga::filesystem::file (this->get_proxy()->get_session(), file_url.get_url(), openmode);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_open_dir (saga::filesystem::directory & ret, 
                                    saga::url                     name_to_open, 
                                    int                           openmode)
  {
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
     if(hdfsExists(fs_, path.string().c_str()) == 0)
     {
        exists = true;
        //Check to see if it is a directory
        hdfsFileInfo *info;
     
        info = hdfsGetPathInfo(fs_, path.string().c_str());
        if(info == NULL)
        {
           SAGA_ADAPTOR_THROW("file_cpi_impl::init failed",
                            saga::NoSuccess);
        }
        if(info->mKind == kObjectKindDirectory)
           is_dir = true;
        hdfsFreeFileInfo(info, 1);
     }
     if ( exists && !is_dir) {
       SAGA_ADAPTOR_THROW(path.string() + ": doesn't refer to a file object",
         saga::DoesNotExist);
     }
    
     ret = saga::filesystem::directory (this->get_proxy()->get_session(), file_url.get_url(),
                              openmode);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_file (bool    & is_file, 
                                   saga::url name)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

} // namespace

