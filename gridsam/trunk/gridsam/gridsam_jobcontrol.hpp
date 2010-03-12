//  Copyright (c) 2005-2009 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(ADAPTOR_GRIDSAM_JOBCONTROL_HK20071028_0156PM)
#define ADAPTOR_GRIDSAM_JOBCONTROL_HK20071028_0156PM

#include <string>
#include <vector>
#include <stdexcept>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include <saga/saga/job.hpp>

#include "common_helpers.hpp"
#include "gsoap_helper.hpp"
#include "stubs/gridsam/gridsamJobControlSOAPBindingProxy.h"

///////////////////////////////////////////////////////////////////////////////
namespace util
{
    ///////////////////////////////////////////////////////////////////////////
    class ControlJobRequest 
    {
    public:
        ControlJobRequest(soap* soap, std::string const& jobid) 
          : registry_(soap),
            request_(registry_.create(soap_new__gridsam__terminateJob, 
                                      soap_delete__gridsam__terminateJob))
        {
            // create and initialize job identifier instance
            gridsam__JobIdentifierType* jid = registry_.create(
                soap_new_gridsam__JobIdentifierType,
                soap_delete_gridsam__JobIdentifierType);
            jid->ID = jobid;

            request_->gridsam__JobIdentifier.push_back(jid);
        }
        ~ControlJobRequest() {}
        
        _gridsam__terminateJob* get() { return request_; }
        
    private:
        soap_registry registry_;
        _gridsam__terminateJob* request_;
    };

    class ControlJobResponse 
    {
    public:
        ControlJobResponse(soap* soap, std::string const& jobid) 
          : jobid_(jobid), registry_(soap),
            response_(registry_.create(soap_new__gridsam__terminateJobResponse, 
                                       soap_delete__gridsam__terminateJobResponse))
        {
        }
        ~ControlJobResponse() {}
        
        _gridsam__terminateJobResponse* get() { return response_; }

        saga::job::state get_job_state() const
        {
            if (1 != response_->gridsam__JobStatus.size()) {
                throw std::runtime_error("ControlJobResponse::get_job_state: "
                    "got invalid job state information");
            }
            gridsam__JobStatusType* s = response_->gridsam__JobStatus[0];
            if (s->gridsam__JobIdentifier->ID != jobid_) {
                throw std::runtime_error("ControlJobResponse::get_job_state: "
                    "jobid mismatch: we asked for '" + jobid_ + "' but got: '" + 
                    s->gridsam__JobIdentifier->ID + "'");
            }
            std::size_t stages = s->gridsam__Stage.size();
            if (0 == stages) {
                throw std::runtime_error("ControlJobResponse::get_job_state: "
                    "got invalid job state information");
            }
            _gridsam__Stage* last_stage = s->gridsam__Stage[stages-1];
            return saga_state_from_gridsam_state(last_stage->State);
        }

    private:
        std::string jobid_;
        soap_registry registry_;
        _gridsam__terminateJobResponse* response_;
    };

///////////////////////////////////////////////////////////////////////////////
}   // namespace util

///////////////////////////////////////////////////////////////////////////////
class JobControl : public JobControlSOAPBindingProxy
{
private:
    typedef JobControlSOAPBindingProxy base_type;
    JobControl* this_() { return this; }
    
public:
    JobControl(saga::impl::v1_0::cpi* cpi, std::string const& endpoint, 
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
    ~JobControl() 
    {}
    
    int terminateJob(saga::job::state& state) 
    {
        int result = base_type::terminateJob(request_.get(), response_.get());
        if (SOAP_OK == result)
            state = response_.get_job_state();
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
    util::ControlJobRequest request_;
    util::ControlJobResponse response_;
};

#endif
