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
#include "naregi_cli.hpp"
#include "naregi_helper.hpp"
#include "naregi_job.hpp"
#include "naregi_job_istream.hpp"
#include "naregi_job_ostream.hpp"

////////////////////////////////////////////////////////////////////////
namespace naregi_job
{

  // constructor
  job_cpi_impl::job_cpi_impl (proxy                           * p,
                              cpi_info const                  & info,
                              saga::ini::ini const            & glob_ini,
                              saga::ini::ini const            & adap_ini,
                              TR1::shared_ptr <saga::adaptor>   adaptor)
    : base_cpi  (p, info, adaptor, cpi::Noflags)
  {
    update_state(proxy_, saga::job::Unknown);

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

      if (!data->init_from_jobid_) {
	update_state(proxy_, saga::job::New);
      } else {
	saga::adaptors::attribute attr(this);
	attr.set_attribute(sja::jobid, data->jobid_);
	saga::job::state state;
	sync_get_state(state);
      }
    } else {
      SAGA_OSSTREAM strm;
      strm << "Could not initialize job service for [" << data->rm_ << "]. "
	   << "Resource discovery is not available yet.";
      SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm),
			 saga::BadParameter);
    }
  }

  // destructor
  job_cpi_impl::~job_cpi_impl (void)
  {
  }

  ////////////////////////////////////////////////////////////////////////
  // call naregi_job_status
  void job_cpi_impl::sync_get_state (saga::job::state & ret)
  {
    // final state?
    saga::job::state s = current_state(proxy_);
    if (s == saga::job::Canceled ||
	s == saga::job::Done ||
	s == saga::job::Failed) {
      ret = s;
      return;
    }

    saga::adaptors::attribute attr(this);

    if(!attr.attribute_exists(sja::jobid)) {
      // New
      ret = s;
      return;
    }

    std::string jobid = attr.get_attribute(sja::jobid);
    std::string id = jobid_converter.convert_naregiid(jobid);

    bool success;
    std::string naregi_status;
    std::ostringstream os;
    success = cli::naregi_job_status(id, naregi_status, os);
    if (success) {
      // TODO naregi_status.empty() => saga::BadParameter ?
      // Job not found: ?
      s = helper::convert_saga_job_state(naregi_status);
    } else {
      std::string msg = os.str();
      saga::error e = cli::em.check(msg);
      if (e == saga::BadParameter) {
	s = saga::job::Canceled;
	success = true;
      } else {
	SAGA_ADAPTOR_THROW(msg, e);
      }
    }

    // Update the state
    update_state(proxy_, s);
    ret = s;
  }

  ////////////////////////////////////////////////////////////////////////
  //
  void job_cpi_impl::sync_get_description (saga::job::description & ret)
  {
    instance_data data(this);

    if (!data->jd_is_valid_) {
      SAGA_ADAPTOR_THROW("the job was not submitted through SAGA.",
                         saga::DoesNotExist);
    }

    ret = data->jd_.clone();
  }

  ////////////////////////////////////////////////////////////////////////
  //
  void job_cpi_impl::sync_get_job_id (std::string & ret)
  {
    saga::adaptors::attribute attr(this);
    if(!attr.attribute_exists(sja::jobid)) {
      // New
      ret = "";
    } else {
      ret = attr.get_attribute(sja::jobid);
    }
  }

  // access streams for communication with the child
  void job_cpi_impl::sync_get_stdin (saga::job::ostream & ret)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_get_stdout (saga::job::istream & ret)
  {
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);

	// final state?
	saga::job::state s = current_state(proxy_);
	if (s == saga::job::Canceled ||
	s == saga::job::Done ||
	s == saga::job::Failed) {
		//std::cout << "status OK to start sync_get_stdout."<< std::endl;
	}
	else {
		SAGA_ADAPTOR_THROW ("Does Not Exist", saga::DoesNotExist);
	}

	saga::adaptors::attribute attr(this);

	if(!attr.attribute_exists(sja::jobid)) {
		SAGA_ADAPTOR_THROW ("Does Not Exist", saga::DoesNotExist);
	}

	std::string jobid = attr.get_attribute(sja::jobid);
	std::string id = jobid_converter.convert_naregiid(jobid);

	bool success;
	std::ostringstream std_out;
	std::ostringstream os;
	success = cli::naregi_std_print(id, std_out, os);

	if (success) {
		std::istringstream is_buf(std_out.str());
		std::ostringstream std_out_buf;
		std::string line;
//		while (std::getline(is_buf, line)) {
//			if (line.find("Stderr") != std::string::npos){
//				break;
//			}
//			else {
//				std_out_buf << line << std::endl;
//			}
//		}
		while (std::getline(is_buf, line)) {
			if (line.find("Stdout") != std::string::npos){
				std_out_buf << line << std::endl;
				break;
			}
		}
		while (std::getline(is_buf, line)) {
			if (line.find("Stderr") != std::string::npos){
				break;
			}
			std_out_buf << line << std::endl;
		}

		std::istringstream is_str(std_out_buf.str());
		saga::job::istream is;
		is.copyfmt(is_str);
		is.clear(is_str.rdstate());
		is.rdbuf(is_str.rdbuf());

		ret = is;

	} else {
	  std::string msg = os.str();
	  saga::error e = cli::em.check(msg);
	  if (e == saga::BadParameter) {
		s = saga::job::Canceled;
		success = true;
	  } else {
		  SAGA_ADAPTOR_THROW(msg, e);
	  }
	}
  }

  void job_cpi_impl::sync_get_stderr (saga::job::istream & ret)
  {
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);

	// final state?
	saga::job::state s = current_state(proxy_);
	if (s == saga::job::Canceled ||
	s == saga::job::Done ||
	s == saga::job::Failed) {
		//std::cout << "status OK to start sync_get_stdout."<< std::endl;
	}
	else {
		SAGA_ADAPTOR_THROW ("Does Not Exist", saga::DoesNotExist);
	}

	saga::adaptors::attribute attr(this);

	if(!attr.attribute_exists(sja::jobid)) {
		SAGA_ADAPTOR_THROW ("Does Not Exist", saga::DoesNotExist);
	}

	std::string jobid = attr.get_attribute(sja::jobid);
	std::string id = jobid_converter.convert_naregiid(jobid);

	bool success;
	std::ostringstream std_out;
	std::ostringstream os;
	success = cli::naregi_std_print(id, std_out, os);

	if (success) {
		std::istringstream is_buf(std_out.str());
		std::ostringstream std_out_buf;
		std::string line;
//		bool rd_flg = false;
//		while (std::getline(is_buf, line)) {
//			if (line.find("Stderr") != std::string::npos){
//				rd_flg = true;
//				std_out_buf << line << std::endl;
//			}
//			else if (rd_flg){
//				std_out_buf << line << std::endl;
//			}
//		}
		while (std::getline(is_buf, line)) {
			if (line.find("Stderr") != std::string::npos){
				std_out_buf << line << std::endl;
				break;
			}
		}
		while (std::getline(is_buf, line)) {
			if (line.find("Stdout") != std::string::npos){
				break;
			}
			std_out_buf << line << std::endl;
		}

		std::istringstream is_str(std_out_buf.str());
		saga::job::istream is;
		is.copyfmt(is_str);
		is.clear(is_str.rdstate());
		is.rdbuf(is_str.rdbuf());

		ret = is;

	} else {
	  std::string msg = os.str();
	  saga::error e = cli::em.check(msg);
	  if (e == saga::BadParameter) {
		s = saga::job::Canceled;
		success = true;
	  } else {
		  SAGA_ADAPTOR_THROW(msg, e);
	  }
	}

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

  void job_cpi_impl::sync_suspend (saga::impl::void_t & ret)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_resume (saga::impl::void_t & ret)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }


  //////////////////////////////////////////////////////////////////////
  // inherited from the task interface
  // call naregi_job_submit
  void job_cpi_impl::sync_run (saga::impl::void_t & ret)
  {
    saga::job::state state = current_state(proxy_);
    if (state != saga::job::New) {
        SAGA_ADAPTOR_THROW("The job has already been started!",
                           saga::IncorrectState);
    }

    instance_data data(this);
    saga::job::description jd = data->jd_;

    adaptor_data_type ad(this);
    boost::scoped_ptr<wfml::workflow>
      wf(ad->create_workflow(jd, localhost));

    std::string id;
    std::ostringstream os;
    bool success = cli::naregi_job_submit(wf.get(), id, os);
    if (!success) {
      std::string msg = os.str();
      saga::error e = cli::em.check(msg);
      SAGA_ADAPTOR_THROW(msg, e);
    }

    std::string jobid = jobid_converter.convert_jobid(id);
    saga::adaptors::attribute attr(this);
    attr.set_attribute(sja::jobid, jobid);

    ad->register_job(id, jd);

    update_state(proxy_, saga::job::Running);
  }

  ////////////////////////////////////////////////////////////////////////
  //
  void job_cpi_impl::sync_cancel (saga::impl::void_t & ret,
                                  double timeout)
  {
    saga::job::state state = current_state(proxy_);
    // saga::job::Running || saga::job::Suspend
    if (state != saga::job::Running) {
        SAGA_ADAPTOR_THROW("The job has not started", saga::IncorrectState);
    }

    saga::adaptors::attribute attr(this);
    std::string jobid = attr.get_attribute(sja::jobid);
    std::string id = jobid_converter.convert_naregiid(jobid);

    ////////////////////////////////////////////////////////////
    //  GFD.90  2.6.3 Timeouts
    //
    //  timeout < 0.0 -- wait forever
    //  timeout = 0.0 -- return immediately
    //  timeout > 0.0 -- wait for this many seconds
    //
    bool success;
    std::ostringstream os;

    // TODO
    if (timeout < 0.0) {
      success = cli::naregi_job_cancel(id, os);
    } else if (timeout == 0.0) {
      success = cli::naregi_job_cancel(id, os);
    } else { // timeout > 0.0
      success = cli::naregi_job_cancel(id, os);
    }

    if (success) {
      update_state(proxy_, saga::job::Canceled);
    } else {
      std::string msg = os.str();
      saga::error e = cli::em.check(msg);
      SAGA_ADAPTOR_THROW(msg, e);
    }
  }

  //  wait for the child process to terminate
  void job_cpi_impl::sync_wait (bool   & ret,
                                double   timeout)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ////////////////////////////////////////////////////////////////////////
  //
  void update_state(saga::impl::proxy* proxy_, saga::job::state s)
  {
    saga::monitorable monitor(proxy_);
    saga::adaptors::metric m(monitor.get_metric(saga::metrics::task_state));

    m.set_attribute(sa::metric_value, boost::lexical_cast<std::string>(s));
    // m.fire(); ?
  }

  ////////////////////////////////////////////////////////////////////////
  //
  saga::job::state current_state(saga::impl::proxy* proxy_)
  {
    saga::monitorable monitor(proxy_);
    saga::adaptors::metric m(monitor.get_metric(saga::metrics::task_state));

    return (saga::job::state)
      boost::lexical_cast<int>(m.get_attribute(sa::metric_value));
  }

} // namespace naregi_job
////////////////////////////////////////////////////////////////////////

