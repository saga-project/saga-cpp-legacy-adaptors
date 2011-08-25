//  Copyright (c) 2007-2008 Ole Christian Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_JOB_SERVICE_HPP
#define ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_JOB_SERVICE_HPP

#include "globus_gram_job_adaptor.hpp"

#include <boost/ptr_container/ptr_map.hpp>

#include <saga/saga/adaptors/adaptor_data.hpp>
#include <saga/impl/packages/job/job_service_cpi.hpp>

///////////////////////////////////////////////////////////////////////////////
//
namespace globus_gram_job_adaptor {
    
    class job_service_cpi_impl 
    : public saga::adaptors::v1_0::job_service_cpi<job_service_cpi_impl>
    {
        
    private:
        
        typedef saga::adaptors::v1_0::job_service_cpi<job_service_cpi_impl> 
        base_cpi;
        
        typedef saga::adaptors::adaptor_data<job_adaptor> adaptor_data_type;
                
    public:
        
        ///////////////////////////////////////////////////////////////////////
        ////////////////////////// SERVICE C'TOR D'TOR ////////////////////////
        ///////////////////////////////////////////////////////////////////////
        
        job_service_cpi_impl        (proxy                * p, 
                                     cpi_info       const & info,
                                     saga::ini::ini const & glob_ini, 
                                     saga::ini::ini const & adap_ini,
                                     TR1::shared_ptr<saga::adaptor> adaptor);
        
        ~job_service_cpi_impl       (void);
        
        ///////////////////////////////////////////////////////////////////////
        //////////////////////////// SERVICE METHODS //////////////////////////
        ///////////////////////////////////////////////////////////////////////
        
        void sync_create_job        (saga::job::job & ret, 
                                     saga::job::description jd);
        
        void sync_run_job           (saga::job::job & ret, 
                                     std::string commandline, 
                                     std::string host, 
                                     saga::job::ostream& in, 
                                     saga::job::istream& out, 
                                     saga::job::istream& err);

        void sync_run_job_noio      (saga::job::job & ret, 
                                     std::string commandline,
                                     std::string host);
        
        void sync_list              (std::vector<std::string>& list_of_jobids);
        
        void sync_get_job           (saga::job::job& job, 
                                     std::string jobid);
        
        //void sync_get_self          (saga::job::self& self);

    }; 
    
}; 

//
///////////////////////////////////////////////////////////////////////////////

#endif // ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_JOB_SERVICE_HPP

