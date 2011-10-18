//  Copyright (c) 2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_LSF_JOB_ADAPTOR_HPP
#define ADAPTORS_LSF_JOB_ADAPTOR_HPP

#include "helper.hpp"

#include <saga/saga/adaptors/adaptor.hpp>

#include <boost/assert.hpp>
#include <boost/process.hpp>

#include <map>
#include <memory>

////////////////////////////////////////////////////////////////////////
namespace saga { namespace adaptors { namespace lsf
    {
        class job_adaptor : public saga::adaptor
        {
            typedef saga::impl::v1_0::op_info         op_info;  
            typedef saga::impl::v1_0::cpi_info        cpi_info;
            typedef saga::impl::v1_0::preference_type preference_type;
            
            // This function registers the adaptor with the factory
            // @param factory the factory where the adaptor registers
            //        its maker function and description table
            saga::impl::adaptor_selector::adaptor_info_list_type 
            adaptor_register (saga::impl::session * s);
            
            bool init(saga::impl::session *, saga::ini::ini const& glob_ini,
                      saga::ini::ini const& adap_ini);
            
            std::string get_name (void) const
            { 
                return BOOST_PP_STRINGIZE (SAGA_ADAPTOR_NAME);
            }
            
        public:
            
            std::string get_binary_path() const
            {
                return binary_path_;
            }
            
            
            boost::process::child 
            run_lsf_command(std::string const & command,
                            std::vector<std::string> const & arguments = std::vector<std::string>(),
                            boost::process::stream_behavior stdin_behavior = boost::process::redirect_stream,
                            boost::process::stream_behavior stdout_behavior = boost::process::redirect_stream,
                            boost::process::stream_behavior stderr_behavior = boost::process::close_stream);
            
        private:

            std::string binary_path_;
            boost::process::launcher cmd_launcher_;
            
            typedef std::pair<std::string, std::string> known_job_id_t;
            
            /** List type of jobs created by this adaptor. 
             * The mapping is rm_contact -> description
             */
            typedef std::map<known_job_id_t, saga::job::description> known_jobs_t;
            known_jobs_t known_jobs_;            
            
            std::vector<std::string> list_jobs (std::string rm) const;
            
            bool register_job (std::string rm_contact, 
                               std::string job_id, 
                               saga::job::description jd);
            
                
            bool unregister_job (std::string rm_contact, 
                                 std::string job_id);
        };
        
    }}} // namespace lsf_job
////////////////////////////////////////////////////////////////////////

#endif // ADAPTORS_LSF_JOB_ADAPTOR_HPP

