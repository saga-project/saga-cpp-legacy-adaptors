//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

// system includes
#include <string.h>

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga adaptor includes
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
#include "ogf_drmaa_job.hpp"

namespace sja = saga::job::attributes;

////////////////////////////////////////////////////////////////////////
namespace ogf_drmaa_job
{
  // constructor
  job_cpi_impl::job_cpi_impl (proxy                           * p, 
                              cpi_info const                  & info,
                              saga::ini::ini const            & glob_ini, 
                              saga::ini::ini const            & adap_ini,
                              TR1::shared_ptr <saga::adaptor>   adaptor)
    : base_cpi (p, info, adaptor, cpi::Noflags)
    , session_ (p->get_session ())
    , state_   (saga::job::New)
  {
    instance_data     idata (this);
    adaptor_data_type adata (this);

    saga::url contact_url = idata->rm_;

    SAGA_LOG_INFO("url: " + contact_url.get_url ());

    // check if URL is usable
    if ( ! contact_url.get_scheme ().empty ()    &&
           contact_url.get_scheme () != "drmaa"    &&
           contact_url.get_scheme () != "any"    )
    {
      SAGA_OSSTREAM strm;
      strm << "Could not initialize job service for [" << contact_url << "]. "
           << "Only these schemas are supported: any://, drmaa://, or none.";

      SAGA_ADAPTOR_THROW (SAGA_OSSTREAM_GETSTRING (strm), 
                          saga::adaptors::AdaptorDeclined);
    }

    // TODO: load drmaa && drmaa_init
    SAGA_LOG_INFO("getting DRMAA singleton");

    drmaa_ = &(saga::adaptors::utils::get_singleton<psnc_drmaa::drmaa>());

    if ( idata->init_from_jobid_ )
    {
      jobid_ = idata->jobid_;
      state_ = drmaa_->get_state(jobid_);
    }
    else
    {
      // init from job description
      jd_ = idata->jd_;
      state_ = saga::job::New;
      
      if ( ! jd_.attribute_exists (sja::description_executable) )
      {
        SAGA_ADAPTOR_THROW ("job description misses executable", saga::BadParameter);
      }
    }

    // FIXME: register metrics etc.
  }


  // destructor
  job_cpi_impl::~job_cpi_impl (void)
  {
	  //TODO: delete JT, drmaa_exit ?
	SAGA_LOG_INFO("releasing DRMAA singleton");
  }


  //  SAGA API functions
  void job_cpi_impl::sync_get_state (saga::job::state & ret)
  {
    adaptor_data_type adata (this);

    ret = drmaa_->get_state(jobid_);

  }

  void job_cpi_impl::sync_get_description (saga::job::description & ret)
  {
    ret = jd_;
  }

  void job_cpi_impl::sync_get_job_id (std::string & ret)
  {
    ret = jobid_;
  }

  void job_cpi_impl::sync_get_stdin (saga::job::ostream & ret)
  {
    // not available in DRMAA
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_get_stdout (saga::job::istream & ret)
  {
    // not available in DRMAA
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_get_stderr (saga::job::istream & ret)
  {
    // not available in DRMAA
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_checkpoint (saga::impl::void_t & ret)
  {
    // not available in DRMAA
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_migrate (saga::impl::void_t     & ret, 
                                   saga::job::description   jd)
  {
    // not available in DRMAA
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_signal (saga::impl::void_t & ret, 
                                  int            signal)
  {
    // not available in DRMAA
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }


  //  suspend the child process 
  void job_cpi_impl::sync_suspend (saga::impl::void_t & ret)
  {
	drmaa_->suspend_job(jobid_);
  }

  //  suspend the child process 
  void job_cpi_impl::sync_resume (saga::impl::void_t & ret)
  {
    drmaa_->resume_job(jobid_);
  }


  //////////////////////////////////////////////////////////////////////
  // inherited from the task interface
  void job_cpi_impl::sync_run (saga::impl::void_t & ret)
  {
    if ( state_ != saga::job::New )
    {
      SAGA_ADAPTOR_THROW ("can run only 'New' jobs", saga::IncorrectState);
    }

    jobid_   =  drmaa_->run_job(jd_);

    SAGA_LOG_INFO("Successfully submitted job: " + jobid_);;
  }

  void job_cpi_impl::sync_cancel (saga::impl::void_t & ret, 
                                  double timeout)
  {
    try
    {
      drmaa_->kill_job(jobid_);
    }
    catch ( const char * msg )
    {
      SAGA_ADAPTOR_THROW (msg, saga::NoSuccess);
    }
  }

  //  wait for the child process to terminate
  void job_cpi_impl::sync_wait (bool   & ret, 
                                double   timeout)
  {
    adaptor_data_type adata(this);
    drmaa_->wait_job(jobid_, (int)timeout);
  }

  // TODO: add state polling and metrics support

} // namespace ogf_drmaa_job
////////////////////////////////////////////////////////////////////////



