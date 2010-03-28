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
#include <vector>
#include <cstdlib>
#include "job_service.hpp"

namespace saga_sphere
{

   job_service::job_service(void) 
    :  dll_path   (""), 
       func       (""), 
       output_path(""), 
       arg        (NULL), 
       rows       (-1), 
       have_output(false), 
       st_        (saga::job::New)
   {
   }
   
   job_service::~job_service(void) 
   {
   }

  job_service::job_service( std::string const &path, std::string const &func , std::vector<std::string> file_list , 
                            std::string const &output_path )
      :   dll_path ( path ), 
          func( func ), 
	  input_fileset( file_list ), 
	  output_path( output_path )
   {
   }
   
  void job_service::set_dll_path    ( std::string const &dll ) 
  {
     dll_path = dll ; 
  }

  void job_service::set_func_name   ( std::string const &f ) 
  {
     func = f ; 
  }

  void job_service::set_num_rows   ( int const &r )
  {
     rows = r ; 
  }
  
  void job_service::set_input_files ( std::vector<std::string> const &in ) 
  {
     input_fileset = in ; 
  }
  
  void job_service::set_output_path( std::string const &out ) 
  {
     output_path = out ; 
  }

  void job_service::set_arg( void * ptr , size_t s )
  {
     size = s ; 
     arg = malloc( size ) ; 
     if( NULL != arg ){
       memcpy( arg, ptr, size ) ; 
     }
  }

  int job_service::cancel_job()
  {
     int rc =  proc.close() ;
     if( rc < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ){
        st_ = saga::job::Unknown ; 
     }
     st_ = saga::job::Canceled ; 
     return rc ; 
  }
  

  int job_service::run_job()
  {
     
     int rc = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ;    
     bool have_arg = false ;  

     if( dll_path.empty() || ( dll_path.rfind( ".so" ) == std::string::npos ) )
     {
        rc = saga_sectorsphere::error::SAGA_SECSP_E_NO_JOB_OPERATOR ; 
	return rc ; 
     }
     else {
        proc.loadOperator( dll_path.c_str() ) ;   
     }

     if( NULL != arg )
     {
        have_arg = true ; 
     }

     if( rc = initialize_input_stream  () < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        return rc ; 
     }

     rc = initialize_output_stream  ()  ; 
     if( rc  == saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        have_output = true ; 
     }
     else if( rc  < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS && 
	      rc  != saga_sectorsphere::error::SAGA_SECSP_E_NO_SPHERE_OUTPUT ) 
     {
        return rc ; 
     }

     /* Start Sphere 
      */
     if( have_arg ) {  
        rc = proc.run( input, output, func, rows, (const char *)arg, size ) ; 
     }
     else{
        rc = proc.run( input, output, func, rows ) ; 
     }

     if( rc == saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ){
        st_ = saga::job::Running ; 
     }
     else {
        st_ = saga::job::Failed ; 
     }
     return rc ; 
  }

  void job_service::get_state( saga::job::state &st )
  {
     
     if( st_ != saga::job::Running ){
	st = st_ ; 
	return ; 
     }

     if( st_ == saga::job::Running ){

        int rc  =  proc.checkProgress() ; 
        if( 0 <= rc < 100 ){
	   st = st_ ; 
	   return ; 
        }

        if( rc == 100 ){
           st_ = saga::job::Done ; 
	   st = st_ ; 
	   return ; 
        }

	if( rc < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ){
           st_ = saga::job::Failed ; 
	   st = st_ ; 
	}
    }
  }


  int job_service::wait      ( const double &timeout ) 
  {
     
     if( st_ != saga::job::Running )
     {
        return saga_sectorsphere::error::SAGA_SECSP_E_JOB_NOT_STARTED ; 
     }
     
     int rc = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 
     std::time_t start = time(0) ; 
     
     while( true )
     {
        SphereResult * res ; 
	if( proc.read( res ) < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
	{
           rc = proc.checkProgress() ; 
	   std::cout << "Progress: " << rc << std::endl ; 
	   if( rc  < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ){
	      st_ = saga::job::Failed ; 
	      return rc ; 
	   }

	   rc = proc.checkProgress()  ; 

	   if( rc == 100 ){
	      st_ = saga::job::Done ; 
	      break ; 
	   }
	}
        
        if( timeout > 0 ) 
        {
           if( timeout - (std::difftime (std::time(0), start)) > 0 ){
	      continue ; 
	   }
	   else {
	      break ; 
	   }
        } 
     } 

     return saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 
  }
  
  int job_service::upload ( std::string const &dir, std::vector<std::string> &files ) 
  {
     
      int rc = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 

      SNode attr ; 
      rc = Sector::stat( dir, attr ) ; 
      if( rc < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS || !attr.m_bIsDir ) {
        if( rc = Sector::mkdir( dir ) < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ) {
	   return rc ; 
	}
      }
      
      std::string to_create ; 
      std::string to_upload ; 
      to_create = dir ; 

      size_t pos = dir.rfind("/") ; 
      if( pos == std::string::npos ){
         to_create = dir ; 
      }
      else if( pos == to_create.size()) {
         size_t p = to_create.find_last_of( "/", to_create.size() - 1 ) ; 
	 if ( p != std::string::npos ) {
            to_create = dir.substr( p, dir.size() - 1 ) ; 
	 }
      }
      else{
	 to_create = dir.substr( pos + 1, std::string::npos ) ; 
      }

      to_upload = dir ; 

      for( std::vector<std::string>::iterator it = files.begin() ; it !=files.end()  ; ++it ){
      
         SectorFile f ;

	 std::string cr = to_create ; 
	 cr += "/" ; 
	 cr += *it ; 

	 std::string up = to_upload ; 
	 up += "/" ; 
	 up += *it ; 

         rc = f.open( cr, SF_MODE::WRITE ) ; 
         if( rc < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ) {
            return rc ; 
         }
      
         if( rc = f.upload( up.c_str() ) < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ){
	    return rc ; 
	 }

	 input_fileset.push_back( cr ) ; 
	 f.close() ; 
      }

      return rc ; 
  }


  void job_service::get_error_msg( std::string & err, int const &code ) {

    saga_sectorsphere::error::get_err_msg( err, code ) ; 
    return ; 
 } 
}

