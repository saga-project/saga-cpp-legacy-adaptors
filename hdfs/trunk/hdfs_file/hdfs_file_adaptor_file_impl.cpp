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
  file_cpi_impl::file_cpi_impl (proxy                * p, 
                                cpi_info       const & info,
                                saga::ini::ini const & glob_ini,
                                saga::ini::ini const & adap_ini,
                                boost::shared_ptr <saga::adaptor> adaptor)

      : file_cpi (p, info, adaptor, cpi::Noflags)
  {        
    adaptor_data_t       adata (this);
    file_instance_data_t idata (this);

    saga::url file_url(idata->location_);
    std::string url = file_url.get_path();

    std::string host(file_url.get_host());
    if(host.empty())
    {
       SAGA_ADAPTOR_THROW("No host supplied", saga::IncorrectURL);
    }

    std::string scheme(file_url.get_scheme());
    if(!scheme.empty() && scheme != "hdfs" && scheme != "any")
    {
       SAGA_ADAPTOR_THROW("Cannot handle scheme other than \"hdfs\" or \"any\"", saga::IncorrectURL);
    }

    fs_ = hdfsConnect(file_url.get_host().c_str(), static_cast<tPort>(file_url.get_port()));
    if(fs_ == NULL)
    {
          SAGA_OSSTREAM strm;
          strm << "Could not connect to host : " << file_url.get_host();
          strm << "on port " << file_url.get_port();
          SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::NoSuccess);
    }
    saga::filesystem::flags mode = (saga::filesystem::flags)idata->mode_;

    //Create file if saga::filesystem::Create flag is given and file does not exist
    if( !(mode & saga::filesystem::Create) && hdfsExists(fs_, url.c_str()) != 0 )
    {
       SAGA_ADAPTOR_THROW("File does not exists and Create flag not given", saga::BadParameter);
    }
    if (idata->mode_ & saga::filesystem::Read && idata->mode_ & saga::filesystem::Write)
    {
       SAGA_ADAPTOR_THROW("Hadoop FS does not support ReadWrite mode", saga::NoSuccess);
    }
    if (idata->mode_ & saga::filesystem::Append)
    {
       SAGA_ADAPTOR_THROW("Hadoop FS does not support append mode yet", saga::NoSuccess);
    }
    if (idata->mode_ & saga::filesystem::Read && !(idata->mode_ & saga::filesystem::Write))
    {
       file_ = hdfsOpenFile(fs_, url.c_str(), O_RDONLY, 1, 0, 0);
       if(file_ == NULL)
       {
          SAGA_OSSTREAM strm;
          strm << "Could not open file: " << idata->location_;
          SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::DoesNotExist);
       }
    }
    else if (idata->mode_ & saga::filesystem::Write && !(idata->mode_ & saga::filesystem::Read))
    {
       file_ = hdfsOpenFile(fs_, url.c_str(), O_WRONLY, 1, 0, 0);
       if(file_ == NULL)
       {
          SAGA_OSSTREAM strm;
          strm << "Could not open file: " << idata->location_;
          SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::DoesNotExist);
       }
    }
    if(idata->mode_ & saga::filesystem::Create)
       idata->mode_ &= ~saga::filesystem::Create; // no need to create this file twice
    if (0 != idata->pointer_)
    {
       if(hdfsSeek(fs_, file_, (tOffset)idata->pointer_) != 0)
       {
          SAGA_OSSTREAM strm;
          strm << "Could not seek in file: " << idata->location_;
          SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::NoSuccess);
       }
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  file_cpi_impl::~file_cpi_impl (void)
  {
     hdfsCloseFile(fs_, file_);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_get_size (saga::off_t & size_out)
  {
     hdfsFileInfo *info;
     instance_data idata(this);

     info = hdfsGetPathInfo(fs_, (idata->location_).get_path().c_str());
     if(info == NULL)
     {
        SAGA_ADAPTOR_THROW("file_cpi_impl::sync_get_size: failed",
                           saga::NoSuccess);
     }
     size_out = (saga::off_t)info->mSize;
     hdfsFreeFileInfo(info, 1);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_read (saga::ssize_t        & len_out,
                                 saga::mutable_buffer   data,
                                 saga::ssize_t          len_in)
  {
     // validate parameters
     if (len_in < 0)
     {
         SAGA_ADAPTOR_THROW(
             "file_cpi_impl::sync_read: 'len_in' should not be negative",
             saga::BadParameter);
     }
     if (data.get_size() != -1 && len_in > data.get_size())
     {
         SAGA_ADAPTOR_THROW(
             "file_cpi_impl::sync_read: buffer too small for data to read",
             saga::BadParameter);
     }
     {
        instance_data idata(this);
        hdfsSeek(fs_, file_, idata->pointer_);
     }
     // initialize buffer
     if (-1 == data.get_size())
     {
         data.set_size(len_in);    // use implementation managed memory
     }
  
     // calculate length of rest of the file
//     mutex_type::scoped_lock lock(mtx_);
     len_out = hdfsRead(fs_, file_, (void*)data.get_data(), len_in);
     {
        instance_data idata(this);
        idata->pointer_ += (saga::off_t) len_out;
     }
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_write (saga::ssize_t      & len_out, 
                                  saga::const_buffer   data,
                                  saga::ssize_t        len_in)
  {
    
     // validate parameters
     if (len_in < 0)
     {
         SAGA_ADAPTOR_THROW(
             "file_cpi_impl::sync_write: 'len_in' should not be negative",
             saga::BadParameter);
     }
    
     // write data 
     mutex_type::scoped_lock lock(mtx_);
    
     {
        instance_data idata(this);
        hdfsSeek(fs_, file_, idata->pointer_);
     }
     tSize result = hdfsWrite(fs_, file_, (void*)data.get_data(), (tSize)data.get_size());
     if(result < 0)
     {
        SAGA_OSSTREAM strm;
        instance_data data (this);
        strm << "file_cpi_impl::write: could not write to file: "
             << data->location_.get_url();
       SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm),
           saga::adaptors::Unexpected);

     }
     {
       instance_data idata (this);
       len_out             = data.get_size();
       idata->pointer_ += (saga::off_t)   len_out;
     }
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_seek (saga::off_t                 & out, 
                                 saga::off_t                   offset, 
                                 saga::filesystem::seek_mode   whence)
  {
    if(file_ == NULL)
    {
          SAGA_OSSTREAM strm;
          strm << "File not open file";
          SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::DoesNotExist);
    }
    mutex_type::scoped_lock lock(mtx_);
    instance_data idata(this);

    switch ( (saga::filesystem::seek_mode) whence )
    {
       case saga::filesystem::Start:
          if(hdfsSeek(fs_, file_, (tOffset)offset) != 0)
          {
             SAGA_OSSTREAM strm;
             strm << "Could not seek in file: " << idata->location_;
             SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::NoSuccess);
          }
          break;
       case saga::filesystem::Current:
          if(hdfsSeek(fs_, file_, (tOffset)idata->pointer_ + (tOffset)offset) != 0)
          {
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
    idata->pointer_ = (saga::off_t)hdfsTell(fs_, file_);
    out = idata->pointer_;
  }

} // namespace

