//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>

#include "opencloud_file_adaptor_dir.hpp"

namespace opencloud_file_adaptor
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_change_dir (saga::impl::void_t & ret, 
                                      saga::url            name)
  {
    
    std::string temp ; 
    int res = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS  ; 

    if( resolve( name, temp ) )
    {

       /* Relative path, construct the full string
        */
       std::string cd ( dir_ ) ; 
       cd += '/' ; 
       cd += temp ; 

       res = dserv_.open( cd , mode )  ; 
       if( res < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
       {
          std::string err_msg ; 
          dserv_.get_error_msg( err_msg, res ) ; 
          SAGA_ADAPTOR_THROW ( err_msg , saga::NoSuccess); 
          return ; 
       }

       location.set_url( cd ) ; 
       dir_ = cd ; 
       return ; 

    }

    /* Save the previous location
     */
    saga::url save = location ; 
    std::string save_d = dir_ ; 
    location = name ; 
    dir_ = temp ; 

    std::string error ("Sector/Sphere: Error in changing directory - ") ; 
    res = open_dir( error ) ; 
    if( res ==  saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
       return ; 
    else
    {

       /* Record the error
        */
       dserv_.get_error_msg( error , res ) ; 

       /* Fall back .. 
        */
       location = save ; 
       dir_ = save_d ; 
       dserv_.close() ; 
       std::string error_reopen ("Sector/Sphere: Cannot re-open original directory. Fatal.") ; 

       if( !open_dir( error_reopen ) )
       {
          SAGA_ADAPTOR_THROW ( error_reopen , saga::NoSuccess); 
       }
       else
       {
          SAGA_ADAPTOR_THROW ( error , saga::NoSuccess); 
       }

       return ; 
    }

  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_list (std::vector <saga::url> & list, 
                                std::string               pattern, 
                                int                       flags)
  {
      int res = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS  ; 
      dserv_.list( list, pattern ) ; 
      if( res < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
      {
          std::string error ( "Sector/Sphere: Error in listing directory - ") ; 
          dserv_.get_error_msg( error , res ) ; 
          SAGA_ADAPTOR_THROW ( error , saga::NoSuccess); 
      }

      return ; 
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_find (std::vector <saga::url> & list, 
                                std::string               entry, 
                                int                       flags)
  {
    // FIXME: Should this be finding in current dir or also search through subdirectories ?
    // If the latter, a recursive solution might not be the best for deeply nested dirs 
    // List is not working right now ; Will implement this once List is figured out with Sec/Sph
    // people ... 
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_exists (bool    & exists, 
                                  saga::url url)
  {
     int rc = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS  ; 
     std::string res ; 
     if( resolve( url, res ) )
     {
        std::string test (dir_) ; 
        test += '/' ; 
        test += res ; 
        res = test ; 
     }

     saga_sector::dir_service ds ; 
     rc = ds.open( res, saga::filesystem::Read  ) ; 
     if( rc ==  saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {  
        ds.close() ; 
        exists = true ; 
        return  ; 
     }

     exists = false ; 
     
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_dir (bool    & is_dir, 
                                  saga::url url)
  {
     std::string res ; 
     int err = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS  ; 

     if( resolve( url, res ) )
     {
        std::string test (dir_) ; 
        test += '/' ; 
        test += res ; 
        res = test ; 
     }

     saga_sector::file_service fs ; 
     if( err = fs.open( res, saga::filesystem::Read ) == saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        fs.is_dir( is_dir ) ; 
        fs.close() ; 
        return  ; 
     }
     else
     {
        std::string error ("Sector/Sphere: Unable to locate requested file - ") ; 
        fs.get_error_msg( error, err ) ; 
        SAGA_ADAPTOR_THROW( error , saga::NoSuccess ) ; 
     }
      
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_entry (bool    & is_entry, 
                                    saga::url url)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_link (bool    & is_link, 
                                   saga::url url)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented in Sector/Sphere", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_read_link (saga::url & ret, 
                                     saga::url   source)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented in Sector/Sphere", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_get_num_entries (std::size_t & num)
  {
     dserv_.get_num_entries( num ) ; 
     return  ; 
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_get_entry (saga::url & ret, 
                                     std::size_t entry )
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_link (saga::impl::void_t & ret, 
                                saga::url            source, 
                                saga::url            url, 
                                int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented in Sector/Sphere", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_copy (saga::impl::void_t & ret, 
                                saga::url            src, 
                                saga::url            dst, 
                                int                  flags)
  {
     std::string src_res ; 
     std::string dst_res ; 
     int rc  = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS  ; 

     if( resolve( src, src_res ))
     {
        std::string test (dir_) ; 
        test += '/' ; 
        test += src_res ; 
        src_res = test ; 
     }

     if( resolve( dst, dst_res ))
     {
        std::string test (dir_) ; 
        test += '/' ; 
        test += dst_res ; 
        dst_res = test ; 
     }

     std::string err ; 
     saga_sector::file_service fserv_ ( src_res ) ; 
     rc = fserv_.copy( dst_res ) ; 
     if( rc < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        err += " Sector/Sphere: Error in copying file - " ; 
	fserv_.get_error_msg( err, rc ) ; 
        SAGA_ADAPTOR_THROW ( err, saga::NoSuccess);
     }

  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_move (saga::impl::void_t & ret, 
                                saga::url            src, 
                                saga::url            dest, 
                                int                  flags)
  {
     std::string src_res ; 
     std::string dst_res ; 
     int rc  = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS  ; 

     if( resolve( src, src_res ))
     {
        std::string test (dir_) ; 
        test += '/' ; 
        test += src_res ; 
        src_res = test ; 
     }

     if( resolve( dest, dst_res ))
     {
        std::string test (dir_) ; 
        test += '/' ; 
        test += dst_res ; 
        dst_res = test ; 
     }

     std::string err ; 
     saga_sector::file_service fserv_ ( src_res ) ; 
     if( rc = fserv_.move( dst_res ) < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        fserv_.get_error_msg( err, rc ) ; 
        SAGA_ADAPTOR_THROW ( err, saga::NoSuccess);
     }
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_remove (saga::impl::void_t & ret, 
                                  saga::url            url, 
                                  int                  flags)
  {
     std::string res ; 
     std::string err ; 
     int rc  = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS  ; 

     if( resolve( url, res ) )
     {
        std::string test (dir_) ; 
        test += '/' ; 
        test += res ; 
        res = test ; 
     }

     saga_sector::file_service fs ( res ) ; 
     if( rc = fs.remove( ) < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        fs.get_error_msg( err, rc ) ; 
        SAGA_ADAPTOR_THROW( err , saga::NoSuccess ) ; 
        return  ; 
     }
   
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_copy_wildcard (saga::impl::void_t & ret, 
                                         std::string          source, 
                                         saga::url            dest, 
                                         int                  flags)
  {
     return (sync_copy( ret, source, dest, flags ) ) ; 
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_link_wildcard (saga::impl::void_t & ret, 
                                         std::string          source, 
                                         saga::url            dest, 
                                         int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_move_wildcard (saga::impl::void_t & ret, 
                                         std::string          source, 
                                         saga::url            dest, 
                                         int                  flags)
  {
     return ( sync_move( ret, source, dest, flags )) ; 
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_remove_wildcard (saga::impl::void_t & ret, 
                                           std::string          url, 
                                           int                  flags)
  {
     return ( sync_remove( ret, url, flags ) ) ; 
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_make_dir (saga::impl::void_t & ret, 
                                    saga::url            url, 
                                    int                  flags)
  {
     
     std::string res ; 
     int rc  = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS  ; 

     if( resolve( url, res ) )
     {
        std::string test (dir_) ; 
        test += '/' ; 
        test += res ; 
        res = test ; 
     }

     if( rc = dserv_.mkdir( res ) < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        std::string error  ; 
        dserv_.get_error_msg( error, rc ) ; 
        SAGA_ADAPTOR_THROW( error , saga::NoSuccess ) ; 
     }

     return ; 

  }

} // namespace

