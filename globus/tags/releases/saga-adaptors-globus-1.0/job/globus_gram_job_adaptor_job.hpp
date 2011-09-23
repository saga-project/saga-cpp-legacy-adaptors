//  Copyright (c) 2007 Ole Christian Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_JOB_HPP
#define ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_JOB_HPP

#include "saga_gass_server/saga_gass_server.hpp"
#include "globus_gram_job_adaptor.hpp"

#include <saga/saga/adaptors/adaptor_data.hpp>
#include <saga/impl/packages/job/job_cpi.hpp>

#include <string>

namespace common = adaptors::globus_preWS::common;

///////////////////////////////////////////////////////////////////////////////
//
namespace globus_gram_job_adaptor 
{
    class job_cpi_impl : public saga::adaptors::v1_0::job_cpi<job_cpi_impl>
    {
        
    private:
        
        typedef saga::adaptors::v1_0::job_cpi<job_cpi_impl> base_cpi;
        
        bool submitted_;
        
        /* Handle to local GASS server for I/O redirection */
        common::saga_gass_server gass_server_[2];
        
        int pipe_to_gass_stdout_[2];
        int pipe_to_gass_stderr_[2];
        
        /* NOTE: Inherited instance data members:
         *
         * init_from_jobid_(false), rm_(rm), jd_(jd) 
         * init_from_jobid_(true), rm_(rm), jobid_(jobid)
         *
         * bool init_from_jobid_;     
         * std::string rm_;        
         * std::string jobid_;        
         * saga::job::description jd_;  
         */
        
        /* adaptor data */
        typedef saga::adaptors::adaptor_data<job_adaptor> adaptor_data_type;
    
        /* utility function */
        saga::job::state get_saga_job_state (void);
        void update_state(saga::job::state newstate);
        
        // pre/post staging stuff
        saga::monitorable::cookie_handle staging_cookie_;
        void do_pre_staging (saga::job::description jd);
        bool do_post_staging (saga::object, saga::metric, saga::context);
        void register_post_staging ();
    
    public:

        ///////////////////////////////////////////////////////////////////////
        //////////////////////////// JOB C'TOR D'TOR //////////////////////////
        ///////////////////////////////////////////////////////////////////////
        
        job_cpi_impl            (proxy                * p, 
                                 cpi_info       const & info,
                                 saga::ini::ini const & glob_ini, 
                                 saga::ini::ini const & adap_ini,
                                 TR1::shared_ptr<saga::adaptor> adaptor);
        
        ~job_cpi_impl           (void);
        
        ///////////////////////////////////////////////////////////////////////
        ///////////////////////// JOB INTERFACE METHODS ///////////////////////
        ///////////////////////////////////////////////////////////////////////
        
        void sync_get_stdin     (saga::job::ostream& ostrm);
        
        void sync_get_stdout    (saga::job::istream& istrm);
        
        void sync_get_stderr    (saga::job::istream& errstrm);
        
        void sync_suspend       (saga::impl::void_t&);
        
        void sync_resume        (saga::impl::void_t&);
        
        //void sync_checkpoint    (saga::impl::void_t&);
         
        //void sync_migrate       (saga::impl::void_t&, saga::job::description jd);
        
        //void sync_signal        (saga::impl::void_t&, int signal_type);
        
        ///////////////////////////////////////////////////////////////////////
        //////////////////////// TASK INTERFACE METHODS ///////////////////////
        ///////////////////////////////////////////////////////////////////////
        
        void sync_run           (saga::impl::void_t&);
        
        void sync_wait          (bool&, double wait);
        
        void sync_cancel        (saga::impl::void_t&,  double timeout = 0.0);
        
        void sync_get_state     (saga::job::state& state);
                
        ///////////////////////////////////////////////////////////////////////
        /////////////////////// LANGUAGE BINDING SPECIFIC /////////////////////
        ///////////////////////////////////////////////////////////////////////
        
        void sync_get_job_id (std::string& jobid);
        
        
    };
    
}; 
//
///////////////////////////////////////////////////////////////////////////////

#endif // ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_JOB_HPP

