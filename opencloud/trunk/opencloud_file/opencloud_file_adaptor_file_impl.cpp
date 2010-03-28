//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>

#include "opencloud_file_adaptor_file.hpp"

namespace opencloud_file_adaptor
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  file_cpi_impl::file_cpi_impl (proxy                * p, 
                                cpi_info       const & info,
                                saga::ini::ini const & glob_ini,
                                saga::ini::ini const & adap_ini,
                                boost::shared_ptr <saga::adaptor> adaptor)

      : file_cpi (p, info, adaptor, cpi::Noflags), 
        secf( serv_.get_sector_file_service())
  {        
    adaptor_data_t       adata (this);
    file_instance_data_t idata (this);

    /* Cache the necessary information
     */
    location =  idata->location_  ; 
    set_file_name() ; 
    s_ = p->get_session() ; 
    mode = idata->mode_ ; 


    
    /*The authentication is moved to the context adaptor
    */

    /*if( !adata->get_auth_flag() )
    {
       int auth_code = adata->authenticate( s_, location, mode, serv_, glob_ini, adap_ini ) ; 
       if( auth_code <  saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
       {
          std::string auth_error ; 
          adata->get_authentication_error( auth_error, auth_code ) ; 
          SAGA_ADAPTOR_THROW ( auth_error += " - Could not authenticate with the Sector/Sphere Master Server.", saga::NoSuccess ) ; 
       }
       adata->set_auth_flag() ; 
       auth_ = adata->auth_ ; 
    }
    else
    {
       auth_ = adata->auth_ ; 
    }   */


    /* Open the file according to the mode 
     */
    open_file() ; 

  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  file_cpi_impl::~file_cpi_impl (void)
  {
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_get_size (saga::off_t & size_out)
  {

    int res  = secf.get_file_size( size_out )   ; 
    if( res < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
    {
       std::string error ; 
       secf.get_error_msg( error, res ) ; 
       SAGA_ADAPTOR_THROW ( error , saga::NoSuccess );
    }
    return ; 
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_read (saga::ssize_t        & len_out,
                                 saga::mutable_buffer   data,
                                 saga::ssize_t          len_in)
  {

    if( len_in < 0 || len_in < data.get_size() ){
       SAGA_ADAPTOR_THROW ("Sector/Sphere: Incorrect buffer size specified in len_in", saga::BadParameter ) ; 
    }
    if( data.get_size() <  0 ){
       SAGA_ADAPTOR_THROW ("Sector/Sphere: Buffer does not have memory to hold data", saga::BadParameter ) ; 
    }

    /* Read the data
     */
    len_out = secf.read( (char*)data.get_data() , len_in ) ; 
    if( len_out < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
    {
       std::string error ; 
       secf.get_error_msg( error , len_out ) ; 
       SAGA_ADAPTOR_THROW ( error , saga::NoSuccess );
    }

    return  ; 
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_write (saga::ssize_t      & len_out, 
                                  saga::const_buffer   data,
                                  saga::ssize_t        len_in)
  {
    
    if( len_in < 0 || len_in < data.get_size() ){
       SAGA_ADAPTOR_THROW ("Sector/Sphere: Incorrect buffer size specified in len_in", saga::BadParameter ) ; 
    }

    char * c_buf = new char [len_in] ; 
    if( data.get_size() < 0 )
    {
       SAGA_ADAPTOR_THROW ("Sector/Sphere: Buffer does not have anything to write. ", saga::BadParameter ) ; 
    }
    
    memset( c_buf, 0, len_in ) ; 
    memcpy( c_buf, data.get_data(), len_in ) ; 

    len_out = secf.write( (const char*) c_buf, len_in ) ; 
    delete c_buf ; 

    if( len_out < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
    {
       std::string error ; 
       secf.get_error_msg( error, len_out ) ; 
       SAGA_ADAPTOR_THROW ( error , saga::NoSuccess );
    }

    return ; 

  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_seek (saga::off_t                 & out, 
                                 saga::off_t                   offset, 
                                 saga::filesystem::seek_mode   whence)
  {


    int res  = secf.seek( (int64_t)offset, whence, (int64_t&)out ) ; 

    if( res  < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
    {
       std::string error ; 
       secf.get_error_msg( error, res ) ; 
       SAGA_ADAPTOR_THROW ( error , saga::NoSuccess );
    }

    return ; 
  }


  bool file_cpi_impl::open_file()
  {
     int rc = secf.open( filename_, mode ) ; 
     if( rc < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        std::string error ; 
        secf.get_error_msg( error , rc ) ; 
        SAGA_ADAPTOR_THROW( error, saga::NoSuccess ) ; 
     }

     return true ; 
  }


  void file_cpi_impl::set_file_name ()
  {

    // can we handle this type of URL ?
    if ( location.get_scheme () != "any" &&
         location.get_scheme () != "sector" &&
         location.get_scheme () != ""    )
    {
      SAGA_LOG_DEBUG  (location.get_string ().c_str ());
      SAGA_ADAPTOR_THROW_NO_CONTEXT ("Cannot handle URL schema", saga::BadParameter);
    }

    resolve( location, filename_) ; 

    return ; 

  }

  void file_cpi_impl::resolve( saga::url const &url_, std::string & ret )
  {
    
    if( (! ( url_.get_path()).empty() ) &&
             url_.get_path() != "/" )
    {
        ret = url_.get_path() ; 
        return ; 
    }
    else
    {
       ret = url_.get_host() ; 
    }

  }


} // namespace

