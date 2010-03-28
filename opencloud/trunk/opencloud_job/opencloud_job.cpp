//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga adaptor icnludes
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/attribute.hpp>
#include <saga/saga/adaptors/file_transfer_spec.hpp>

// saga engine includes
#include <saga/impl/config.hpp>
#include <saga/impl/exception_list.hpp>

// saga package includes
#include <saga/saga/packages/job/adaptors/job_self.hpp>
#include <saga/saga/packages/job/job_description.hpp>

// adaptor includes
#include "opencloud_job.hpp"
#include "opencloud_job_istream.hpp"
#include "opencloud_job_ostream.hpp"


////////////////////////////////////////////////////////////////////////
namespace opencloud_job
{

  // constructor
  job_cpi_impl::job_cpi_impl (proxy                           * p, 
                              cpi_info const                  & info,
                              saga::ini::ini const            & glob_ini, 
                              saga::ini::ini const            & adap_ini,
                              TR1::shared_ptr <saga::adaptor>   adaptor)
    : base_cpi  (p, info, adaptor, cpi::Noflags), 
      jserv_( serv_.get_sphere_service()) 
  {

  }


  // destructor
  job_cpi_impl::~job_cpi_impl (void)
  {
  }


  //  SAGA API functions
  void job_cpi_impl::sync_get_state (saga::job::state & ret)
  {
     jserv_.get_state( ret ) ; 
     return ; 
  
  }

  void job_cpi_impl::sync_get_description (saga::job::description & ret)
  {
    instance_data idata( this ) ;   
    if( idata->jd_is_valid_ )
       ret = idata->jd_.clone() ; 
    else
    {
       SAGA_ADAPTOR_THROW ("job description could not be retrieved", saga::DoesNotExist ) ; 
    }
  }

  void job_cpi_impl::sync_get_job_id (std::string & ret)
  {
      saga::job::state state = saga::job::Unknown;
      sync_get_state(state);
      if (saga::job::New == state) {
         return;
      }
      saga::attribute attr (this->proxy_);
      ret = attr.get_attribute(saga::job::attributes::jobid);
  }

  // access streams for communication with the child
  void job_cpi_impl::sync_get_stdin (saga::job::ostream & ret)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_get_stdout (saga::job::istream & ret)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_get_stderr (saga::job::istream & ret)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_checkpoint (saga::impl::void_t & ret)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_migrate (saga::impl::void_t           & ret, 
                                   saga::job::description   jd)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_signal (saga::impl::void_t & ret, 
                                  int            signal)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }


  //  suspend the child process 
  void job_cpi_impl::sync_suspend (saga::impl::void_t & ret)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  //  suspend the child process 
  void job_cpi_impl::sync_resume (saga::impl::void_t & ret)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }


  //////////////////////////////////////////////////////////////////////
  // inherited from the task interface
  void job_cpi_impl::sync_run (saga::impl::void_t & ret)
  {

     int rc = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 
     saga::job::state st = saga::job::Unknown ; 
     sync_get_state ( st ) ; 
     if( st != saga::job::New )
     {
        SAGA_ADAPTOR_THROW("Job has been already been started", 
                            saga::IncorrectState )
     }
     
     saga::job::description jd ; 
     {
        instance_data idata( this ) ; 
	if( !idata->jd_is_valid_ )
	{
            SAGA_ADAPTOR_THROW("Job description cannot be retrieved.", 
                                saga::NoSuccess ) ; 
	}
        jd = idata->jd_ ; 
     }

     /* The DLL executable
      */
     if( !jd.attribute_exists( saga::job::attributes::description_executable ) )
     {
          SAGA_ADAPTOR_THROW("No DLL/executable name provided", 
                              saga::NoSuccess ) ; 
     }

     std::string dll( jd.get_attribute( saga::job::attributes::description_executable ) ) ; 

     /* FIXME: This suffix is added because of a bug in the default adaptor that causes
      * a segmentation fault if a DLL name is given in the executable attribute. 
      */
     dll += ".so" ; 
     jserv_.set_dll_path( dll ) ; 


     /* The Arguments. The following syntax holds:
      *     input_file_dir output_file_dir function_name num_rows param_address<optional> param_size<optional>
      */
     std::vector<std::string> args = jd.get_vector_attribute( saga::job::attributes::description_arguments ) ; 
     std::vector<std::string>::iterator begin = args.begin() ; 
     int num_args = args.size() ; 
     if( num_args !=4 && num_args !=6 )
     {
          SAGA_ADAPTOR_THROW("Sector/Sphere: Malformed arguments. usage: input_file_dir output_file_dir function_name num_rows param_address<optional> param_size<optional> ", 
                              saga::NoSuccess ) ; 
     }
   
     jserv_.set_output_path  ( args[1] ) ; 
     jserv_.set_func_name    ( args[2] ) ; 
     std::istringstream istr ( args[3] ) ; 
     int rows = 0 ; 
     if( istr >> rows ){
        jserv_.set_num_rows  ( rows ) ;  
     }
     else{
        SAGA_ADAPTOR_THROW( "Sector/Sphere: Malformed arguments: rows", saga::BadParameter ) ; 
     }
        
     if( num_args == 6 )
     {
        void * ptr = NULL ; 
	size_t size = 0 ; 
        convert_arg( args[4], ptr ) ; 
	convert_arg( args[5], size ) ; 
        if( ptr ){
	   jserv_.set_arg( ptr, size ) ; 
	}
     }

     if( jd.attribute_exists( saga::job::attributes::description_file_transfer ))
     {
        std::vector<std::string> ft  = jd.get_vector_attribute( saga::job::attributes::description_file_transfer)  ; 
        std::string file_dir = ft.back() ;  
        std::vector<std::string> fileset ; 
        scan_and_find( file_dir , fileset ) ; 
	if( fileset.size() <= 0 ){
           SAGA_ADAPTOR_THROW( "Sector/Sphere: No input files present in directory to transfer", saga::DoesNotExist ) ; 
	}

        /*std::string dir ; 
	size_t pos = file_dir.rfind("/") ; 
	if( pos == std::string::npos ){
	   dir =  file_dir ; 
	}
	else{
	   dir  = file_dir.substr( pos + 1, std::string::npos ) ; 
	} */

        rc = jserv_.upload( file_dir , fileset ) ; 
        if( rc < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
        {
            std::string error ("Sector/Sphere: Error in file transfer - " ) ; 
            jserv_.get_error_msg( error, rc ) ; 
            SAGA_ADAPTOR_THROW( error , saga::NoSuccess ) ; 
        }
     }
     else {
        std::vector <std::string> in   ; 
        in.push_back( args[0] ) ; 
        jserv_.set_input_files  ( in ) ;  
     }


     rc = jserv_.run_job() ; 
     if( rc < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        std::string error ; 
	jserv_.get_error_msg( error, rc ) ; 
        SAGA_ADAPTOR_THROW( error , saga::NoSuccess ) ; 
     }

     return ; 
  }

  void job_cpi_impl::sync_cancel (saga::impl::void_t & ret, 
                                  double timeout)
  {
     int rc = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 
     rc = jserv_.cancel_job( ) ; 
     if( rc < saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
     {
        std::string error ; 
	jserv_.get_error_msg( error, rc ) ; 
        SAGA_ADAPTOR_THROW ( error , saga::NoSuccess ) ; 
     }

     return ; 
  }

  //  wait for the child process to terminate
  void job_cpi_impl::sync_wait (bool   & ret, 
                                double   timeout)
  {
     int rc = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 
     rc = jserv_.wait( timeout ) ; 
     if ( rc == saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS )
        ret = true ; 
  }


  int job_cpi_impl::scan_and_find( std::string const &file_dir , std::vector<std::string> &fileset )
  {
     int rc = saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 
     /* Open this directory and read in all the filenames 
      */
     if( boost::filesystem::exists( file_dir ) && boost::filesystem::is_directory( file_dir ) && 
         !boost::filesystem::is_empty( file_dir ) )
     {
        boost::filesystem::directory_iterator iter( file_dir ) ; 
        boost::filesystem::directory_iterator end ; 

	for( ; iter != end ; ++iter ){
	  if( !boost::filesystem::is_directory( *iter  ) ){
               std::string ( iter->leaf() ) ; 
	       fileset.push_back( iter->leaf() ) ; 
	  }
	  else{
	     /* Recurse into directory 
	      */
	     scan_and_find( file_dir, fileset ) ; 
	  }
        } 
     }
     else{
        SAGA_ADAPTOR_THROW ( "Sector/Sphere: Input file directory path does not exist or is not a directory", saga::DoesNotExist ) ; 
     }

     return rc ; 

  }

  template <typename PointerType > 
  void
  job_cpi_impl::
  convert_arg( std::string const &str , PointerType &in )
  {
     std::istringstream ss (str) ;  
     ss >> in ; 
  }

} // namespace opencloud_job
////////////////////////////////////////////////////////////////////////

