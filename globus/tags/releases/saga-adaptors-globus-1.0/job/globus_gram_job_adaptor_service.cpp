//  Copyright (c) 2007-2010 Ole Christian Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "globus_gram_job_adaptor_connector.hpp"
#include "globus_gram_job_adaptor_service.hpp"
#include "globus_gram_job_adaptor_errorhandler.hpp"
#include "globus_gram_job_adaptor_istream.hpp"

#include "../shared/globus_gsi_cert_utils.hpp"
#include "../loader/globus_global_loader.hpp"

#include <saga/impl/config.hpp>
#include <saga/saga/exception.hpp>
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/utils.hpp>
#include <saga/saga/adaptors/attribute.hpp>
#include <saga/saga/packages/job/adaptors/job.hpp>

///////////////////////////////////////////////////////////////////////////////
//
globus_gram_job_adaptor::job_service_cpi_impl::
job_service_cpi_impl (proxy                * p, 
                      cpi_info       const & info,
                      saga::ini::ini const & glob_ini, 
                      saga::ini::ini const & adap_ini,
                      TR1::shared_ptr<saga::adaptor> adaptor)
: base_cpi (p, info, adaptor, cpi::Noflags)
{
    instance_data data(this);
        
    if (!data->rm_.get_url().empty())
    {
        saga::url rm(data->rm_);
        std::string host(rm.get_host());
        std::string scheme(rm.get_scheme());
        
        // check if we can handle url scheme
        if (scheme != "gram" && scheme !=  "any")
        {
            SAGA_OSSTREAM strm;
            strm << "Could not initialize job service for " << data->rm_ << ". " 
                 << "Only gram:// schemes are supported.";
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                               saga::adaptors::AdaptorDeclined); 
        }
        
        if (host.empty())
        {
            SAGA_OSSTREAM strm;
            strm << "Could not initialize job service for " << data->rm_ << ". " 
            << "URL doesn't define a hostname.";
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                               saga::adaptors::AdaptorDeclined); 
        }
    }
    else
    {
        SAGA_OSSTREAM strm;
        strm << "Could not initialize job service for " << data->rm_ << ". " 
             << "Resource discovery is not available yet.";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                           saga::adaptors::AdaptorDeclined); 
    }
    
    // check if we have x.509 contexts available and if they are usable
    // with this adaptor. if no context is usable, the constructor fails with
    // an authorization failed exception.
    std::vector <saga::context> contexts = p->get_session ().list_contexts ();
    std::vector <saga::context> context_list;
    // holds a list of reasons why a context can't be used. if no context
    // can be used, the list will be appended to the exception message otherwise
    // it will be discarded. 
    std::vector <std::string> context_error_list;
    
    for (unsigned int i = 0; i < contexts.size (); i++)
    {
      globus_adaptors_shared::check_x509_globus_cert(contexts[i], context_list, 
                                                    context_error_list);
    } 
    
    if(context_list.size() <1) {
        SAGA_OSSTREAM strm;
        strm << "Could not initialize job service for " << data->rm_ << ". "
             << "No valid and/or usable x.509 context could be found:\n";
        for(unsigned int i=0; i<context_error_list.size(); ++i) {
          strm << "    - " << context_error_list[i] << "\n";
        }
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm),
                           saga::AuthorizationFailed);
    }

    // If we've made it here, it should be safe to load
    // the GRAM modules now. The loader employs a sigleton mechanism,
    // so ut doesn't matter if we call this method multiple times.
    globus_module_loader::globus_init ();
}

///////////////////////////////////////////////////////////////////////////////
//
globus_gram_job_adaptor::job_service_cpi_impl::
~job_service_cpi_impl (void)
{
    
}

///////////////////////////////////////////////////////////////////////////////
//
void globus_gram_job_adaptor::job_service_cpi_impl::
sync_create_job(saga::job::job & ret, saga::job::description jd)
{
    instance_data data(this);   
    
    saga::attribute attr (jd);
    // A job description needs at least an 'Executable' 
    // attribute. Doesn't make sense without one.
    if (!attr.attribute_exists(saga::job::attributes::description_executable) ||
        attr.get_attribute(saga::job::attributes::description_executable).empty())
    {
        SAGA_OSSTREAM strm;
		strm << "Could not create a job object for " << data->rm_ << ". " 
             << "TheThe job description is missing the mandatory 'executable' attribute.";
		SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter); 
    }
    
    try {
        // Translate the given job description to a Globus RSL string. Throws.
        std::string rsl = utility::create_rsl_from_description(jd);
    }
    catch(globus_gram_job_adaptor::exception const & e)
    {
        SAGA_OSSTREAM strm;
		strm << "Could not create a job instance for " << data->rm_ << ". " 
             << e.GlobusErrorText();
		SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError()); 
    }
    
    
    saga::job::job job = saga::adaptors::job(data->rm_, jd, 
                                             proxy_->get_session());
    ret = job;
}

///////////////////////////////////////////////////////////////////////////////
//
void globus_gram_job_adaptor::job_service_cpi_impl::
sync_get_job(saga::job::job& ret, std::string jobid)
{
    instance_data instance_data(this);
    adaptor_data_type adaptor_data(this);
    
    saga::job::description jd;
    
    jd = adaptor_data->get_job_desc (instance_data->rm_.get_url(), jobid);
    
    ret = saga::adaptors::job (instance_data->rm_, jobid, 
                                   proxy_->get_session());
}

///////////////////////////////////////////////////////////////////////////////
//
void globus_gram_job_adaptor::job_service_cpi_impl::
sync_run_job_noio(saga::job::job & ret,
                  std::string host,
                  std::string commandline)
{
    instance_data data(this);

    std::string error_text("The adaptor couldn't run the job for the "
                           "following reason: ");
    
    // A usable job description needs at 
    // least an 'Executable' attribute.
    if (commandline.empty())
    {
        error_text.append("Empty 'commandline' parameter specified!");
        SAGA_ADAPTOR_THROW(error_text, saga::BadParameter);
    }
 
    std::vector<std::string> cmdline = saga::adaptors::utils::split_commandline (commandline);
    std::string executable = cmdline[0];
    
    // Try to convert the supplied 'host' into a Globus GRAM compliant
    // URL. If it doesn't work, throw an exception - job_run failed!
    std::string gram_url;

    gram_url += "gram://";
    gram_url += host;

    try {
        connector::ping_jobmanager(data->rm_.get_url());
    }
    catch(globus_gram_job_adaptor::exception const& e)
    {
        SAGA_OSSTREAM strm;
		strm << "Could not initialize job service [" << data->rm_ << "]. " 
        << e.GlobusErrorText();
		SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError()); 
    }
    
    // Let's create a job description so we can later attach it to the job 
    // object.
    saga::job::description jobdef_self;
    
    jobdef_self.set_attribute (saga::job::attributes::description_interactive, "False");
    //jobdef_self.set_attribute(saga::job::attributes::description_candidate_hosts, host);
    jobdef_self.set_attribute(saga::job::attributes::description_executable, executable);
    
    if (cmdline.size() > 1)
    {
        cmdline.erase(cmdline.begin());
        jobdef_self.set_vector_attribute(saga::job::attributes::description_arguments, 
                                cmdline);
    }
    
    adaptor_data_type adaptor_data(this);
    
    // Translate the created job description to
    // a Globus RSL string. If this fails, we abort.
    std::string rsl = utility::create_rsl_from_description(jobdef_self);
    
    saga::job::job job = saga::adaptors::job(data->rm_,
                                        jobdef_self, proxy_->get_session());
    
    // Set the job's creation timestamp attribute
    // to the current system time.
    std::time_t current = 0;
    std::time(&current);
    
    saga::adaptors::attribute jobattr (job);
    jobattr.set_attribute(saga::job::attributes::created, ctime(&current));
        
    // Finally, fire up the job.
    job.run();
    ret = job;
}

///////////////////////////////////////////////////////////////////////////////
//
void globus_gram_job_adaptor::job_service_cpi_impl::
sync_run_job(saga::job::job & ret, 
             std::string commandline, 
             std::string host, 
             saga::job::ostream& in, saga::job::istream& out, 
             saga::job::istream& err)
{
    instance_data data(this);

    std::string error_text("The adaptor couldn't run the job for the "
                           "following reason: ");
    
    // A usable job description needs at 
    // least an 'Executable' attribute.
    if (commandline.empty())
    {
        error_text.append("Empty 'commandline' parameter specified!");
        SAGA_ADAPTOR_THROW(error_text, saga::BadParameter);
    }
 
    std::vector<std::string> cmdline = saga::adaptors::utils::split_commandline (commandline);
    std::string executable = cmdline[0];
    
    // Try to convert the supplied 'host' into a Globus GRAM compliant
    // URL. If it doesn't work, throw an exception - job_run failed!
    std::string gram_url;

    gram_url += "gram://";
    gram_url += host;

    try {
        connector::ping_jobmanager(data->rm_.get_url());
    }
    catch(globus_gram_job_adaptor::exception const& e)
    {
        SAGA_OSSTREAM strm;
		strm << "Could not initialize job service [" << data->rm_ << "]. " 
        << e.GlobusErrorText();
		SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError()); 
    }
    
    // Let's create a job description so we can later attach it to the job 
    // object.
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
    
    adaptor_data_type adaptor_data(this);
    
    // Translate the created job description to
    // a Globus RSL string. If this fails, we abort.
    std::string rsl = utility::create_rsl_from_description(jobdef_self);
    
    saga::job::job job = saga::adaptors::job(data->rm_,
                                        jobdef_self, proxy_->get_session());
    
    // Set the job's creation timestamp attribute
    // to the current system time.
    std::time_t current = 0;
    std::time(&current);
    
    saga::adaptors::attribute jobattr (job);
    jobattr.set_attribute(saga::job::attributes::created, ctime(&current));
        
    // Finally, fire up the job.
    job.run();
    
    // Redirect the in/out/err streams
    out = job.get_stdout();
    err = job.get_stderr();
    in  = job.get_stdin();
    
    ret = job;
}

////////// class: job_service_cpi_impl ////////////////////////////////////////
////////// SYNC list_jobs /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void globus_gram_job_adaptor::job_service_cpi_impl::
sync_list(std::vector<std::string>& list_of_jobids)
{
    instance_data instance_data(this);
    adaptor_data_type adaptor_data(this);
  
    list_of_jobids = adaptor_data->list_jobs(instance_data->rm_.get_url());
}
