//  Copyright (c) 2005-2009 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_OMII_GRIDSAM_JOB_HPP
#define ADAPTORS_OMII_GRIDSAM_JOB_HPP

#include <string>

//#include <saga/saga/util.hpp>
#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/saga/adaptors/adaptor_data.hpp>

#include <saga/impl/engine/proxy.hpp>
#include <saga/impl/packages/job/job_cpi.hpp>

#include "omii_gridsam_adaptor.hpp"

///////////////////////////////////////////////////////////////////////////////
// forward declaration only
class state_setter;
class JobSubmission;

class omii_gridsam_job 
    : public saga::adaptors::v1_0::job_cpi<omii_gridsam_job>
{
private:
    // base class
    typedef saga::adaptors::v1_0::job_cpi<omii_gridsam_job> base_cpi;

    // adaptor data
    typedef saga::adaptors::adaptor_data<omii_gridsam_adaptor> adaptor_data_type;

    std::string jobid_;       // jobid identifying the remote job
    std::string endpoint_;    // (fallback) url to be used to connect to GRIDSAM
    
    friend class state_setter;
    
public:
    typedef base_cpi::mutex_type mutex_type;
    
public:
    /// constructor of the job cpi 
    omii_gridsam_job  (proxy* p, cpi_info const& info, 
        saga::ini::ini const& glob_ini, saga::ini::ini const& adap_ini,
        TR1::shared_ptr<saga::adaptor> adaptor);

    /// destructor of the job cpi 
    ~omii_gridsam_job (void);

    // CPI functions
    void sync_get_state (saga::job::state& state);
    void sync_get_job_description(saga::job::description& jd);
    void sync_get_job_id (std::string& jobid);

    // inherited from the task interface
    void sync_run (saga::impl::void_t&);
    void sync_cancel (saga::impl::void_t&, double timeout);
    void sync_wait (bool&, double wait);
    void sync_suspend (saga::impl::void_t&);
    void sync_resume (saga::impl::void_t&);
    
    void sync_get_stdin(saga::job::ostream& ostrm);
    void sync_get_stdout(saga::job::istream& istrm);
    void sync_get_stderr(saga::job::istream& errstrm);
    
//     void sync_checkpoint(saga::impl::void_t&);
    void sync_migrate(saga::impl::void_t&, saga::job::description jd);
//     void sync_signal(saga::impl::void_t&, int signal_type);

protected:
    saga::job::state get_state();
    std::string get_state_detail();
    void update_state(saga::job::state newstate);
    void update_exit_code(int exit_code);

    bool fill_jobdescription(std::string jobid, saga::job::description& jd);

    // register our jobid with the adaptor data
    void register_jobid(bool mayfail = false);

    void initialize_staging (saga::job::description jd, JobSubmission& submit);
}; 

#endif // ADAPTORS_OMII_GRIDSAM_JOB_HPP

