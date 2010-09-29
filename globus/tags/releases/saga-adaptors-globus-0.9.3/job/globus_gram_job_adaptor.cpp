//  Copyright (c) 2007 Ole Christian Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "globus_gram_job_adaptor_job.hpp"
#include "globus_gram_job_adaptor.hpp"
#include "globus_gram_job_adaptor_service.hpp"

#include "globus_gram_job_adaptor_connector.hpp"
#include "globus_gram_job_adaptor_errorhandler.hpp"

#include <boost/function_output_iterator.hpp>

#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/config.hpp>
#include <saga/saga/adaptors/adaptor.hpp>

#include "../loader/globus_global_loader.hpp"

#include <map>
#include <vector>
#include <algorithm>

SAGA_ADAPTOR_REGISTER (job_adaptor);

using namespace globus_gram_job_adaptor;

job_adaptor::job_adaptor()
{    
    // load the required globus modules
    globus_module_loader::globus_init ();
}
    
job_adaptor::~job_adaptor()
{
    
}

///////////////////////////////////////////////////////////////////////////////
//
saga::impl::adaptor_selector::adaptor_info_list_type
job_adaptor::adaptor_register(saga::impl::session *s)
{
    // list of implemented cpi's
    saga::impl::adaptor_selector::adaptor_info_list_type list;
    preference_type prefs; 

    job_service_cpi_impl::register_cpi(list, prefs, adaptor_uuid_);
    globus_gram_job_adaptor::job_cpi_impl::register_cpi(list, prefs, adaptor_uuid_);
    
    return (list);
}
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
bool job_adaptor::register_job (std::string rm_contact, std::string job_id, 
                                saga::job::description jd)
{

    known_job_id_t id;
    id.first = std::string(rm_contact);
    id.second = std::string(job_id);
    
    std::pair<known_jobs_t::iterator, bool> p =
        known_jobs_.insert(known_jobs_t::value_type(id, jd));
    return p.second;
}
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
bool job_adaptor::unregister_job(std::string rm_contact, std::string job_id)
{
    known_job_id_t id;
    id.first = std::string(rm_contact);
    id.second = std::string(job_id);
    
    known_jobs_t::iterator it = known_jobs_.find(id);
    if (it == known_jobs_.end())
        return false;
    
    known_jobs_.erase(it);
    return true;
}
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
saga::job::description 
job_adaptor::get_job_desc (std::string rm_contact, std::string job_id) const
{
    known_job_id_t id;
    id.first = std::string(rm_contact);
    id.second = std::string(job_id);
    
    known_jobs_t::const_iterator it = known_jobs_.find(id);
    if (it == known_jobs_.end()) {
        // could be a reconnect from A DIFFERENT SESSION! 
        // in this case we can't really fill out the job description...
        // unless we write it to a file and read it. i guess that's a TODO item.
        
        saga::job::description jd;
        return jd;
    }
    
    return (*it).second;
}
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
std::vector<std::string> job_adaptor::list_jobs(std::string rm) const
{
    std::vector <std::string> jobids;
    
    known_jobs_t::const_iterator it;
    for (it = known_jobs_.begin(); it != known_jobs_.end(); ++it)
    {
        if ( ((*it).first).first == rm )
        {
            jobids.push_back( ((*it).first).second );
        }
    }

    return jobids;
}
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//bool job_adaptor::knows_job(std::string jobid) const
//{
//  return known_jobs_.find(jobid) != known_jobs_.end();
//}
//
///////////////////////////////////////////////////////////////////////////////



