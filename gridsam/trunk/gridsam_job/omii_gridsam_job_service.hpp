//  Copyright (c) 2005-2009 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_OMII_GRIDSAM_JOB_SERVICE_HPP
#define ADAPTORS_OMII_GRIDSAM_JOB_SERVICE_HPP

#include <string>
#include <iosfwd>

#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/saga/adaptors/adaptor_data.hpp>

#include <saga/impl/engine/proxy.hpp>
#include <saga/impl/packages/job/job_service_cpi.hpp>

#include "omii_gridsam_adaptor.hpp"

///////////////////////////////////////////////////////////////////////////////
class omii_gridsam_job_service
    : public saga::adaptors::v1_0::job_service_cpi<omii_gridsam_job_service>
{
private:
    // base class
    typedef saga::adaptors::v1_0::job_service_cpi<omii_gridsam_job_service> 
        base_cpi;

    // adaptor data
    typedef saga::adaptors::adaptor_data<omii_gridsam_adaptor> adaptor_data_type;

    std::string endpoint_;    // resource manager
    
public:
    /// constructor of the job_service cpi
    omii_gridsam_job_service (proxy* p, cpi_info const& info,
        saga::ini::ini const& glob_ini, saga::ini::ini const& adap_ini,
        TR1::shared_ptr<saga::adaptor> adaptor);

    /// destructor of the job_service cpi 
    ~omii_gridsam_job_service (void);

    // CPI functions
    void sync_create_job(saga::job::job & ret, saga::job::description jd);
    void run_job_noio(saga::job::job & ret, std::string commandline, std::string host);
    void sync_run_job(saga::job::job & ret, std::string commandline, std::string host,
                      saga::job::ostream& in, saga::job::istream& out, 
        saga::job::istream& err);
    void sync_list(std::vector<std::string>& list_of_jobids);
    void sync_get_job(saga::job::job& job, std::string jobid);
    void sync_get_self(saga::job::self& self);
};  

#endif // ADAPTORS_OMII_GRIDSAM_JOB_SERVICE_HPP

