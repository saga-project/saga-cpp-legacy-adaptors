//  Copyright (c) 2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "lsf_job_adaptor.hpp"
#include "lsf_job_service.hpp"
#include "lsf_job.hpp"

#include <boost/version.hpp>
#if BOOST_VERSION >= 103400
#include <boost/filesystem.hpp>
#else
#include <boost/filesystem/operations.hpp>  
#include <boost/filesystem/convenience.hpp>
#endif

SAGA_ADAPTOR_REGISTER (saga::adaptors::lsf::job_adaptor);


////////////////////////////////////////////////////////////////////////
namespace saga { namespace adaptors { namespace lsf
    {
        // register function for the SAGA engine
        saga::impl::adaptor_selector::adaptor_info_list_type
        job_adaptor::adaptor_register (saga::impl::session * s)
        {
            // list of implemented cpi's
            saga::impl::adaptor_selector::adaptor_info_list_type list;
            
            // create empty preference list
            // these list should be filled with properties of the adaptor, 
            // which can be used to select adaptors with specific preferences.
            // Example:
            //   'security' -> 'gsi'
            //   'logging'  -> 'yes'
            //   'auditing' -> 'no'
            preference_type prefs; 
            
            // create file adaptor infos (each adaptor instance gets its own uuid)
            // and add cpi_infos to list
            job_service_cpi_impl::register_cpi (list, prefs, adaptor_uuid_);
            job_cpi_impl::register_cpi         (list, prefs, adaptor_uuid_);
            
            // and return list
            return (list);
        }
        
        bool job_adaptor::init(saga::impl::session *,
                               saga::ini::ini const& glob_ini, saga::ini::ini const& adap_ini)
        {            
            namespace fs = boost::filesystem;
            
            // LSF binaries path
            if (adap_ini.has_section("preferences"))
            {
                saga::ini::ini prefs = adap_ini.get_section("preferences");
                binary_path_ = prefs.get_entry("binary_path", "");
                
                if ( !fs::exists( binary_path_ ) )
                {
                    SAGA_OSSTREAM strm;
                    strm << "The binary path provided in platform_lsf_job_adaptor.ini [" << binary_path_ 
                    << "] does not exist. Can't use the LSF adaptor."; 
                    SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter); 
                }
            }
    
            return true;
        }
            
            
            boost::process::child 
            job_adaptor::run_lsf_command(std::string const & command,
                                         std::vector<std::string> const & arguments,
                                         boost::process::stream_behavior stdin_behavior,
                                         boost::process::stream_behavior stdout_behavior,
                                         boost::process::stream_behavior stderr_behavior)
            {
                boost::process::launcher cmd_launcher;
                // This does something WEIRD. I can't even explain what. It simply doesn't work
                // as expected. Fortunately, command_line::shell works. It's not really portable,
                // but I don't really care at this point. 
                //boost::process::command_line cl(command, command, get_binary_path());
               
                std::string cmd_line = get_binary_path() + "/" + command + " "; 
                std::vector<std::string>::const_iterator it;
                for(it = arguments.begin(); it != arguments.end(); ++it) {
                  cmd_line.append(*it);
                  cmd_line.append(" ");
                }

                boost::process::command_line cl = 
                  boost::process::command_line::shell(cmd_line);
                
                //std::vector<std::string>::const_iterator end = arguments.end();
                //for (std::vector<std::string>::const_iterator it = arguments.begin(); 
                //     it != end; ++it)
                //{
                //    cl.argument(*it);
                //}
                
                cmd_launcher.set_stdin_behavior(stdin_behavior);
                cmd_launcher.set_stdout_behavior(stdout_behavior);
                cmd_launcher.set_stderr_behavior(stderr_behavior);
                
                if (boost::process::close_stream != stdout_behavior
                    && boost::process::close_stream == stderr_behavior)
                    cmd_launcher.set_merge_out_err(true);
                
                return cmd_launcher.start(cl);
            }
        
        
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
        
            
        }}} // namespace lsf_job
    ////////////////////////////////////////////////////////////////////////
    
