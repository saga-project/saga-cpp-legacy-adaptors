/*
 * Copyright (C) 2008-2009 High Energy Accelerator Research Organization (KEK)
 * Copyright (C) 2008-2009 National Institute of Informatics in Japan.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
#include "torque_cli.hpp"
#include "torque_job_service.hpp"


////////////////////////////////////////////////////////////////////////
namespace torque_job
{
  // constructor
  job_service_cpi_impl::job_service_cpi_impl (proxy                * p,
                                              cpi_info const       & info,
                                              saga::ini::ini const & glob_ini,
                                              saga::ini::ini const & adap_ini,
                                              TR1::shared_ptr <saga::adaptor> adaptor)
    : base_cpi (p, info, adaptor, cpi::Noflags)
  {
    instance_data data(this);

    if (!data->rm_.get_url().empty()) {
      saga::url rm(data->rm_);

      std::string scheme(rm.get_scheme());

      if (!scheme.empty() && scheme != "torque" && scheme != "xt5torque") {
        SAGA_OSSTREAM strm;
        strm << "Could not initialize job service for [" << data->rm_ << "]. "
             << "Only any://, torque:// and xt5torque:// schemes are supported.";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm),
                           saga::BadParameter);
      }

      localhost = rm.get_host();
      if (localhost.empty()) {
	// TODO && local host check

        SAGA_OSSTREAM strm;
        strm << "Could not initialize job service for [" << data->rm_ << "]. "
             << "invalid hostname.";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm),
                           saga::BadParameter);
      }

      jobid_converter = helper::jobid_converter(rm);

    } else {
      SAGA_OSSTREAM strm;
      strm << "Could not initialize job service for [" << data->rm_ << "]. "
	   << "Resource discovery is not available yet.";
      SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm),
			 saga::BadParameter);
    }
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
    if (!jd.attribute_exists(sja::description_executable)
        || jd.get_attribute(sja::description_executable).empty()) {
      SAGA_ADAPTOR_THROW("Missing 'Executable' attribute in job description.",
                         saga::BadParameter);
    }

    if (jd.attribute_exists(sja::description_interactive)
	&& jd.get_attribute(sja::description_interactive) == sa::common_true) {
      SAGA_ADAPTOR_THROW("Interactive execution not implemented.",
                         saga::NotImplemented);
    }

    instance_data data(this);

    // create new job. state == saga::job::New
    saga::job::job job = saga::adaptors::job(data->rm_.get_url(),
                                             jd, proxy_->get_session());
    ret = job;
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
    job_service_cpi_impl::sync_run_job_noio (saga::job::job     & ret,
                                             std::string          cmd,
                                             std::string          host)
  {
    saga::job::description jd;
    if (helper::create_saga_job_description(jd, cmd, host)
	== false) {
      SAGA_ADAPTOR_THROW("Could not parse command.", saga::BadParameter);
    }

    instance_data data(this);
    adaptor_data_type ad(this);

    // create new job. state == saga::job::New
    saga::job::job job = saga::adaptors::job(data->rm_.get_url(),
                                             jd, proxy_->get_session());
    std::string pbsid;
    std::ostringstream os;

    std::string bin_pth(ad->get_binary_path());
    
    saga::url rm(data->rm_);
    std::string url_scheme(rm.get_scheme());
    cli::qsub qsub(localhost, bin_pth, url_scheme);

    if (qsub.execute(jd, pbsid, os) == false) {
      std::string msg = os.str();
      // please check user@host
      SAGA_ADAPTOR_THROW(msg, saga::NoSuccess);
    }

    std::string sagaid = jobid_converter.convert_jobid(pbsid);
    saga::adaptors::attribute attr(job);
    attr.set_attribute(sja::jobid, sagaid);

    ad->register_job(pbsid, jd);
    // set current state
    job.get_state();
    ret = job;
  }

  void
    job_service_cpi_impl::sync_list (std::vector <std::string> & ret)
  {
    std::vector<std::string> backend_list;

    std::ostringstream os;

    adaptor_data_type ad(this);
    std::string bin_pth(ad->get_binary_path());
    cli::qstat qstat(bin_pth);
    
    if (qstat.execute(backend_list, os) == false) {
      std::string msg = os.str();
      //saga::error e = cli::em.check(msg);
      //SAGA_ADAPTOR_THROW(msg, e);
      SAGA_ADAPTOR_THROW(msg, saga::NoSuccess);
    }
#if 0
    std::vector<std::string> adaptor_list;
    //instance_data data(this);
    transform(adaptor_list.begin(), adaptor_list.end(), adaptor_list.begin(),
	      boost::bind<std::string>(&helper::jobid_converter::convert_jobid,
				       &jobid_converter, _1));
#endif
    transform(backend_list.begin(), backend_list.end(), backend_list.begin(),
	      boost::bind<std::string>(&helper::jobid_converter::convert_jobid,
				       &jobid_converter, _1));
    ret = backend_list;
  }

  void
    job_service_cpi_impl::sync_get_job (saga::job::job & ret,
                                        std::string      jobid)
  {
    std::string pbsid = jobid_converter.convert_pbsid(jobid);
    // throw saga::BadParameter if format error

    adaptor_data_type ad(this);
    instance_data data(this);

    saga::job::description jd;
    saga::job::job job;

    if (ad->find_job(pbsid, jd)) {
      job = saga::adaptors::job(data->rm_.get_url(),
				jd, proxy_->get_session());
      // TODO shared?
    } else {
      std::string unuse;
      std::ostringstream os;

    adaptor_data_type ad(this);
    std::string bin_pth(ad->get_binary_path());
    cli::qstat qstat(bin_pth);
    
      bool found = qstat.get_state(pbsid, unuse, os);
      if (found) {
	job = saga::adaptors::job(data->rm_.get_url(),
				  jobid, proxy_->get_session());
	// TODO register_job(pbsid, null)
	// set state?
      } else {
	std::string msg = os.str();
	SAGA_ADAPTOR_THROW(msg, saga::DoesNotExist);
	//SAGA_ADAPTOR_THROW(msg, saga::NoSuccess);
      }
    }

    saga::adaptors::attribute attr(job);
    attr.set_attribute(sja::jobid, jobid);

    ret = job;
  }

  void job_service_cpi_impl::sync_get_self (saga::job::self & ret)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

} // namespace torque_job
////////////////////////////////////////////////////////////////////////

