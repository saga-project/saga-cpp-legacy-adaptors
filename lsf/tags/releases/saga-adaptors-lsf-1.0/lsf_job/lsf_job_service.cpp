//  Copyright (c) 2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "lsf_job_service.hpp"
#include "lsf_job_adaptor.hpp"
#include "helper.hpp"

#include <saga/saga/adaptors/attribute.hpp>
#include <saga/saga/adaptors/utils/is_local_address.hpp>
#include <saga/saga/packages/job/adaptors/job.hpp>

#include <boost/tokenizer.hpp>

#include <iostream>
#include <string>


namespace {
    

}    

////////////////////////////////////////////////////////////////////////
namespace saga { namespace adaptors { namespace lsf
    {
        // constructor
        job_service_cpi_impl::job_service_cpi_impl (proxy                * p, 
                                                    cpi_info const       & info,
                                                    saga::ini::ini const & glob_ini, 
                                                    saga::ini::ini const & adap_ini,
                                                    TR1::shared_ptr <saga::adaptor> adaptor)
        : base_cpi (p, info, adaptor, cpi::Noflags)
        {
            instance_data data(this);
            
            if (!data->rm_.get_url().empty())
            {
                saga::url rm(data->rm_);
                std::string host(rm.get_host());
                
                std::string scheme(rm.get_scheme());
                
                if (scheme != "any" && scheme != "lsf" )
                {
                    SAGA_OSSTREAM strm;
                    strm << "Could not initialize job service for [" << data->rm_ << "]. " 
                    << "Only any:// and lsf:// schemes are supported.";
                    SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                                       saga::adaptors::AdaptorDeclined); 
                }
                
                if(host.empty() || host == "localhost")
                {
                    data->rm_.set_host(detail::get_hostname());
                }
                else if (!saga::adaptors::utils::is_local_address(rm))
                {
                    SAGA_OSSTREAM strm;
                    strm << "Job submission to remote LSF clusters [" << data->rm_ << "]"
                    << " has to be configured in LSF directly [" << data->rm_ << "]. ";
                    SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                                       saga::adaptors::AdaptorDeclined); 
                }
                
            }
            else
            {
                SAGA_OSSTREAM strm;
                strm << "Could not initialize job service for [" << data->rm_ << "]. " 
                << "Resource discovery is not available yet.";
                SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                                   saga::adaptors::AdaptorDeclined); 
            }
        }
        
        // destructor
        job_service_cpi_impl::~job_service_cpi_impl (void)
        {
        }
        
        //////////////////////////////////////////////////////////////////////
        // SAGA API functions
        void 
        job_service_cpi_impl::sync_create_job (saga::job::job         & ret, 
                                               saga::job::description   jd)
        {
            using namespace saga::job::attributes;
            
            instance_data data(this);   
            saga::attribute attr (jd);
            
            // A job description needs at least an 'Executable' 
            // attribute. Doesn't make sense without one.
            if (!attr.attribute_exists(description_executable) ||
                attr.get_attribute(description_executable).empty())
            {
                SAGA_OSSTREAM strm;
                strm << "Could not create a job instance for [" << data->rm_ << "]. " 
                << "The job description doesn't have an 'executable' attribute.";
                SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter); 
            }
            
            saga::url const instance_rm = instance_data(this)->rm_;
            saga::job::job job = saga::adaptors::job(instance_rm.get_string(), jd,
                                                     proxy_->get_session());
            
            // set the created attribute
            saga::adaptors::attribute jobattr (job);
            std::time_t current = 0;
            std::time(&current);
            jobattr.set_attribute(saga::job::attributes::created, ctime(&current));
            
            ret = job;        
        }
        
        void 
        job_service_cpi_impl::sync_run_job (saga::job::job     & ret, 
                                            std::string          cmd, 
                                            std::string          host, 
                                            saga::job::ostream & in, 
                                            saga::job::istream & out, 
                                            saga::job::istream & err)
        {
            SAGA_OSSTREAM strm;
            strm << "Interactive jobs are not supported (yet).";
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                               saga::NotImplemented); 
        }
        
        void 
        job_service_cpi_impl::sync_run_job_noio (saga::job::job & ret, 
                                                 std::string      cmd, 
                                                 std::string      host)
        {
            instance_data data(this);
            
            std::string error_text("The adaptor couldn't run the job for the following reason: ");
            
            if (cmd.empty())
            {
                error_text.append("'Executable' parameter is empty!");
                SAGA_ADAPTOR_THROW(error_text, saga::BadParameter);
            }
            
            std::vector<std::string> cmdline = detail::split_cmdline (cmd);
            std::string executable = cmdline[0];
            
            saga::job::description jobdef_self;
            
            jobdef_self.set_attribute (saga::job::attributes::description_interactive, "True");
            jobdef_self.set_attribute(saga::job::attributes::description_candidate_hosts, host);
            jobdef_self.set_attribute(saga::job::attributes::description_executable, executable);
            
            if (cmdline.size() > 1)
            {
                cmdline.erase(cmdline.begin());
                jobdef_self.set_vector_attribute(saga::job::attributes::description_arguments, 
                                                 cmdline);
            }
            
            saga::job::job job = saga::adaptors::job(data->rm_.get_url(), 
                                                     jobdef_self, proxy_->get_session());
            
            std::time_t current = 0;
            std::time(&current);
            
            saga::adaptors::attribute jobattr (job);
            jobattr.set_attribute(saga::job::attributes::created, ctime(&current));
            
            // Finally, fire up the job.
            job.run();
            
            ret = job;
        }
        
        void 
        job_service_cpi_impl::sync_list (std::vector <std::string> & ret)
        {
            instance_data data(this);
            saga::url rm(data->rm_);
            
            try {
                std::string full_cmd_l; // only used for debug output
                
                std::string lsf_cmd("bjobs");  full_cmd_l.append("bjobs");
                std::vector<std::string> args; 
                args.push_back("-u");          full_cmd_l.append(" -u");
                args.push_back("all");         full_cmd_l.append(" all");
                
                SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_DEBUG) {
                    std::cerr << DBG_PRFX << "Trying to execute: " << full_cmd_l << std::endl;
                }
                
                boost::process::child c = adaptor_data(this) -> 
                    run_lsf_command(lsf_cmd, args, boost::process::close_stream);
                
                boost::process::pistream & out = c.get_stdout();
                
                std::string line;
                
                while (getline(out, line))
                {
                    SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_DEBUG) {
                        std::cerr << DBG_PRFX << "Output: " << line << std::endl;
                    }
                    // skip the header
                    if(line.find("JOBID") != std::string::npos) {
                        continue;
                    }
                    
                    size_t pos = line.find(" ");
                    std::string pid =  line.substr(0, pos);
                    if(pid.length() > 1) 
                        ret.push_back("[" + rm.get_url() + "]-[" + pid + "]");
                }
                
                boost::process::status status = c.wait();
            }
            catch (std::exception const & e)
            {
                SAGA_OSSTREAM strm;
                strm << "Problem retrieving list of LSF jobs for [" << rm << "]: " 
                << e.what() << ".";
                SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter); 
            }
        }
        
        void
        job_service_cpi_impl::sync_get_job (saga::job::job & ret, 
                                            std::string      jobid)
        {
            // we only want to accept any:// or lsf:// schemes here as well! 
            if(!detail::is_lsf_or_any_scheme(jobid))
            {
                SAGA_OSSTREAM strm;
                strm << "This doesn't look like an LSF job ID: " << jobid << ".";
                SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
            }
            
            std::string native_jobid(detail::saga_to_native_jobid(jobid));            
            
            instance_data data(this);
            ret = saga::adaptors::job (data->rm_.get_url(),jobid); //native_jobid);
        }

        
        void job_service_cpi_impl::sync_get_self (saga::job::self & ret)
        {
            SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
        }
        
    }
}
} 

