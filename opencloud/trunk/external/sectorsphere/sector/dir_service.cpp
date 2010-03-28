//  Copyright (c) 2005-2007 Saurabh Sehgal ( saurabh.r.s@gmail.com)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)





#include <saga/saga.hpp>
#include <fsclient.h>
#include <dcclient.h>
#include <util.h>
#include <iostream>
#include "dir_service.hpp"

namespace saga_sector
{


   dir_service::dir_service(void) 
   {
      
   }
   
   dir_service::~dir_service(void) 
   {

   }

   int dir_service::open( std::string filename , int smode )
   {

      SNode attr ; 
      int error = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 

      error =  Sector::stat( filename, attr ) ; 
      if( error <  saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
      {
         /* Make the directory   
         */
         error = Sector::mkdir( filename ) ; 
         if( error <  saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
            return error ; 
         path = filename ; 
         op = true ; 
         fserv_.set_file_name( filename ) ; 
         return error ; 
      }
     
      bool is_dir = attr.m_bIsDir ; 
      if( is_dir )
      {
         op = true ; 
         path = filename ; 
         fserv_.set_file_name( filename ) ; 
	 return error ; 
      }
      else
         error = saga_sectorsphere::error::SAGA_SECSP_E_DIR_IS_FILE   ; 

      return error ; 

  }

  void dir_service::close( )
  {
     op = false ; 
     path = "" ; 
  }

  int dir_service::get_dir_size( saga::off_t &size )
  {
      if( path.empty() )
      {
         return saga_sectorsphere::error::SAGA_SECSP_E_BAD_ARGS ; 
      }
      
      return( fserv_.get_file_size( size ) ) ; 
  }


  int dir_service::copy ( std::string dest, std::string &err )
  {

     int error = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 
     error = fserv_.copy( dest )  ; 
     if( error <  saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        saga_sectorsphere::error::get_err_msg( err, error ) ; 
     } 
     return error ; 
  }


  int dir_service::move( std::string dest, std::string &err )
  {
     int error = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 
     error =  fserv_.move( dest )  ; 
     if( error <  saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        saga_sectorsphere::error::get_err_msg( err, error ) ; 
     }
     return error ; 
  }

  int dir_service::remove( std::string &err )
  {

     int error = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 
     error =  fserv_.remove() ; 
     if( error <  saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        saga_sectorsphere::error::get_err_msg( err, error ) ; 
     }
     return error ; 
  }

  int dir_service::list( std::vector <saga::url> & list , std::string pattern )
  {
     
      std::vector<SNode> filelist;
      bool pt = false ; 
      int error = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 
      error = Sector::list(path, filelist);

      if( error <  saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
      {
         return error ; 
      }

      if ( pattern != "" && pattern != "*" )
      {
         pt = true ; 
      }

      for ( std::vector<SNode>::iterator i = filelist.begin(); i != filelist.end(); ++ i)
      {
         if( pt )
         {
            if( WildCard::match( pattern, i->m_strName ))
            {
               list.push_back( saga::url(i->m_strName )) ; 
            }
         }
         else
         {
              list.push_back( saga::url(i->m_strName )) ; 
         }
      }

      return error ; 
  }

  int dir_service::get_num_entries( std::size_t &num )
  {
     
      std::vector<SNode> filelist;
      num = 0 ; 
      int error = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 
      
      error = Sector::list(path, filelist);
      if( error <  saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
      {
         return error ; 
      }

      for ( std::vector<SNode>::iterator i = filelist.begin(); i != filelist.end(); ++ i)
      {
         num++ ; 
      }

      return error ; 
  }


  int dir_service::mkdir( std::string & dir )
  {
     int error = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 
     error = Sector::mkdir( dir ) ; 
     return error ; 
  }

  void dir_service::get_error_msg( std::string & err, int const &code )
  {
     saga_sectorsphere::error::get_err_msg( err, code ) ; 
     return ; 
  }

}

