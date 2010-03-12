//  Copyright (c) 2005-2009 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_OMII_GRIDSAM_ADAPTOR_HPP
#define ADAPTORS_OMII_GRIDSAM_ADAPTOR_HPP

#include <map>
#include <saga/saga/adaptors/adaptor.hpp>

///////////////////////////////////////////////////////////////////////////////
//  forward decl only
class omii_gridsam_job;

///////////////////////////////////////////////////////////////////////////////
struct omii_gridsam_adaptor : public saga::adaptor
{
    typedef saga::impl::v1_0::op_info         op_info;  
    typedef saga::impl::v1_0::cpi_info        cpi_info;
    typedef saga::impl::v1_0::preference_type preference_type;

    /**
    * This functions registers the adaptor with the factory
    *
    * @param factory the factory where the adaptor registers
    *        its maker function and description table
    */
    saga::impl::adaptor_selector::adaptor_info_list_type 
            adaptor_register (saga::impl::session *s);
            
    std::string get_name (void) const
    { 
        return BOOST_PP_STRINGIZE(SAGA_ADAPTOR_NAME);
    }

    bool init(saga::impl::session *, saga::ini::ini const& glob_ini, 
        saga::ini::ini const& adap_ini);

    // list of job ids of jobs created by this adaptor
    typedef std::map<std::string, saga::job::description> known_jobs_type;
    known_jobs_type known_jobs_;
    
    bool register_job(std::string jobid, saga::job::description jd);
    bool unregister_job(std::string jobid);
    bool knows_job(std::string jobid) const;
    saga::job::description get_job(omii_gridsam_job const* job, 
        std::string jobid) const;
    std::vector<std::string> list_jobs() const;
};

#endif 

