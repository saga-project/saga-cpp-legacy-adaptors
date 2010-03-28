//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

// stl includes
#include <vector>

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga engine includes
#include <saga/impl/config.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/attribute.hpp>

// saga package includes
#include <saga/saga/packages/job/adaptors/job.hpp>
#include <saga/saga/packages/job/adaptors/job_self.hpp>

// adaptor includes
#include "opencloud_job_service.hpp"


////////////////////////////////////////////////////////////////////////
namespace opencloud_job
{
  // constructor
  job_service_cpi_impl::job_service_cpi_impl (proxy                * p, 
                                              cpi_info const       & info,
                                              saga::ini::ini const & glob_ini, 
                                              saga::ini::ini const & adap_ini,
                                              TR1::shared_ptr <saga::adaptor> adaptor)
    : base_cpi (p, info, adaptor, cpi::Noflags)
  {
//     adaptor_data adata( this ) ; 
 //    instance_data idata( this ) ; 

     s_ = p->get_session() ; 
  }

  // destructor
  job_service_cpi_impl::~job_service_cpi_impl (void)
  {
  }

  //////////////////////////////////////////////////////////////////////
  // SAGA API functions
  void 
    job_service_cpi_impl::sync_create_job (saga::job::job         & ret, 
                                           saga::job::description   jd)
  {
     
    // Make sure we have all the information we need 
    saga::attribute attr (jd);
    std::vector<std::string> args = attr.get_vector_attribute( saga::job::attributes::description_arguments ) ; 
    int num_args = args.size() ;
    if( num_args != 6 && num_args != 4 ){
         SAGA_ADAPTOR_THROW("Malformed arguments. usage - input_fileset output_path function_name num_rows param_address<optional> param_size<optional>", 
                             saga::NoSuccess ) ;
    }

    if (!attr.attribute_exists(saga::job::attributes::description_executable) ||
        attr.get_attribute(saga::job::attributes::description_executable).empty())
    {
        SAGA_ADAPTOR_THROW("Missing 'Executable' attribute in job description.",
                           saga::BadParameter);
    }

    saga::url rm ; 
    {
      instance_data data(this) ;  
      rm = data->rm_ ; 
    }
    
    saga::job::job j_  = saga::adaptors::job( rm, jd, s_ ) ; 

    // set the created attribute
    saga::adaptors::attribute jobattr (j_) ; 
    std::time_t current = 0;
    std::time(&current);
    jobattr.set_attribute(saga::job::attributes::created, ctime(&current));

    ret = j_ ; 

  }

  void 
    job_service_cpi_impl::sync_run_job (saga::job::job     & ret, 
                                        std::string          cmd, 
                                        std::string          host, 
                                        saga::job::ostream & in, 
                                        saga::job::istream & out, 
                                        saga::job::istream & err)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    job_service_cpi_impl::sync_list (std::vector <std::string> & ret)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void
    job_service_cpi_impl::sync_get_job (saga::job::job & ret, 
                                        std::string      jobid)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_service_cpi_impl::sync_get_self (saga::job::self & ret)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

} // namespace opencloud_job
////////////////////////////////////////////////////////////////////////

