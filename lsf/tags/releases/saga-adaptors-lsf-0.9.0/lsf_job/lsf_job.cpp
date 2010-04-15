//  Copyright (c) 2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga adaptor icnludes
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/attribute.hpp>
#include <saga/saga/adaptors/file_transfer_spec.hpp>

// saga engine includes
#include <saga/impl/config.hpp>
#include <saga/impl/exception_list.hpp>

// saga package includes
#include <saga/saga/packages/job/adaptors/job_self.hpp>
#include <saga/saga/packages/job/job_description.hpp>

// adaptor includes
#include "helper.hpp"
#include "lsf_job.hpp"
#include "lsf_job_istream.hpp"
#include "lsf_job_ostream.hpp"


////////////////////////////////////////////////////////////////////////
namespace saga { namespace adaptors { namespace lsf {
    
    // constructor
    job_cpi_impl::job_cpi_impl (proxy                           * p, 
                                cpi_info const                  & info,
                                saga::ini::ini const            & glob_ini, 
                                saga::ini::ini const            & adap_ini,
                                TR1::shared_ptr <saga::adaptor>   adaptor)
    : base_cpi  (p, info, adaptor, cpi::Noflags)
    {
        namespace attr = saga::job::attributes;
        instance_data data(this);
		
		// first of all, check if we can handle this request
		if (!data->rm_.get_url().empty())
		{
			saga::url rm(data->rm_);
			std::string host(rm.get_host());
			
			std::string scheme(rm.get_scheme());
			if (scheme != "lsf" && scheme != "any")
			{
				SAGA_OSSTREAM strm;
				strm << "Could not initialize job for [" << data->rm_ << "]. " 
				<< "Only any:// and lsf:// schemes are supported.";
				SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
								   saga::adaptors::AdaptorDeclined); 
			}
			
			if (host.empty())
			{
				SAGA_OSSTREAM strm;
				strm << "Could not initialize job for [" << data->rm_ << "]. " 
				<< "URL doesn't define a hostname.";
				SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
								   saga::BadParameter); 
			}
		}
		
	
        if (data->init_from_jobid_) {
            //saga::attribute attr (this->proxy_);
            //attr.set_attribute(saga::job::attributes::jobid, data->jobid_);
            if(!detail::is_lsf_or_any_scheme(data->jobid_))
	    {
	        SAGA_OSSTREAM strm;
		strm << "This doesn't look like an LSF job id: " << data->jobid_ << ".";
		SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
	    }
	    else
	    {
                set_job_attributes(data->jobid_);
	    }
        }
        else {
            saga::job::description jd = data->jd_;
            if(jd.attribute_exists(saga::job::attributes::description_interactive))
                if(saga::job::attributes::description_interactive == saga::attributes::common_true)
                {
                    SAGA_OSSTREAM strm;
                    strm << "Interactive jobs are not supported (yet).";
                    SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                                       saga::NotImplemented); 
                }
            
            // From now on the job is in 'New' state - ready to run!
            update_state(saga::job::New);
            
        }
        
        
    }
    
    
    // destructor
    job_cpi_impl::~job_cpi_impl (void)
    {
    }
    
    void job_cpi_impl::update_state(saga::job::state newstate)
    {
        saga::monitorable monitor (this->proxy_);
        saga::adaptors::metric m (monitor.get_metric(saga::metrics::task_state));
        m.set_attribute(saga::attributes::metric_value, 
                        saga::adaptors::job_state_enum_to_value(newstate));
    }  
    
    //  SAGA API functions
    void job_cpi_impl::sync_get_state (saga::job::state & ret)
    {
        instance_data data(this);

        saga::monitorable monitor (this->proxy_);
        saga::metric m (monitor.get_metric(saga::metrics::task_state));
        saga::job::state state = saga::adaptors::job_state_value_to_enum(
                m.get_attribute(saga::attributes::metric_value));

        if(state == saga::job::New)
            // job has not been submitted yet - no need to query the RM
            ret = state;
        else {
            // query the RM for the job's state. 
            ret = get_rm_job_state(data->jobid_);
        }
    }

    void job_cpi_impl::sync_get_description (saga::job::description & ret)
    {
        // return a deep copy of the job description
        instance_data data(this);
        ret = data->jd_.clone();     
    }

    void job_cpi_impl::sync_get_job_id (std::string & ret)
    { 
        saga::attribute attr (this->proxy_);
        ret = attr.get_attribute(saga::job::attributes::jobid);
    }

    // access streams for communication with the child
    void job_cpi_impl::sync_get_stdin (saga::job::ostream & ret)
    {
        SAGA_OSSTREAM strm;
        strm << "Interactive jobs are not supported (yet).";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                           saga::NotImplemented); 
    }
    
    void job_cpi_impl::sync_get_stdout (saga::job::istream & ret)
    {
        SAGA_OSSTREAM strm;
        strm << "Interactive jobs are not supported (yet).";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                           saga::NotImplemented); 
    }
    
    void job_cpi_impl::sync_get_stderr (saga::job::istream & ret)
    {
        SAGA_OSSTREAM strm;
        strm << "Interactive jobs are not supported (yet).";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                           saga::NotImplemented); 
    }
    
    void job_cpi_impl::sync_checkpoint (saga::impl::void_t & ret)
    {
        SAGA_OSSTREAM strm;
        strm << "Checkpointing is not supported (yet).";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                           saga::NotImplemented); 
    }    
    
    void job_cpi_impl::sync_migrate (saga::impl::void_t           & ret, 
                                     saga::job::description   jd)
    {
        SAGA_OSSTREAM strm;
        strm << "Migration is not supported (yet).";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                           saga::NotImplemented); 
    }    
    
    
    void job_cpi_impl::sync_signal (saga::impl::void_t & ret, 
                                    int            signal)
    {
        SAGA_OSSTREAM strm;
        strm << "Signaling is not supported (yet).";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                           saga::NotImplemented);     
    }
    
    
    //  suspend the child process 
    void job_cpi_impl::sync_suspend (saga::impl::void_t & ret)
    {    
	std::string job_id;
	sync_get_job_id(job_id);
        std::string native_jobid(detail::saga_to_native_jobid(job_id));
 
        saga::job::state state;
        sync_get_state(state);  
        
        if (saga::job::Running != state) {
            SAGA_ADAPTOR_THROW("Can't suspend a non-running job!",
                               saga::IncorrectState);
        }
        else {
            std::vector<std::string> args;
            args.push_back(native_jobid);
            std::string output;
            
            boost::process::child c = adaptor_data(this)->
            run_lsf_command("bstop", args, boost::process::close_stream);
            
            boost::process::pistream & out = c.get_stdout();
            std::string line;
            getline(out, line);
            
            // handle possible errors
            if(line.find("is being stopped") == std::string::npos)
            {
                // spec: if the resource manager cannot parse the job_id 
                // at all, a ’BadParameter’ exception is thrown.
                SAGA_OSSTREAM strm;
                strm << "Problem suspending LSF job [" << job_id << "]: "
                << line << "."; 
                SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
            }
            
            boost::process::status status = c.wait();
        }
    }
    
    //  suspend the child process 
    void job_cpi_impl::sync_resume (saga::impl::void_t & ret)
    {
        std::string job_id;
        sync_get_job_id(job_id);
        std::string native_jobid(detail::saga_to_native_jobid(job_id));
 
        saga::job::state state;
        sync_get_state(state);  
        
        if (saga::job::Suspended != state) {
            SAGA_ADAPTOR_THROW("Can't resume a non-suspended job!",
                               saga::IncorrectState);
        }
        else {
            std::vector<std::string> args;
            args.push_back(native_jobid);
            std::string output;
            
            boost::process::child c = adaptor_data(this)->
            run_lsf_command("bresume", args, boost::process::close_stream);
            
            boost::process::pistream & out = c.get_stdout();
            std::string line;
            getline(out, line);
            
            // handle possible errors
            if(line.find("is being resumed") == std::string::npos)
            {
                // spec: if the resource manager cannot parse the job_id 
                // at all, a ’BadParameter’ exception is thrown.
                SAGA_OSSTREAM strm;
                strm << "Problem resuming LSF job [" << job_id << "]: "
                << line << "."; 
                SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
            }
            
            boost::process::status status = c.wait();
        }    
    }
    
    
    //////////////////////////////////////////////////////////////////////
    // inherited from the task interface
    void job_cpi_impl::sync_run (saga::impl::void_t & ret)
    {
        instance_data inst_data (this);
        saga::job::description jd = inst_data->jd_;  // Job description
        std::string rm = inst_data->rm_.get_url();  // RM 'host' string
        
        saga::job::state state;
        sync_get_state(state);  
        
        if (saga::job::New != state)
            SAGA_ADAPTOR_THROW("Job has been started already!",
                               saga::IncorrectState);
        
        // we're in 'New' state. Let's try to submit!
        std::vector<std::string> args(jd_to_arg_list(jd));
        
        try
        {
            std::string output;
            
            boost::process::child c = adaptor_data(this)->
            run_lsf_command("bsub", args, boost::process::close_stream);
            
            boost::process::pistream & out = c.get_stdout();
            boost::process::status status = c.wait();
            //if (!status.exited() || status.exit_status())
            //    SAGA_ADAPTOR_THROW("Failed to submit job to LSF. "
            //                       "Output from lsf_submit follows:\n" + output,
            //                       saga::NoSuccess);
            
            
            std::string line;
            getline(out, line);
            
            if(line.find("Job <") == std::string::npos) {
                SAGA_ADAPTOR_THROW("Failed to submit job to LSF: " + line,
                                   saga::NoSuccess);
            }
            
            else 
            {
                typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                boost::char_separator<char> sep(" ");
                tokenizer tok(line, sep);
                
                int attr_count=0;
                
                for(tokenizer::iterator beg=tok.begin(); beg!=tok.end();++beg)
                {
                    if( attr_count == 1 ) {
                        std::string pid = (*beg);
                        pid.replace(0,1,"[");
                        pid.replace(pid.length()-1,1,"]");
                        
                        std::string job_id("[" + rm + "]-" + pid);
                        inst_data->jobid_ = job_id;
                        saga::adaptors::attribute attr (this);
                        attr.set_attribute(saga::job::attributes::jobid, job_id);     
                        
                        saga::monitorable monitor (this->proxy_);
                        saga::adaptors::metric m (monitor.get_metric(saga::metrics::task_state));
                        m.set_attribute(saga::attributes::metric_value,
                                        saga::adaptors::job_state_enum_to_value(saga::job::Running));
                    } 
                    ++attr_count;
                }
                // Ugly Hack (FIXME): Even though bsub returns a jobid, a sub-
                // sequent call of bjobs might not show the job. sometimes LSF
                // needs some time to update its database.
                // For now, we just SLEEP for 3 secs. until we find a better solution.
                sleep(3);
            }
        }
        catch (saga::adaptors::exception const &)
        {
            // Let our exceptions fall through.
            throw;
        }
        catch (std::exception const & e)
        {
            SAGA_ADAPTOR_THROW("Problem launching LSF job: "
                               "(std::exception caught: " + e.what() + ")",
                               saga::BadParameter);
        }
        
        
    }
    
    void job_cpi_impl::sync_cancel (saga::impl::void_t & ret, 
                                    double timeout)
    {
        std::string job_id;
        sync_get_job_id(job_id);
        std::string native_jobid(detail::saga_to_native_jobid(job_id));
        
        saga::job::state state;
        sync_get_state(state);  
        
        if (saga::job::New == state) {
            SAGA_ADAPTOR_THROW("Can't cancel a non-active job!",
                               saga::IncorrectState);
        }
        else {
            std::vector<std::string> args;
            args.push_back(native_jobid);
            std::string output;
            
            boost::process::child c = adaptor_data(this)->
            run_lsf_command("bkill", args, boost::process::close_stream);
            
            boost::process::pistream & out = c.get_stdout();
            std::string line;
            getline(out, line);
            
            // handle possible errors
            if(line.find("is being terminated") == std::string::npos)
            {
                // spec: if the resource manager cannot parse the job_id 
                // at all, a ’BadParameter’ exception is thrown.
                SAGA_OSSTREAM strm;
                strm << "Problem terminating LSF job [" << job_id << "]: "
                << line << "."; 
                SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
            }
            
            boost::process::status status = c.wait();
        }    
    }
    
    //  wait for the child process to terminate
    void job_cpi_impl::sync_wait (bool   & ret, 
                                  double   wait)
    {
        saga::adaptors::attribute attr (this);
        std::string job_id = attr.get_attribute (saga::job::attributes::jobid);
        
        double wait_count = 0.0;
        saga::job::state s; 
        ret = false;
        
        try {
            this->sync_get_state(s);
            if(s == saga::job::New) {
                SAGA_ADAPTOR_THROW("job is in saga::job::New state.",
                                   saga::IncorrectState);
            }
            
            if(wait < 0.0) {
                this->sync_get_state(s);
                while(s == saga::job::Running) {
                    this->sync_get_state(s);
                    sleep(1);
                }
                ret = true;
            }
            
            else if(wait > 0.0) {
                while(wait_count <= wait) {
                    this->sync_get_state(s);
                    if(s != saga::job::Running) {
                        ret = true;
                        break;
                    }
                    wait_count += 1.0; sleep(1); 
                }
            }
            else {
                this->sync_get_state(s);
                if(s != saga::job::Running) {
                    ret = true;
                }
            }
        }
        catch(saga::exception const & e)
        {
            //catch exceptions from other methods
            SAGA_OSSTREAM strm;
            strm << "Could not wait for job [" << job_id << "]. " 
            << e.get_message();
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.get_error()); 
        }
        
    }
    
    //////////////////////////////////////////////////////////////////////
    //
    std::vector<std::string> job_cpi_impl::jd_to_arg_list(const saga::job::description &jd)
    {
        namespace sja = saga::job::attributes;
        saga::attribute attr (jd);
        
        std::vector<std::string> ret;
        
        typedef std::map<const std::string,  std::string> translation_table_t;
        translation_table_t tt; 
        
        tt[sja::description_queue] =                   "-q";
        tt[sja::description_job_project] =             "-P";
        tt[sja::description_job_contact] =             "-u";
        
        tt[sja::description_input] =                   "-i";
        tt[sja::description_output] =                  "-o";
        tt[sja::description_error] =                   "-e";
        tt[sja::description_working_directory] =       "-cwd";

        translation_table_t::const_iterator it;
        
        // Process all mappings from the translation table above
        for( it = tt.begin(); it != tt.end(); ++it )
        {
            if( attr.attribute_exists((*it).first) )
            {
                // only add if attribute is non-empty
                std::string val = attr.get_attribute((*it).first);
                if (!val.empty()) {                
                    ret.push_back((*it).second+" "+val);
                }
                
            }
        }
        
        if( attr.attribute_exists(sja::description_executable) )
        {
            // LSF doesn't understand the concept of './'. Either the file
            // is within CWD or you supply the full path. Period. Any './'
            // before the executable name will lead to a failiure.
            std::string exe = attr.get_attribute(sja::description_executable);
            if(exe.find("./") == 0) {
              exe = exe.substr(2, exe.length()-1);
            } 

            ret.push_back(exe);
        }
        
        if (jd.attribute_exists(sja::description_arguments))
        {
            std::vector<std::string> arguments = 
            jd.get_vector_attribute(sja::description_arguments);
            
            if( arguments.size() > 0 ) 
            {                
                std::vector<std::string>::iterator end = arguments.end();
                for (std::vector<std::string>::iterator it = arguments.begin(); 
                     it != end; ++it)
                {
                    ret.push_back((*it));
                }
            }
        }
        
        return ret;
    }
    
    //////////////////////////////////////////////////////////////////////
    //
    saga::job::state job_cpi_impl::get_rm_job_state(std::string jobid)
    {
        instance_data data(this);
        saga::job::state job_state = saga::job::Unknown;
        
        try {
            
            std::string native_jobid(detail::saga_to_native_jobid(jobid));
            
            std::vector<std::string> args;
            args.push_back(native_jobid);    
            
            boost::process::child c = adaptor_data(this)->run_lsf_command("bjobs", 
                args, boost::process::close_stream);
            
            boost::process::pistream & out = c.get_stdout();
            
            std::string line;
            getline(out, line);
            
            // handle possible errors
            if(line.find("Illegal job ID") != std::string::npos)
            {
                // spec: if the resource manager cannot parse the job_id 
                // at all, a ’BadParameter’ exception is thrown.
                SAGA_OSSTREAM strm;
                strm << "Problem retrieving job information for LSF job id " << jobid << ": "
                << line << "."; 
                SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
            }
            else if(line.find("is not found") != std::string::npos)
            {
                // spec: if the resource manager can handle the job_id, 
                // but the referenced job is not alive, a ’DoesNotExist’ exception is thrown. 
                SAGA_OSSTREAM strm;
                strm << "Problem retrieving job information for LSF job id " << jobid << ": "
                << line << ".";
                SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::DoesNotExist);
            }
            
            while (getline(out, line))
            {
                if(line.find("JOBID") != std::string::npos)
                    continue; // skip header
                else 
                {
                    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                    boost::char_separator<char> sep(" ");
                    tokenizer tok(line, sep);
                    
                    int attr_count=0;
                    
                    for(tokenizer::iterator beg=tok.begin(); beg!=tok.end();++beg)
                    {
                        if(attr_count == 2) {
                            
                            if( ((*beg) == "RUN")   || 
                               ((*beg) == "WAIT")  ||
                               ((*beg) == "PEND"))
                                job_state = saga::job::Running;
                            
                            else if((*beg) == "DONE")
                                job_state = saga::job::Done;
                            
                            else if( ((*beg) == "UNKNOWN") || 
                                    ((*beg) == "ZOMBI")   ||
                                    ((*beg) == "EXIT"))
                                job_state = saga::job::Failed;
                            
                            else if( ((*beg) == "USUSP") || 
                                    ((*beg) == "SSUSP")   ||
                                    ((*beg) == "PSUSP"))
                                job_state = saga::job::Suspended;
                        }
                        ++attr_count;
                    }
                }
            }
            boost::process::status status = c.wait();
        }
        catch (std::exception const & e)
        {
            SAGA_OSSTREAM strm;
            strm << "Problem retrieving job information for LSF job id " << jobid << ": " 
            << e.what() << ".";
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter); 
        }
        
        return job_state;
    }
    
    //////////////////////////////////////////////////////////////////////
    //
    void job_cpi_impl::set_job_attributes(std::string jobid)
    {
        
        // try to execute bjobs -l <jobid> to get detailed informations 
        // about the job.
        try {
            
            std::string native_jobid(detail::saga_to_native_jobid(jobid));
            
            std::vector<std::string> args;
            args.push_back("-W");
            args.push_back(native_jobid);
            
            instance_data data(this);
            
            boost::process::child c = adaptor_data(this)->run_lsf_command("bjobs", 
                args, boost::process::close_stream);
            
            boost::process::pistream & out = c.get_stdout();
            
            std::string line;
            getline(out, line);
            
            // handle possible errors
            if(line.find("Illegal job ID") != std::string::npos)
            {
                // spec: if the resource manager cannot parse the job_id 
                // at all, a ’BadParameter’ exception is thrown.
                SAGA_OSSTREAM strm;
                strm << "Problem retrieving job information for LSF job id " << jobid << ": "
                << line << "."; 
                SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
            }
            else if(line.find("is not found") != std::string::npos)
            {
                // spec: if the resource manager can handle the job_id, 
                // but the referenced job is not alive, a ’DoesNotExist’ exception is thrown. 
                SAGA_OSSTREAM strm;
                strm << "Problem retrieving job information for LSF job id " << jobid << ": "
                << line << ".";
                SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::DoesNotExist);
            }
            
            
            saga::job::description jd = data->jd_;
            saga::job::state job_state = saga::job::Unknown;
            std::string state_detail, cpu_time, memory_use; // metric values
            
            saga::adaptors::attribute attr (this);
            attr.set_attribute(saga::job::attributes::jobid, std::string(jobid)); 
            
            while (getline(out, line))
            {
                if(line.find("JOBID") != std::string::npos)
                    continue; // skip header
                else 
                {
                    namespace attr = saga::job::attributes;
                    int attr_count=0;
                    
                    
                    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
                    boost::char_separator<char> sep(" ");
                    tokenizer tok(line, sep);
                    for(tokenizer::iterator beg=tok.begin(); beg!=tok.end();++beg)
                    {
                        switch(attr_count) {
                              
                          case 2 : {
                              // STATE -> mapped to job_detail metric
                              state_detail = (*beg);
                              
                              if( (state_detail == "RUN")   || 
                                 (state_detail == "WAIT")  ||
                                 (state_detail == "PEND"))
                                  job_state = saga::job::Running;
                              
                              else if(state_detail == "DONE")
                                  job_state = saga::job::Done;
                              
                              else if( (state_detail == "UNKNOWN") || 
                                      (state_detail == "ZOMBI")   ||
                                      (state_detail == "EXIT"))
                                  job_state = saga::job::Failed;
                              
                              else if( (state_detail == "USUSP") || 
                                      (state_detail == "SSUSP")   ||
                                      (state_detail == "PSUSP"))
                                  job_state = saga::job::Suspended;
                              
                              break;
                          }
                          case 3 :
                              // QUEUE
                              jd.set_attribute(attr::description_queue,*beg);
                              break;
                          case 5 : {
                              // EXEC HOSTS -> candidate_hosts
                              std::vector<std::string> candidate_hosts;
                              boost::char_separator<char> sepa(":");
                              tokenizer toka(*beg, sepa);
                              for(tokenizer::iterator bega=toka.begin(); bega!=toka.end();++bega)
                                  candidate_hosts.push_back(*bega);
                              
                              jd.set_vector_attribute(attr::description_candidate_hosts,candidate_hosts);
                              break;
                          }
                          case 6 :
                              // JOB_NAME -> can be mapped to executalbe - TODO: split of arguments
                              jd.set_attribute(attr::description_executable,*beg);
                              break;
                              // SUBMIT_TIME - created
                              jd.set_attribute(attr::created,*beg);
                              break;
                          case 8 : {
                              // PROJ_NAME
                              std::vector<std::string> prj;
                              prj.push_back(*beg);
                              jd.set_vector_attribute(attr::description_job_project,prj);
                              break; 
                          }
                          case 9 : {
                              // CPU_USED -> mapped to cpu_time metric
                              cpu_time = (*beg);
                              break;
                          }
                          case 10 : {
                              // MEM -> mapped to memory_use metric
                              memory_use = (*beg);
                              break;
                          }
                          case 13 :
                              // START_TIME - can be "-" if not started 
                              jd.set_attribute(attr::description_job_start_time,*beg);
                              break;
                          default :
                              // do nothing
                              break;
                        }
                        
                        ++attr_count;
                    }
                }
            }
            boost::process::status status = c.wait();
            
            // set the metrics for the newly created job object
            
            // The job is already registered with the resource 
            // manager. That means that it has to be in running
            // state (even if it is still pending).
            saga::monitorable monitor (this->proxy_);
            saga::adaptors::metric m (monitor.get_metric(saga::metrics::task_state));
            m.set_attribute(saga::attributes::metric_value, 
                            saga::adaptors::job_state_enum_to_value(job_state));
            
            m = (monitor.get_metric(saga::job::metrics::state_detail));
            m.set_attribute(saga::attributes::metric_value, state_detail);
            
            m = (monitor.get_metric(saga::job::metrics::cpu_time));
            m.set_attribute(saga::attributes::metric_value, cpu_time);
            
            m = (monitor.get_metric(saga::job::metrics::memory_use));
            m.set_attribute(saga::attributes::metric_value, memory_use);
            
        }
        catch (std::exception const & e)
        {
            SAGA_OSSTREAM strm;
            strm << "Problem retrieving job information for LSF job id " << jobid << ": " 
            << e.what() << ".";
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter); 
        }
        
    }
    
    
    
}}} // namespace lsf_job
////////////////////////////////////////////////////////////////////////

