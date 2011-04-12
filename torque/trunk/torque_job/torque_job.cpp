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
#include "torque_job.hpp"
#include "torque_job_istream.hpp"
#include "torque_job_ostream.hpp"

#include "torque_cli.hpp"
#include "torque_helper.hpp"


////////////////////////////////////////////////////////////////////////
namespace torque_job
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
      if (!scheme.empty() && scheme != "torque" && scheme != "any") {
        SAGA_OSSTREAM strm;
        strm << "Could not initialize job service for [" << data->rm_ << "]. "
             << "Only any:// and torque:// schemes are supported.";
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

	saga::job::description& jd = data->jd_;
	if (!jd.attribute_exists(sja::description_job_contact)
	    || jd.get_attribute(sja::description_job_contact).empty()) {
	  adaptor_data_type ad(this);
	  jd.set_attribute(sja::description_job_contact, ad->get_job_contact());
	}

      } else {
	saga::adaptors::attribute attr(this);
	attr.set_attribute(sja::jobid, data->jobid_);
	// FIXME ? saga::job::Unknown?
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


  //  SAGA API functions
  /*******************************************************************
   *  sync_get_state
   *
   *  PBS job state:
   *    C -  Job is completed after having run.
   *    E -  Job is exiting after having run.
   *    H -  Job is held.
   *    Q -  job is queued, eligible to run or routed.
   *    R -  job is running.
   *    S -  (Unicos only) job is suspend.
   *    T -  job is being moved to new location.
   *    W -  job is waiting for its execution time
   *         (-a option) to be reached.
   */
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

    std::string sagaid = attr.get_attribute(sja::jobid);
    std::string pbsid = jobid_converter.convert_pbsid(sagaid);

    std::ostringstream os;

    adaptor_data_type ad(this);
    std::string bin_pth(ad->get_binary_path());
    cli::qstat qstat(bin_pth);


    cli::jobstat_ptr fullstat = qstat.get_full_status(pbsid, os);
    if (fullstat.get()) {
      std::string pbs_state = fullstat->get_job_state();

      if (pbs_state == "E" ||
	  pbs_state == "H" ||
	  pbs_state == "Q" ||
	  pbs_state == "R" ||
	  pbs_state == "W") {
	s = saga::job::Running;                       /*   2 */
      } else if (pbs_state == "C") {
	if (fullstat->get_exit_status() == "0") {
	  s = saga::job::Done;                        /*   3 */
	} else {
	  s = saga::job::Failed;                      /*   5 */
	}
      } else if (pbs_state == "S") {
	s = saga::job::Suspended;                     /*   6 */
      } else {
	s = saga::job::Unknown;                       /*  -1 */
      }

    } else {
      std::string msg = os.str();
      // please check user@host
      SAGA_ADAPTOR_THROW(msg, saga::NoSuccess);
      // DoesNotExist?
    }

    // Update the state
    update_state(proxy_, s);
    ret = s;
  }

  void job_cpi_impl::sync_get_description (saga::job::description & ret)
  {
    instance_data data(this);

    if (!data->jd_is_valid_) {
      SAGA_ADAPTOR_THROW("the job was not submitted through SAGA.",
                         saga::DoesNotExist);
    }

    ret = data->jd_.clone();
  }

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
	std::string id = jobid_converter.convert_pbsid_short(jobid);

	//Get Current Job Description
	instance_data data(this);
	saga::job::description& jd = data->jd_;

	//Check Output file name
	std::string stdof;
	if (jd.attribute_exists(sja::description_output)) {
		stdof = jd.get_attribute(sja::description_output);
		if (!(stdof.size() > 0)){
			stdof = "saga-app.o" + id;
		}
	}
	else {
		stdof = "saga-app.o" + id;
	}

	//Check Working Directory
	std::string wkdir;
	std::string home_pth = getenv("HOME");
	if (jd.attribute_exists(sja::working_directory)) {
		wkdir = jd.get_attribute(sja::working_directory);
		if (wkdir.size() > 0) {
			// Absolute path ?
			if (wkdir.substr(0,1) != "/"){
				wkdir = home_pth + "/" + wkdir;
			}
			if (wkdir.substr(wkdir.size()-1,1) != "/"){
				wkdir += "/";
			}
		}
		else {
			wkdir = home_pth + "/";
		}
	}
	else {
		wkdir = home_pth + "/";
	}

	// Open Standard Output file
	stdof = wkdir + stdof;
	std::ifstream ifs(stdof.c_str());
	std::ostringstream os;
	std::string str;

	if (ifs.fail()){
		str = "Cannot find stdout file." ;
	}
	else {
		while(ifs && getline(ifs, str)) {
			os << str << std::endl;
	  }
	}

	std::istringstream is_str(os.str());
	saga::job::istream is;
	is.copyfmt(is_str);
	is.clear(is_str.rdstate());
	is.rdbuf(is_str.rdbuf());

	ret = is;
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
	std::string id = jobid_converter.convert_pbsid_short(jobid);

	//Get Current Job Description
	instance_data data(this);
	saga::job::description& jd = data->jd_;

	//Check Output file name
	std::string stdef;
	if (jd.attribute_exists(sja::description_error)) {
		stdef = jd.get_attribute(sja::description_error);
		if (!(stdef.size() > 0)){
			stdef = "saga-app.e" + id;
		}
	}
	else {
		stdef = "saga-app.e" + id;
	}

	//Check Working Directory
	std::string wkdir;
	std::string home_pth = getenv("HOME");
	if (jd.attribute_exists(sja::working_directory)) {
		wkdir = jd.get_attribute(sja::working_directory);
		if (wkdir.size() > 0) {
			// Absolute path ?
			if (wkdir.substr(0,1) != "/"){
				wkdir = home_pth + "/" + wkdir;
			}
			if (wkdir.substr(wkdir.size()-1,1) != "/"){
				wkdir += "/";
			}
		}
		else {
			wkdir = home_pth + "/";
		}
	}
	else {
		wkdir = home_pth + "/";
	}

	// Open Standard Error file
	stdef = wkdir + stdef;
	std::ifstream ifs(stdef.c_str());
	std::ostringstream os;
	std::string str;

	if (ifs.fail()){
		str = "Cannot find stderr file." ;
	}
	else {
		while(ifs && getline(ifs, str)) {
			os << str << std::endl;
	  }
	}

	std::istringstream is_str(os.str());
	saga::job::istream is;
	is.copyfmt(is_str);
	is.clear(is_str.rdstate());
	is.rdbuf(is_str.rdbuf());

	ret = is;
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
    saga::job::state state = current_state(proxy_);
    if (state != saga::job::New) {
        SAGA_ADAPTOR_THROW("The job has already been started!",
                           saga::IncorrectState);
    }

    instance_data data(this);
    adaptor_data_type ad(this);

    saga::job::description& jd = data->jd_;

    std::string bin_pth(ad->get_binary_path());
    cli::qsub qsub(localhost, bin_pth);

    std::string pbsid;
    std::ostringstream os;
    bool success = qsub.execute(jd, pbsid, os);
    if (!success) 
    {
      std::string msg = os.str();
 
      SAGA_ADAPTOR_THROW(msg, saga::NoSuccess);
    }

    std::string sagaid = jobid_converter.convert_jobid(pbsid);
    saga::adaptors::attribute attr(this);
    attr.set_attribute(sja::jobid, sagaid);

    ad->register_job(pbsid, jd);

    update_state(proxy_, saga::job::Running);
  }

  void job_cpi_impl::sync_cancel (saga::impl::void_t & ret,
                                  double timeout)
  {
    // please check user@host
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
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
    m.fire();
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

} // namespace torque_job
////////////////////////////////////////////////////////////////////////

