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
#include <algorithm>
#include <vector>

// boost includes
#include <boost/bind.hpp>

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/exception.hpp>

// saga engine includes
#include <saga/impl/config.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/attribute.hpp>

// saga package includes
#include <saga/saga/packages/job/adaptors/job.hpp>
#include <saga/saga/packages/job/adaptors/job_self.hpp>

// adaptor includes
#include "naregi_cli.hpp"
#include "naregi_helper.hpp"
#include "naregi_job.hpp"
#include "naregi_job_service.hpp"

////////////////////////////////////////////////////////////////////////
namespace naregi_job
{

  ////////////////////////////////////////////////////////////////////////
  //
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

      if (!scheme.empty() && scheme != "naregi" && scheme != "any") {
        SAGA_OSSTREAM strm;
        strm << "Could not initialize job service for [" << data->rm_ << "]. "
             << "Only any:// and naregi:// schemes are supported.";
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
                         saga::BadParameter);
    }

    instance_data data(this);

    // create new job. state == saga::job::New
    saga::job::job job = saga::adaptors::job(data->rm_.get_url(),
                                             jd, proxy_->get_session());
    ret = job;
  }

  //////////////////////////////////////////////////////////////////////
  // call naregi_simplejob_submit
  void
    job_service_cpi_impl::sync_run_job (saga::job::job     & ret,
                                        std::string          cmd,
                                        std::string          host,
                                        saga::job::ostream & in,
                                        saga::job::istream & out,
                                        saga::job::istream & err)
  {
    // call naregi_simplejob_submit
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  //////////////////////////////////////////////////////////////////////
  // call naregi_simplejob_submit
  // <JobName> "Program"
  void
    job_service_cpi_impl::sync_run_job_noio (saga::job::job     & ret,
                                             std::string          cmd,
                                             std::string          host)
  {
    saga::job::description jd;

    if (helper::create_saga_job_description(jd, cmd, host) == false) {
      SAGA_ADAPTOR_THROW("Could not parse command.", saga::BadParameter);
    }

    instance_data data(this);

    // create new job. state == saga::job::New
    saga::job::job job = saga::adaptors::job(data->rm_.get_url(),
                                             jd, proxy_->get_session());

    std::string id;
    std::ostringstream os;
    if (cli::naregi_simplejob_submit(jd, id, os) == false) {
      std::string msg = os.str();
      saga::error e = cli::em.check(msg);
      SAGA_ADAPTOR_THROW(msg, e);
    }

    //std::cout << "job_service_cpi_impl::sync_run_job_noio , id =" << id << std::endl;

    std::string jobid = jobid_converter.convert_jobid(id);
    saga::adaptors::attribute attr(job);
    attr.set_attribute(sja::jobid, jobid);

    adaptor_data_type ad(this);
    ad->register_job(id, jd);

    // set current state
    job.get_state();

    ret = job;
  }

  //////////////////////////////////////////////////////////////////////
  // call naregi_job_list
  void
    job_service_cpi_impl::sync_list (std::vector <std::string> & ret)
  {
    std::ostringstream os;
    if (cli::naregi_job_list(ret, os) == false) {
      std::string msg = os.str();
      saga::error e = cli::em.check(msg);
      SAGA_ADAPTOR_THROW(msg, e);
    }
    // ret = "CID_XXX", "CID_YYY", "CID_ZZZ" ...

    instance_data data(this);
    transform(ret.begin(), ret.end(), ret.begin(),
	      boost::bind<std::string>(&helper::jobid_converter::convert_jobid,
				       &jobid_converter, _1));
    // ret = "[rm]-[CID_XXX]", "[rm]-[CID_YYY]", "[rm]-[CID_ZZZ]" ...
  }

  ////////////////////////////////////////////////////////////////////////
  //
  void
    job_service_cpi_impl::sync_get_job (saga::job::job & ret,
                                        std::string      jobid)
  {
    std::string id = jobid_converter.convert_naregiid(jobid);
    // throw saga::BadParameter if format error

    adaptor_data_type ad(this);
    instance_data data(this);

    saga::job::description jd;
    saga::job::job job;

    if (ad->find_job(id, jd)) {
      job = saga::adaptors::job(data->rm_.get_url(),
				jd, proxy_->get_session());
      // TODO shared?
    } else {
      bool success;
      std::string naregi_status;
      std::ostringstream os;
      success = cli::naregi_job_status(id, naregi_status, os);
      if (success) {
	job = saga::adaptors::job(data->rm_.get_url(),
				  jobid, proxy_->get_session());
      } else {
	std::string msg = os.str();
	saga::error e = cli::em.check(msg);
	if (e == saga::BadParameter) {
	  SAGA_ADAPTOR_THROW("Could not found job " + jobid,
			     saga::DoesNotExist);
	} else {
	  SAGA_ADAPTOR_THROW(msg, e);
	}
      }
    }

    saga::adaptors::attribute attr(job);
    attr.set_attribute(sja::jobid, jobid);
    ret = job;
  }

  //////////////////////////////////////////////////////////////////////
  // ?
  void job_service_cpi_impl::sync_get_self (saga::job::self & ret)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

} // namespace naregi_job
////////////////////////////////////////////////////////////////////////

