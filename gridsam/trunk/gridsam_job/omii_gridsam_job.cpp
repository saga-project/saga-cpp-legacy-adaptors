//  Copyright (c) 2005-2009 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "stdsoap2.h"   // needs to be included first

#include <saga/saga/util.hpp>
#include <saga/saga/exception.hpp>
#include <saga/saga/url.hpp>

#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/metric.hpp>
#include <saga/saga/adaptors/attribute.hpp>
#include <saga/saga/adaptors/file_transfer_spec.hpp>
#include <saga/saga/packages/job/adaptors/job_self.hpp>

#include <saga/impl/config.hpp>

#include <saga/saga/packages/job/job_description.hpp>

#include "omii_gridsam_job.hpp"
#include "common_helpers.hpp"
#include "gridsam_jobsubmission.hpp"
#include "gridsam_jobmonitoring.hpp"
#include "gridsam_jobcontrol.hpp"
#include "gridsam_job_istream.hpp"
#include "gridsam_job_ostream.hpp"

#ifndef MAX_PATH
#define MAX_PATH _POSIX_PATH_MAX
#endif

///////////////////////////////////////////////////////////////////////////
class state_setter 
{
public:
    state_setter(omii_gridsam_job *cpi, bool commit_state = true)
      : commit_state_(commit_state), state_(saga::job::Unknown), 
        exit_code_(0), cpi_(cpi)
    {
        state_ = cpi_->get_state();
    }
    ~state_setter()
    {
        if (commit_state_) {
            cpi_->update_state(state_);
            if (state_ == saga::job::Done)
                cpi_->update_exit_code(exit_code_);
        }
    }

    bool commit_state_;
    saga::job::state state_;
    int exit_code_;

private:
    omii_gridsam_job *cpi_;
};

///////////////////////////////////////////////////////////////////////////////
omii_gridsam_job::omii_gridsam_job (proxy* p, cpi_info const& info,
        saga::ini::ini const& glob_ini, saga::ini::ini const& adap_ini,
        TR1::shared_ptr<saga::adaptor> adaptor)
  : base_cpi (p, info, adaptor, cpi::Noflags)
{
    // initialize instance data
    std::string gridsam_id;         // jobid assigned by gridsam
    bool init_from_jobid = false;

    {
        // check if we can handle this request
        instance_data inst_data(this);
        std::string rm(inst_data->rm_.get_url());
        if (rm.empty()) 
            rm = get_resourcemanager(adap_ini);

        if (rm.empty()) {
            SAGA_ADAPTOR_THROW("No resource manager connect point given. Consider "
                "adding a preference 'connect = <url>' to your adaptor ini file.", 
                saga::NotImplemented);
        }

        saga::url rm_url(rm);
        std::string scheme(rm_url.get_scheme());
        if (scheme.empty() || 
            (scheme != "gridsam" && scheme != "any" && scheme != "https"))
        {
          SAGA_OSSTREAM strm;
          strm << "Could not initialize job for [" << inst_data->rm_ << "]. " 
          << "Only any://, gridsam:// and https:// schemes are supported.";

          SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                             saga::adaptors::AdaptorDeclined);
        }
        if (rm_url.get_host().empty())
        {
          SAGA_OSSTREAM strm;
          strm << "Could not initialize job for [" << inst_data->rm_ << "]. " 
               << "No hostname given.";

            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                saga::BadParameter);
        }

        // make sure rm is complete and usable
        endpoint_ = ensure_resourcemanager(rm);

        if (inst_data->init_from_jobid_) {
        // This cpi instance has to be attached to a running resource manager 
        // job. The difference to the code below is, that SAGA does not know
        // anything about this job except its jobid. We need to fill the 
        // job description from the data we may be able to get from the 
        // resource manager.
            init_from_jobid = true;
            std::string jobid = inst_data->jobid_;
            if (jobid.empty()) {
                SAGA_ADAPTOR_THROW(
                    "Cannot attach to a job without a given jobid",
                    saga::adaptors::Unexpected);
            }

            // extract the process id from the given handle
            if (!extract_jobid(jobid, gridsam_id)) {
                SAGA_ADAPTOR_THROW("Job does not exist: " + jobid, 
                    saga::adaptors::Unexpected);
            }

            // initialize the jobid attribute
            saga::adaptors::attribute attr (this);
            jobid_ = jobid;
            attr.set_attribute(saga::job::attributes::jobid, jobid);

            inst_data->jd_is_valid_ = fill_jobdescription(jobid, inst_data->jd_);
        }
    }

    // initialize adaptor instance inst_data
    state_setter setstate (this);

    {
        // the first assumption is that we're running
        if (init_from_jobid) 
            setstate.state_ = saga::job::Running;

        if (!gridsam_id.empty()) {
        // this cpi instance has to be attached to a running child, SAGA
        // doesn't know anything about this child yet
            saga::job::state state = saga::job::Unknown; //state_from_gridsam(gridsam_id, endpoint_);

            mutex_type::scoped_lock lock(mtx_);
            setstate.state_ = state;
            if (saga::job::Running == setstate.state_) 
                register_jobid(true);   // register the jobid with this cpi
        } 
        else {
        // child has not been started yet
            mutex_type::scoped_lock lock(mtx_);
            setstate.state_ = saga::job::New;
        }
    }

    // if the job is (was) running, we need to have a jobid as well
    BOOST_ASSERT(setstate.state_ == saga::job::New || !jobid_.empty());
}

omii_gridsam_job::~omii_gridsam_job (void)
{
}

///////////////////////////////////////////////////////////////////////////////
void omii_gridsam_job::update_state(saga::job::state newstate)
{
    // update state attribute and promote state change to the metric callbacks
    saga::monitorable monitor (this->proxy_);
    saga::adaptors::metric m (monitor.get_metric(saga::metrics::task_state));
    m.set_attribute(saga::attributes::metric_value, 
        saga::adaptors::job_state_enum_to_value(newstate));
    m.fire();

    if (!jobid_.empty())
    {
        // update state detail metric
        m = monitor.get_metric(saga::job::metrics::state_detail);
        m.set_attribute(saga::attributes::metric_value, get_state_detail());
        m.fire();
    }
}

void omii_gridsam_job::update_exit_code(int exit_code)
{
    // update exitcode attribute 
    saga::adaptors::attribute attr (this);
    attr.set_attribute(saga::job::attributes::exitcode, 
        boost::lexical_cast<std::string>(exit_code));
}

///////////////////////////////////////////////////////////////////////////////
saga::job::state omii_gridsam_job::get_state()
{
    // get state attribute 
    saga::monitorable monitor (this->proxy_);
    saga::metric m (monitor.get_metric(saga::metrics::task_state));
    return saga::adaptors::job_state_value_to_enum(
        m.get_attribute(saga::attributes::metric_value));
}

///////////////////////////////////////////////////////////////////////////////
bool get_omii_gridsam_contexts(saga::session s, std::vector<saga::context>& ctxs_)
{
    std::vector<saga::context> ctxs(s.list_contexts());
    std::vector<saga::context>::iterator end = ctxs.end();
    for (std::vector<saga::context>::iterator it = ctxs.begin(); it != end; ++it) 
    {
        if (!(*it).attribute_exists(saga::attributes::context_type))
            continue;

        if ("omii_gridsam" == (*it).get_attribute(saga::attributes::context_type))
            ctxs_.push_back(*it);
    }
    return !ctxs_.empty();
}

///////////////////////////////////////////////////////////////////////////////
//  SAGA API functions
void omii_gridsam_job::sync_get_state (saga::job::state& state)
{
    saga::job::state current_state = saga::job::Unknown;
    std::string error;

    try {
        std::string remote_id;
        if (!extract_jobid(jobid_, remote_id)) {
            error = "Job does not exist: " + jobid_;
        }
        else {
            saga::session s(this->proxy_->get_session());
            std::vector<saga::context> ctxs;
            if (!get_omii_gridsam_contexts(s, ctxs))
            {
                SAGA_ADAPTOR_THROW(
                    "Problem in accessing security context for: " + jobid_,
                    saga::NoSuccess);
            }

            std::string desc;
            //int exit_code = 0; // not used at all
            current_state = state_from_gridsam(this, endpoint_, ctxs, remote_id);
        }
    }
    catch (saga::adaptors::exception const&) {
        throw;      // just rethrow our adaptor exceptions
    }
    catch (std::exception const& e) {
        SAGA_ADAPTOR_THROW(
            "Problem in accessing job state for: " + jobid_ + 
                " (std::exception caught: " + e.what() + ")",
            saga::NoSuccess);
    }

    // throw error, if appropriate
    if (!error.empty()) {
        SAGA_ADAPTOR_THROW("Problem in accessing job state for: " 
            + jobid_ + ": " + error, saga::NoSuccess)
    }

    // update state in local metric
    update_state(current_state);
    state = current_state;
}

std::string omii_gridsam_job::get_state_detail()
{
    std::string state_detail;
    std::string error;

    try {
        std::string remote_id;
        if (!extract_jobid(jobid_, remote_id)) {
            error = "Job does not exist: " + jobid_;
        }
        else {
            saga::session s(this->proxy_->get_session());
            std::vector<saga::context> ctxs;
            if (!get_omii_gridsam_contexts(s, ctxs))
            {
                SAGA_ADAPTOR_THROW(
                    "Problem in accessing security context for: " + jobid_,
                    saga::NoSuccess);
            }

            JobMonitoring monitor(this, endpoint_, ctxs, remote_id);
            if (SOAP_OK != monitor.getJobStatusDetails(state_detail)) 
                error = monitor.error();
        }
    }
    catch (saga::adaptors::exception const&) {
        throw;      // just rethrow our adaptor exceptions
    }
    catch (std::exception const& e) {
        SAGA_ADAPTOR_THROW(
            "Problem in accessing job state details for: " + jobid_ + 
                " (std::exception caught: " + e.what() + ")",
            saga::NoSuccess);
    }

    // throw error, if appropriate
    if (!error.empty()) {
        SAGA_ADAPTOR_THROW("Problem in accessing job state details for: " 
            + jobid_ + ": " + error, saga::NoSuccess)
    }
    return state_detail;
}

void omii_gridsam_job::sync_get_job_description(saga::job::description& jd)
{
    instance_data data(this);
    if (data->jd_is_valid_)
        jd = data->jd_.clone();     // return a deep copy of the job description
    else {
        SAGA_ADAPTOR_THROW("Job description cannot be retrieved.", 
            saga::NotImplemented);
    }
}

void omii_gridsam_job::sync_get_job_id (std::string& jobid)
{
    // verify this job is running already (not New)
    saga::job::state state = get_state();
    if (saga::job::New == state) {
        SAGA_ADAPTOR_THROW("Job has not been started yet!", 
            saga::IncorrectState);
    }

    saga::attribute attr (this->proxy_);
    jobid = attr.get_attribute(saga::job::attributes::jobid);
    BOOST_ASSERT(jobid == jobid_);
}

///////////////////////////////////////////////////////////////////////////////
// inherited from the task interface

// handle staging attributes
void 
omii_gridsam_job::initialize_staging (saga::job::description jd, 
    JobSubmission& submit)
{
    if (!jd.attribute_exists(saga::job::attributes::description_file_transfer))
        return;

    // get the staging specifications
    std::vector<std::string> specs (
        jd.get_vector_attribute(saga::job::attributes::description_file_transfer));

    std::vector<std::string>::iterator end = specs.end();
    for (std::vector<std::string>::iterator it = specs.begin(); it != end; ++it)
    {
        using namespace saga::adaptors;
        std::string left_url, right_url;
        saga::adaptors::file_transfer_operator mode;
        if (!parse_file_transfer_specification(*it, left_url, mode, right_url))
        {
            SAGA_ADAPTOR_THROW(
                "omii_gridsam_job::initialize_staging: "
                    "ill formatted file transfer specification: " + *it,
                saga::BadParameter);
        }

        switch (mode) {
        case copy_local_remote:
            submit.add_StageInStep(left_url, right_url, true);
            break;

        case append_local_remote:
            submit.add_StageInStep(left_url, right_url, false);
            break;

        case copy_remote_local:
            submit.add_StageOutStep(left_url, right_url, true);
            break;

        case append_remote_local:
            submit.add_StageOutStep(left_url, right_url, false);
            break;

        default:
            SAGA_ADAPTOR_THROW(
                "omii_gridsam_job::initialize_staging: "
                    "ill formatted file transfer mode: " + *it,
                saga::BadParameter);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void omii_gridsam_job::sync_run (saga::impl::void_t&)
{
    // verify this job is not running yet (has to be New)
    saga::job::state state = get_state();
    if (saga::job::New != state) {
        SAGA_ADAPTOR_THROW("Job has been started already!", 
            saga::IncorrectState);
    }

    saga::job::description jd;
    {
        instance_data data(this);
        if (!data->jd_is_valid_) {
            SAGA_ADAPTOR_THROW("Job description cannot be retrieved.", 
                saga::NotImplemented);
        }
        jd = data->jd_;
    }

    std::string exename(
        jd.get_attribute(saga::job::attributes::description_executable));

    try {
        saga::session s(this->proxy_->get_session());
        std::vector<saga::context> ctxs;
        if (!get_omii_gridsam_contexts(s, ctxs))
        {
            SAGA_ADAPTOR_THROW(
                "Problem in accessing security context for: " + jobid_,
                saga::NoSuccess);
        }

        JobSubmission submit(this, endpoint_, ctxs, exename);
        
        // collect all job description related information
        if (jd.attribute_exists(saga::job::attributes::description_arguments))
        {
            std::vector<std::string> arguments = jd.get_vector_attribute(
                saga::job::attributes::description_arguments);
            submit.set_arguments(arguments);
        }

        // set up environment
        if (jd.attribute_exists(saga::job::attributes::description_environment))
        {
            std::vector<std::string> env = jd.get_vector_attribute(
                    saga::job::attributes::description_environment);

            std::map<std::string, std::string> keyval;
            std::vector<std::string>::iterator end = env.end();
            for (std::vector<std::string>::iterator it = env.begin(); 
                it != end; ++it)
            {
                std::string key, value;
                if (!split_environment(*it, key, value)) 
                {
                    SAGA_ADAPTOR_THROW(
                        "Bogus formatting of a environment entry: ':" + *it + "'",
                        saga::adaptors::Unexpected);
                }
                keyval.insert(std::make_pair(key, value));
            }
            submit.set_environment(keyval);
        }

        // set up I/O redirection
        if (jd.attribute_exists(saga::job::attributes::description_interactive) &&
            jd.get_attribute(saga::job::attributes::description_interactive) ==
                saga::attributes::common_true)
        {
            SAGA_ADAPTOR_THROW(
                "Interactive jobs are not supported by the Gridsam job "
                "submission service", saga::NotImplemented);
        }
        else {
            if (jd.attribute_exists(saga::job::attributes::description_input)) {
            // set up input file (used as stdin)
                submit.set_stdin_file(
                    jd.get_attribute(saga::job::attributes::description_input));
            }
            if (jd.attribute_exists(saga::job::attributes::description_output)) {
            // set up output file (used as stdout)
                submit.set_stdout_file(
                    jd.get_attribute(saga::job::attributes::description_output));
            }
            if (jd.attribute_exists(saga::job::attributes::description_error)) {
            // set up output file (used as stderr)
                submit.set_stderr_file(
                    jd.get_attribute(saga::job::attributes::description_error));
            }
        }

        // other job related attributes
        if (jd.attribute_exists(saga::job::attributes::description_number_of_processes))
        {
            int count = boost::lexical_cast<int>(
                jd.get_attribute(saga::job::attributes::description_number_of_processes));
            submit.set_ProcessCountLimit(count);
        }
        if (jd.attribute_exists(saga::job::attributes::description_threads_per_process))
        {
            int count = boost::lexical_cast<int>(
                jd.get_attribute(saga::job::attributes::description_threads_per_process));
            submit.set_ThreadCountLimit(count);
        }
        if (jd.attribute_exists(saga::job::attributes::description_total_cpu_count))
        {
            int count = boost::lexical_cast<int>(
                jd.get_attribute(saga::job::attributes::description_total_cpu_count));
            submit.set_TotalCPUCount(count);
        }
        if (jd.attribute_exists(saga::job::attributes::description_total_cpu_time))
        {
            int count = boost::lexical_cast<int>(
                jd.get_attribute(saga::job::attributes::description_total_cpu_time));
            submit.set_TotalCPUTime(count);
        }
        if (jd.attribute_exists(saga::job::attributes::description_total_physical_memory))
        {
            double memory = boost::lexical_cast<double>(
                jd.get_attribute(saga::job::attributes::description_total_physical_memory));
            submit.set_TotalPhysicalMemory(memory);
        }
        if (jd.attribute_exists(saga::job::attributes::description_cpu_architecture))
        {
            submit.set_CPUArchitecture(
                jd.get_attribute(saga::job::attributes::description_cpu_architecture));
        }
        if (jd.attribute_exists(saga::job::attributes::description_operating_system_type))
        {
            submit.set_OperatingSystem(
                jd.get_attribute(saga::job::attributes::description_operating_system_type));
        }
        if (jd.attribute_exists(saga::job::attributes::description_total_cpu_time))
        {
            int limit = boost::lexical_cast<int>(
                jd.get_attribute(saga::job::attributes::description_wall_time_limit));
            submit.set_WallTimeLimit(limit);
        }
        if (jd.attribute_exists(saga::job::attributes::description_candidate_hosts))
        {
            std::vector<std::string> hosts(
                jd.get_vector_attribute(saga::job::attributes::description_candidate_hosts));
            submit.set_CandidateHosts(hosts);
        }

        initialize_staging(jd, submit);

        saga::adaptors::attribute attr (this);

        // working directory
        if (jd.attribute_exists(saga::job::attributes::description_working_directory))
        {
            std::string wd (jd.get_attribute(
                saga::job::attributes::description_working_directory));
            submit.set_working_directory(wd);
            attr.set_attribute(saga::job::attributes::working_directory, wd);
        }
        else 
        {
            submit.set_working_directory(".");
            attr.set_attribute(saga::job::attributes::working_directory, ".");
        }

    // execute the new process
        state_setter setstate(this);
        std::string jobid;

        if (SOAP_OK != submit.submitJob(jobid))
        {
            // get rid of trailing newlines
            std::string desc(submit.error());
            std::string::size_type p = desc.find_last_of("\n");
            if (p == desc.size()-1)
                desc.erase(p);
                
            // report the error
            SAGA_ADAPTOR_THROW(
                "Could not start process: '" + exename + "' using endpoint: '" + 
                endpoint_ + "'\nGot GSOAP error: " + desc, 
                saga::NoSuccess);
        }

        mutex_type::scoped_lock lock(mtx_);
        std::string desc;
        int exit_code = 0;
        setstate.state_ = state_from_gridsam(this, endpoint_, ctxs, jobid, &desc, &exit_code);
        if (setstate.state_ != saga::job::Running && 
            setstate.state_ != saga::job::Done)
        {
            if (desc.empty()) {
                SAGA_ADAPTOR_THROW(
                    "Could not start process: '" + exename + "' using endpoint: '" + 
                    endpoint_, saga::NoSuccess);
            }
            else {
                // get rid of trailing newlines
                std::string::size_type p = desc.find_last_of("\n");
                if (p == desc.size()-1)
                    desc.erase(p);
                    
                // report the error
                SAGA_ADAPTOR_THROW(
                    "Could not start process: '" + exename + "' using endpoint: '" + 
                    endpoint_ + "'\nGot Gridsam error: " + desc, saga::NoSuccess);
            }
        }

        if (saga::job::Done == setstate.state_ || 
            saga::job::Failed == setstate.state_) 
        {
            setstate.exit_code_ = exit_code;
        }

        // set the job id of the newly created process
        SAGA_OSSTREAM strm;
        strm << "[" << endpoint_ << "]-[" 
             << ensure_hostname(endpoint_) << ":" << jobid << "]";
        jobid_ = SAGA_OSSTREAM_GETSTRING(strm);
        attr.set_attribute(saga::job::attributes::jobid, jobid_);

        // set other required attributes of the new job
        std::vector<std::string> hosts;
        hosts.push_back(ensure_hostname(endpoint_));
        attr.set_vector_attribute(saga::job::attributes::execution_hosts, hosts);

        std::time_t started = 0;
        std::time(&started);
        attr.set_attribute(saga::job::attributes::started, ctime(&started));

        // register this job with the adaptor data
        register_jobid();
    }
    catch (saga::adaptors::exception const&) {
        throw;      // just rethrow our adaptor exceptions
    }
    catch (std::exception const& e) {
        SAGA_ADAPTOR_THROW(
            "Problem in starting process: " + exename + 
                " (std::exception caught: " + e.what() + ")",
            saga::NoSuccess);
    }
}

//  terminate the child process (if any)
void omii_gridsam_job::sync_cancel (saga::impl::void_t&, double timeout)
{
    saga::job::state current_state = saga::job::Unknown;
    std::string error;
    
    try {
        std::string remote_id;
        if (!extract_jobid(jobid_, remote_id)) {
            error = "Job does not exist: " + jobid_;
        }
        else {
            saga::session s(this->proxy_->get_session());
            std::vector<saga::context> ctxs;
            if (!get_omii_gridsam_contexts(s, ctxs))
            {
                SAGA_ADAPTOR_THROW(
                    "Problem in accessing security context for: " + jobid_,
                    saga::NoSuccess);
            }

            JobControl control(this, endpoint_, ctxs, remote_id);
            if (SOAP_OK != control.terminateJob(current_state)) 
                error = control.error();
        }
    }
    catch (saga::adaptors::exception const&) {
        throw;      // just rethrow our adaptor exceptions
    }
    catch (std::exception const& e) {
        SAGA_ADAPTOR_THROW(
            "Problem during job termination for: " + jobid_ + 
                " (std::exception caught: " + e.what() + ")",
            saga::NoSuccess);
    }

    // throw error, if appropriate
    if (!error.empty()) {
        SAGA_ADAPTOR_THROW("Problem during job termination for: " 
            + jobid_ + ": " + error, saga::NoSuccess)
    }

    // update state in local metric
    update_state(current_state);
}

//  wait for the child process to terminate
void omii_gridsam_job::sync_wait (bool& retval, double timeout)
{
    // use a consistent value throughout this routine
    saga::job::state state = saga::job::Unknown;
    sync_get_state(state);

    if (saga::job::Canceled == state) {
        SAGA_THROW("Job not running: task was canceled!",
            saga::IncorrectState);
    }
    if (saga::job::New == state) {
        SAGA_THROW("Job not running, yet: is still pending!",
            saga::IncorrectState);
    }

    // if the task has finished, we return immediately
    if (saga::job::Done == state || saga::job::Failed == state) {
        retval = true;
        return;
    }

    // --> 1st case: wait until the task has finished
    if (timeout < 0.0) {
        // blocking call, in order to wait for the thread to finish!
        sync_get_state(state);
        while (saga::job::Running == state)
        {
            // we consider 100msec as a good slice
            saga::impl::task_base::sleep (100);
            sync_get_state(state);
        }
        retval = true;
        return;
    }

    // --> 2nd case: wait for timeout seconds and return the finished tasks
    if (timeout > 0.0)
    {
        std::time_t start_time = std::time(0);
        while ((timeout - (std::difftime(std::time(0), start_time))) > 0)
        {
            sync_get_state(state);
            if (saga::job::Running != state) {
                retval = true;
                return;
            }

            // we consider 100msec as a good slice
            saga::impl::task_base::sleep (100);
        }
    }

    // timeout == 0.0, or after waiting for timeout
    sync_get_state(state);
    retval = (saga::job::Running != state) ? true : false;
}

//  suspend the child process 
void omii_gridsam_job::sync_suspend (saga::impl::void_t&)
{
    SAGA_ADAPTOR_THROW(
        "The suspend operation is not supported by the Gridsam job "
        "submission service.", saga::NotImplemented);
}

//  suspend the child process 
void omii_gridsam_job::sync_resume (saga::impl::void_t&)
{
    SAGA_ADAPTOR_THROW(
        "The resume operation is not supported by the Gridsam job "
        "submission service.", saga::NotImplemented);
}

// access streams for communication with the child
void omii_gridsam_job::sync_get_stdin(saga::job::ostream& ostrm)
{
    omii_job_ostream strm;
    ostrm = strm;   // stream has eofbit set
}

void omii_gridsam_job::sync_get_stdout(saga::job::istream& istrm)
{
    omii_job_istream strm;
    istrm = strm;   // stream has eofbit set
}

void omii_gridsam_job::sync_get_stderr(saga::job::istream& errstrm)
{
    omii_job_istream strm;
    errstrm = strm;   // stream has eofbit set
}
    
void omii_gridsam_job::sync_migrate(saga::impl::void_t&, saga::job::description jd)
{
    SAGA_ADAPTOR_THROW(
        "The migrate operation is not supported by the Gridsam job "
        "submission service.", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
// initialize the job description from a given jobid
bool omii_gridsam_job::fill_jobdescription(std::string jobid, 
    saga::job::description& jd)
{
    adaptor_data_type data (this);
    if (!data->knows_job(jobid))
        return false;
    jd = data->get_job(this, jobid);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
void omii_gridsam_job::register_jobid(bool mayfail)
{
    BOOST_ASSERT(!jobid_.empty());
    
    saga::job::description jd;
    {
        instance_data data(this);
        if (data->jd_is_valid_)
            jd = data->jd_.clone();
        else if (mayfail)
            return;
        else {
            SAGA_ADAPTOR_THROW("Can't register job: " + jobid_, 
                saga::adaptors::Unexpected);
        }
    }
    
    adaptor_data_type data(this);
    if (!data->register_job(jobid_, jd) && !mayfail) {
        SAGA_ADAPTOR_THROW("Can't register job: " + jobid_, 
            saga::adaptors::Unexpected);
    }
}

