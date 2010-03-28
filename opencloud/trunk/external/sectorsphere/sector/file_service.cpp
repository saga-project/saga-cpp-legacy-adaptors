//  Copyright (c) 2005-2007 Saurabh Sehgal ( saurabh.r.s@gmail.com )
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)


#include <saga/saga.hpp>
#include <fsclient.h>
#include <dcclient.h>
#include <util.h>
#include <iostream>
#include "file_service.hpp"

namespace saga_sector
{

   const char  file_service::mv       []   = "mv" ; 
   const char  file_service::upload   []   = "upload" ; 
   const char  file_service::download []   = "download" ; 
   const char  file_service::mkdir    []   = "mkdir"  ; 
   const char  file_service::cp       []   = "cp" ; 
   const char  file_service::rm       []   = "rm" ; 

   file_service::file_service(void) 
    :  path (""), 
       op( false ),
       mode (-1), 
       saga_mode (-1), 
       tools_path("")
   {

   }
   
   file_service::~file_service(void) 
   {
      if( op ) close() ; 
   }

   file_service::file_service( std::string const &filename )
   :   path (filename), 
       op( false ),
       mode (-1), 
       saga_mode (-1), 
       tools_path("")
   {

   }

   void file_service::set_file_name ( std::string const & name )
   {
      path = name ; 
   }

   int file_service::open( std::string filename , int smode )
   {

      int error = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 

      if( op && saga_mode == smode )
      {
         return error ; 
      }

      if( saga_mode != smode  && op && saga_mode != -1 )
      {
         close() ; 
      }


      switch( smode )
      {
         case saga::filesystem::Read:
            mode = SF_MODE::READ ; 
            break  ;

         case saga::filesystem::Write:
            mode = SF_MODE::WRITE ; 
            break ; 

         case saga::filesystem::ReadWrite:
            mode = SF_MODE::RW ; 
            break ; 
       
         case saga::filesystem::Truncate:
            // Not yet supported !
            error = saga_sectorsphere::error::SAGA_SECSP_E_NOT_SUPPORTED ; 
            return error ; 
            break  ; 

         case saga::filesystem::Append:
            mode = SF_MODE::APPEND ; 
            break ;

         case saga::filesystem::Overwrite:
            // Not yet supported ! 
            error = saga_sectorsphere::error::SAGA_SECSP_E_NOT_SUPPORTED ;
            return error ; 
            break ; 

        default:
            return saga_sectorsphere::error::SAGA_SECSP_E_NOT_SUPPORTED ; 


      }

      error = sfile.open( filename, mode ) ; 

      if( error <  saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
      {
        return error ; 
      }

      path = filename ; 
      saga_mode = smode ; 
      op = true ; 

      return error ; 
   }

  void file_service::close( )
  {

    if( path.empty() )
    {
        return  ; 
    }

    sfile.close() ; 
    op = false ; 
    saga_mode = -1 ; 
    mode = -1 ; 
   
  }

  int file_service::read( char * buffer, saga::ssize_t len )
  {
     int bytes_read = 0 ; 
     int error = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 
     if( path.empty() )
     {
        error = saga_sectorsphere::error::SAGA_SECSP_E_BAD_ARGS ; 
        return error ; 
     }

     bytes_read = sfile.read( buffer, (const boost::int64_t) len )  ; 

     return bytes_read ; 

  }


  int file_service::write( const char * buffer, saga::ssize_t len )
  {
     int bytes_written = 0 ; 
     int error = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 

     if( path.empty() )
     {
        error = saga_sectorsphere::error::SAGA_SECSP_E_BAD_ARGS ; 
        return error ; 
     }

     bytes_written = sfile.write( buffer, (const boost::int64_t) len ) ; 
     return bytes_written ; 
  }


  
  int file_service::get_file_size( saga::off_t &size )
  {
      int error = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 
      if( path.empty() )
      {
         return saga_sectorsphere::error::SAGA_SECSP_E_BAD_ARGS ; 
      }

      SNode attr ; 
      error = Sector::stat( path , attr )  ; 

      if( error < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ) 
      {
         size = -1 ; 
         return error ; 
      }

      size = (saga::off_t) attr.m_llSize ; 
      return error ; 
  }

  int file_service::is_dir( bool &is_dir )
  {
     int error = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 
     if( path.empty() )
     {
        return error ; 
     }

     SNode attr ; 
     error = Sector::stat( path , attr ) ;
     if( error < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        return error ; 
     }

     is_dir  = attr.m_bIsDir ; 
     return error ; 
  }



  int file_service::seek( int64_t offset, saga::filesystem::seek_mode w, int64_t &out )
  {

      int error = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 

      switch( w )
      {
          case saga::filesystem::Current:
             whence = SF_POS::CUR ; 
             break ; 
           
          case saga::filesystem::Start:
             whence = SF_POS::BEG ; 
             break ; 
             
          case saga::filesystem::End:
             whence = SF_POS::END ; 
             break ; 

          default: 
             whence = SF_POS::CUR ; 
      }


      if( mode == SF_MODE::READ || mode == SF_MODE::RW )
      {
         error = sfile.seekg( (boost::int64_t) offset, whence ) ; 
         if( error < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
         {
            return error ; 
         }
         out = (saga::off_t) sfile.tellg() ; 
      }
      else
      {
         error = sfile.seekp( (boost::int64_t) offset, whence ) ; 
         if( error < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
         {
            return error ; 
         }
         out = (saga::off_t) sfile.tellp() ; 
      } 

      return error ; 
  }



  int file_service::copy ( std::string dest )
  {

     // Will not use the utilities anymore .. 
     // They relogin into the server, which sometimes 
     // causes problems (as far as I have seen), and
     // if the user logged in through other means 
     // through SAGA, incorrect information will be used
     // to authenticate
     
     /*std::string exec = tools_path ; 
     exec += cp ; 
     std::vector <std::string> args ; 
     args.push_back( path ) ; 
     args.push_back( dest ) ; 
     bool res  = execute_command( exec, args, err ) ; */
     
     int error = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 
     bool wc_s = false ;  
     bool wc_d = false ;  

     std::vector <std::string> filtered_dest ; 
     std::vector <std::string> filtered_src ; 

     error  = get_wildcard_entries( filtered_dest, dest , wc_d ) ; 
     if( error < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        return error ; 
     }

     error  = get_wildcard_entries( filtered_src, path, wc_s  ) ; 
     if( error < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        return error ; 
     }

     if (!wc_s && !wc_d)
     {
        error = Sector::copy( path, dest ) ; 
        if( error < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
        {
           return error ; 
        }
     }

     if( wc_d ){

        for ( std::vector<std::string>::iterator i = filtered_dest.begin(); i != filtered_dest.end(); ++ i){
            error = Sector::copy( path, *i ) ;  
	    if( error < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ){
	       return error ; 
	    }
        }
     }

     return error ; 
  }


  int file_service::get_wildcard_entries( std::vector <std::string> &filtered , std::string str , bool &wc )
  {

     int error = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 

     wc = WildCard::isWildCard( str ) ; 
     if( !wc ) return error ; 

     std::string orig = str ; 

     // Find the last directory in the path hierarchy
     size_t p = str.rfind('/');
     if (p == std::string::npos)
           str  = "/";
     else
     {
         str  = path.substr(0, p);
         orig = orig.substr(p + 1, orig.length() - p);
     }


     // List the directory and see which pattern matches 
     std::vector<SNode> filelist;
     error  = Sector::list(path, filelist);
     if ( error <  saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        return error ; 
     }
         
     for ( std::vector<SNode>::iterator i = filelist.begin(); i != filelist.end(); ++ i)
     {
         if (WildCard::match(orig, i->m_strName))
            filtered.push_back(str + "/" + i->m_strName);
     }

     return error ; 

  }

  int file_service::move( std::string dest )
  {

     /*std::string exec = tools_path ; 
     exec += mv ; 

     std::vector <std::string> args ; 
     args.push_back( path ) ; 
     args.push_back( dest ) ; 
     bool res  = execute_command( exec, args, err ) ; 
     return res ; */

     bool wc_s = false ; 
     bool wc_d = false ; 

     int error = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 

     std::vector <std::string> filtered_dest ; 
     std::vector <std::string> filtered_src ; 

     error = get_wildcard_entries( filtered_dest, dest, wc_d ) ; 
     if ( error <  saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        return error ; 
     }

     error = get_wildcard_entries( filtered_src, path, wc_s  ) ; 
     if ( error <  saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        return error ; 
     }

     if (!wc_s && !wc_d)
     {
        error = Sector::move(path, dest ) ; 
        if ( error <  saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
        {
           return error ; 
        }
     }

     return error ; 
  }

  int file_service::remove( void )
  {

     /*std::string exec = tools_path ; 
     exec += rm ; 

     std::vector <std::string> args ; 
     args.push_back( path ) ; 
     bool res  = execute_command( exec, args, err ) ; 

     return res ; */
     bool wc = false ; 
     
     int error = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 

     std::vector <std::string> filtered ; 

     error = get_wildcard_entries( filtered, path, wc ) ; 
     if ( error <  saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        return error ; 
     }

     if ( !wc )
     {
        error = Sector::remove(path) ; 
        if ( error <  saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
        {
           return error ; 
        }
     }

     return error ; 

  }

  bool file_service::execute_command ( std::string exec, std::vector <std::string> args, std::string &err )
  {
     
     int ecode ; 

     saga::adaptors::utils::process proc ( exec , args ) ; 
     proc.run_sync() ; 


     ecode = proc.exitcode() ; 

     if( 1 == ecode )
     {
        // Success !
        return true ; 
     }
     
     // Trouble ..
     // See if we have something on stdout ...
     err = proc.get_out_s() ; 
     return false ; 

  }

  void file_service::set_tools_path( std::string tools ) 
  {
     tools_path = tools ;  
  } 

  void file_service::get_error_msg( std::string & err, int const &code )
  {
     saga_sectorsphere::error::get_err_msg( err, code ) ; 
     return ; 
  }

}

