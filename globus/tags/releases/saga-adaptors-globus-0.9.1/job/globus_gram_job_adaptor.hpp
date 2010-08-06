//  Copyright (c) 2007 Ole Christian Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_HPP
#define ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_HPP

#include <saga/saga/adaptors/adaptor.hpp>
#include <boost/spirit/core/non_terminal/impl/static.hpp>

///////////////////////////////////////////////////////////////////////////////
//  forward decl only
class job_cpi_impl;

/////////////////////////////////////////////////////////////////////////////
//
struct job_adaptor : public saga::adaptor
{
    typedef saga::impl::v1_0::op_info         op_info;  
    typedef saga::impl::v1_0::cpi_info        cpi_info;
    typedef saga::impl::v1_0::preference_type preference_type;

    
    std::string get_name (void) const
    {
        return BOOST_PP_STRINGIZE(SAGA_ADAPTOR_NAME);
    }

    job_adaptor();
    
    
    ~job_adaptor();

    /**
      * This functions registers the adaptor with the factory
      *
      * @param factory the factory where the adaptor registers
      *        its maker function and description table
      */
    saga::impl::adaptor_selector::adaptor_info_list_type 
        adaptor_register (saga::impl::session *s);

    typedef std::pair<std::string, std::string> known_job_id_t;

    /** List type of jobs created by this adaptor. 
      * The mapping is rm_contact -> description
      */
    typedef std::map<known_job_id_t, saga::job::description> known_jobs_t;
        known_jobs_t known_jobs_;

    /** DOC
      */
    bool register_job (std::string rm_contact, 
                       std::string job_id, 
                       saga::job::description jd);

    /** DOC
      */                  
    bool unregister_job (std::string rm_contact, 
                         std::string job_id);

    /** DOC
      */
    saga::job::description get_job_desc (std::string rm_contact, 
                                        std::string job_id) const;

    /** DOC
      */                                
    std::vector<std::string> list_jobs (std::string rm) const;
};

#endif // ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_HPP
