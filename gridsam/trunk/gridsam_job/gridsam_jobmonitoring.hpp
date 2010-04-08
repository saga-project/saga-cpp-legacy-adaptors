//  Copyright (c) 2005-2009 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(ADAPTOR_GRIDSAM_JOBMONITORING_HK20070910_1217PM)
#define ADAPTOR_GRIDSAM_JOBMONITORING_HK20070910_1217PM

#include <string>
#include <vector>
#include <stdexcept>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include <saga/saga/job.hpp>

#include "common_helpers.hpp"
#include "gsoap_helper.hpp"
#include "stubs/gridsam/gridsamJobMonitoringSOAPBindingProxy.h"

///////////////////////////////////////////////////////////////////////////////
namespace util
{
    ///////////////////////////////////////////////////////////////////////////
    class MonitorJobRequest 
    {
    public:
        MonitorJobRequest(soap* soap, std::string const& jobid) 
          : registry_(soap),
            request_(registry_.create(soap_new__gridsam__getJobStatus, 
                                      soap_delete__gridsam__getJobStatus))
        {
            // create and initialize job identifier instance
            gridsam__JobIdentifierType* jid = registry_.create(
                soap_new_gridsam__JobIdentifierType,
                soap_delete_gridsam__JobIdentifierType);
            jid->ID = jobid;

            request_->gridsam__JobIdentifier.push_back(jid);
        }
        ~MonitorJobRequest() {}
        
        _gridsam__getJobStatus* get() { return request_; }
        
    private:
        soap_registry registry_;
        _gridsam__getJobStatus* request_;
    };

    ///////////////////////////////////////////////////////////////////////////
    saga::job::state saga_state_from_gridsam_state(gridsam__StateType state)
    {
        saga::job::state result = saga::job::Unknown;
        switch(state) {
        case gridsam__StateType__pending:
        case gridsam__StateType__staging_in:
        case gridsam__StateType__staged_in:
        case gridsam__StateType__staging_out:
        case gridsam__StateType__staged_out:
        case gridsam__StateType__active:
        case gridsam__StateType__executed:
            result = saga::job::Running;
            break;
            
        case gridsam__StateType__terminating:
        case gridsam__StateType__terminated:
            result = saga::job::Canceled;
            break;
        
        case gridsam__StateType__failed:
            result = saga::job::Failed;
            break;

        case gridsam__StateType__done:
            result = saga::job::Done;
            break;

        case gridsam__StateType__undefined:
            break;
        }
        return result;
    }
    
    std::string saga_state_details_from_gridsam_state(gridsam__StateType state,
        int exitcode)
    {
        switch(state) {
        case gridsam__StateType__pending:     return "BES:Pending";
        case gridsam__StateType__staging_in:  return "BES:StagingIn";
        case gridsam__StateType__staged_in:   return "BES:StagedIn";
        case gridsam__StateType__staging_out: return "BES:StagingOut";
        case gridsam__StateType__staged_out:  return "BES:StagedOut";
        case gridsam__StateType__active:      return "BES:Active";
        case gridsam__StateType__executed:    return "BES:Executed";
        case gridsam__StateType__failed:      return "BES:Failed";
        case gridsam__StateType__done:
            // The exitcode '126' seems to be the way of the GridSAM service to 
            // report non-existing executables.
            if (exitcode == 126)
                return "BES:Failed (couldn't find executable)";
            return "BES:Done";
        case gridsam__StateType__terminating: return "BES:Terminating";
        case gridsam__StateType__terminated:  return "BES:Terminated";
        case gridsam__StateType__undefined:   
            break;
        }
        return "BES:Undefined";
    }
    
    class MonitorJobResponse 
    {
    public:
        MonitorJobResponse(soap* soap, std::string const& jobid) 
          : jobid_(jobid), registry_(soap),
            response_(registry_.create(soap_new__gridsam__getJobStatusResponse, 
                                       soap_delete__gridsam__getJobStatusResponse))
        {
        }
        ~MonitorJobResponse() {}
        
        _gridsam__getJobStatusResponse* get() { return response_; }

        saga::job::state get_job_state() const
        {
            if (1 != response_->gridsam__JobStatus.size()) {
                throw std::runtime_error("MonitorJobResponse::get_job_state: "
                    "got invalid job state information");
            }
            gridsam__JobStatusType* s = response_->gridsam__JobStatus[0];
            if (s->gridsam__JobIdentifier->ID != jobid_) {
                throw std::runtime_error("MonitorJobResponse::get_job_state: "
                    "jobid mismatch: we asked for '" + jobid_ + "' but got: '" + 
                    s->gridsam__JobIdentifier->ID + "'");
            }
            std::size_t stages = s->gridsam__Stage.size();
            if (0 == stages) {
                throw std::runtime_error("MonitorJobResponse::get_job_state: "
                    "got invalid job state information");
            }
            _gridsam__Stage* last_stage = s->gridsam__Stage[stages-1];
            return saga_state_from_gridsam_state(last_stage->State);
        }

        gridsam__StateType get_state() const
        {
            if (1 != response_->gridsam__JobStatus.size()) {
                throw std::runtime_error("MonitorJobResponse::get_state: got "
                    "invalid job state information");
            }
            gridsam__JobStatusType* s = response_->gridsam__JobStatus[0];
            if (s->gridsam__JobIdentifier->ID != jobid_) {
                throw std::runtime_error("MonitorJobResponse::get_state: "
                    "jobid mismatch: we asked for '" + jobid_ + "' but got: '" + 
                    s->gridsam__JobIdentifier->ID + "'");
            }
            std::size_t stages = s->gridsam__Stage.size();
            if (0 == stages) {
                throw std::runtime_error("MonitorJobResponse::get_state: got "
                    "invalid job state information");
            }
            return s->gridsam__Stage[stages-1]->State;
        }

        std::string get_job_state_description() const
        {
            if (1 != response_->gridsam__JobStatus.size()) {
                throw std::runtime_error(
                    "MonitorJobResponse::get_job_state_description: got "
                    "invalid job state information");
            }
            gridsam__JobStatusType* s = response_->gridsam__JobStatus[0];
            if (s->gridsam__JobIdentifier->ID != jobid_) {
                throw std::runtime_error(
                    "MonitorJobResponse::get_job_state_description: "
                    "jobid mismatch: we asked for '" + jobid_ + "' but got: '" + 
                    s->gridsam__JobIdentifier->ID + "'");
            }
            std::size_t stages = s->gridsam__Stage.size();
            if (0 == stages) {
                throw std::runtime_error(
                    "MonitorJobResponse::get_job_state_description: got "
                    "invalid job state information");
            }
            _gridsam__Stage* last_stage = s->gridsam__Stage[stages-1];
            return last_stage->Description;
        }

        int get_job_exit_code() const
        {
            if (1 != response_->gridsam__JobStatus.size()) {
                throw std::runtime_error(
                    "MonitorJobResponse::get_job_exit_code: got "
                    "invalid job state information");
            }
            gridsam__JobStatusType* s = response_->gridsam__JobStatus[0];
            if (s->gridsam__JobIdentifier->ID != jobid_) {
                throw std::runtime_error(
                    "MonitorJobResponse::get_job_exit_code: "
                    "jobid mismatch: we asked for '" + jobid_ + "' but got: '" + 
                    s->gridsam__JobIdentifier->ID + "'");
            }
            std::size_t stages = s->gridsam__Stage.size();
            if (0 == stages) {
                throw std::runtime_error(
                    "MonitorJobResponse::get_job_exit_code: got "
                    "invalid job state information");
            }

            int exit_code = 0;
            for (std::size_t i = 0; i < stages; ++i)
            {
                if (s->gridsam__Stage[i]->State == gridsam__StateType__executed)
                {
                    std::string msg = s->gridsam__Stage[i]->Description;
                    std::string::size_type p = msg.find_last_of(" ");
                    if (p == std::string::npos) {
                        throw std::runtime_error(
                            "MonitorJobResponse::get_job_exit_code: got "
                            "invalid job state information for Done state");
                    }
                    exit_code = boost::lexical_cast<int>(msg.substr(p+1));
                }
            }
            return exit_code;
        }

    private:
        std::string jobid_;
        soap_registry registry_;
        _gridsam__getJobStatusResponse* response_;
    };

///////////////////////////////////////////////////////////////////////////////
}   // namespace util

///////////////////////////////////////////////////////////////////////////////
class JobMonitoring : public JobMonitoringSOAPBindingProxy
{
private:
    typedef JobMonitoringSOAPBindingProxy base_type;
    JobMonitoring* this_() { return this; }
    
public:
    JobMonitoring(saga::impl::v1_0::cpi* cpi, std::string const& endpoint, 
            std::vector<saga::context> const& ctxs, std::string const& jobid) 
      : endpoint_(endpoint), request_(this_(), jobid), response_(this_(), jobid)
    {
        this->soap_endpoint = endpoint_.c_str();
        
        saga::impl::exception_list exceptions;
        std::vector<saga::context>::const_iterator end = ctxs.end();
        for (std::vector<saga::context>::const_iterator it = ctxs.begin();
             it != end; ++it)
        {
            std::string certs, usercert, userkey, userpass;
            certs = retrieve_attribute(*it, saga::attributes::context_certrepository);
            usercert = retrieve_attribute(*it, saga::attributes::context_usercert);
            userkey = retrieve_attribute(*it, saga::attributes::context_userkey);
            userpass = retrieve_attribute(*it, saga::attributes::context_userpass);
            
            try {
                util::connect_to_gridsam(cpi, this, certs, usercert, 
                    userkey, userpass);
            }
            catch (saga::adaptors::exception const& e) {
                exceptions.add(e);
            }
        }
        
        if (exceptions.get_error_count())
        {
            SAGA_ADAPTOR_THROW_PLAIN_LIST(cpi, exceptions)
        }
    }
    ~JobMonitoring() 
    {}
    
    int getJobStatus(saga::job::state& state) 
    { 
        int result = base_type::getJobStatus(request_.get(), response_.get());
        if (SOAP_OK == result)
            state = response_.get_job_state();
        return result;
    }

    int getJobStatusDetails(std::string& state_detail) 
    { 
        int result = base_type::getJobStatus(request_.get(), response_.get());
        if (SOAP_OK == result) {
            state_detail = util::saga_state_details_from_gridsam_state(
                response_.get_state(), response_.get_job_exit_code());
        }
        return result;
    }

    int getJobStatusDescription(std::string& description) 
    { 
        int result = base_type::getJobStatus(request_.get(), response_.get());
        if (SOAP_OK == result) 
            description = response_.get_job_state_description();
        return result;
    }

    int getJobStatusExitCode(int& exitcode) 
    { 
        int result = base_type::getJobStatus(request_.get(), response_.get());
        if (SOAP_OK == result) 
            exitcode = response_.get_job_exit_code();
        return result;
    }

    std::string error()
    {
        char buffer[512] = { '\0' };
        soap_sprint_fault(buffer, sizeof(buffer));
        return buffer;
    }
    
private:
    std::string endpoint_;
    util::MonitorJobRequest request_;
    util::MonitorJobResponse response_;
};

///////////////////////////////////////////////////////////////////////////////
inline saga::job::state 
state_from_gridsam(saga::impl::v1_0::cpi* cpi, 
    std::string const& endpoint, std::vector<saga::context> const& ctxs, 
    std::string const& gridsam_id, std::string* desc = NULL,
    int* exit_code = NULL)
{
    JobMonitoring monitor(cpi, endpoint, ctxs, gridsam_id);
    saga::job::state state;

    if (SOAP_OK != monitor.getJobStatus(state)) 
        throw std::runtime_error(monitor.error());

    if (NULL != desc && SOAP_OK != monitor.getJobStatusDescription(*desc)) 
        throw std::runtime_error(monitor.error());

    int exitcode = 0;
    if (SOAP_OK != monitor.getJobStatusExitCode(exitcode)) 
        throw std::runtime_error(monitor.error());

    if (NULL != exit_code)
        *exit_code = exitcode;

    // The exitcode '126' seems to be the way of the GridSAM service to report 
    // non-existing executables.
    if (state == saga::job::Done && 126 == exitcode)
        state = saga::job::Failed;
    return state;
}

#endif

