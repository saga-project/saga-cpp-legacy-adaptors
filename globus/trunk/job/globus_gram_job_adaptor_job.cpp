//  Copyright (c) 2007 Ole Christian Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "globus_gram_job_adaptor_job.hpp"
#include "globus_gram_job_adaptor_istream.hpp"
#include "globus_gram_job_adaptor_ostream.hpp"
#include "globus_gram_job_adaptor_connector.hpp"
#include "globus_gram_job_adaptor_errorhandler.hpp"

#include <saga/saga/util.hpp>
#include <saga/saga/exception.hpp>
#include <saga/saga/url.hpp>
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/metric.hpp>
#include <saga/saga/adaptors/attribute.hpp>
#include <saga/saga/adaptors/file_transfer_spec.hpp>


#include <saga/impl/config.hpp>

#include <saga/saga/packages/job/job_description.hpp>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/device/null.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/ref.hpp>

////////// class: job_cpi_impl ////////////////////////////////////////////////
////////// job_cpi_impl (CONSTRUCTOR) /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

globus_gram_job_adaptor::
job_cpi_impl::job_cpi_impl (proxy                * p, 
                            cpi_info       const & info,
                            saga::ini::ini const & glob_ini, 
                            saga::ini::ini const & adap_ini,
                            TR1::shared_ptr<saga::adaptor> adaptor)
: base_cpi (p, info, adaptor, cpi::Noflags), submitted_(false), staging_cookie_(0)
{
    std::string error_text("The adaptor couldn't construct the job "
                           "for the following reason: ");
    
    instance_data inst_data(this);
    
    // first of all, check if we can handle this request
    if (!inst_data->rm_.get_url().empty())
    {
        saga::url rm(inst_data->rm_);
        std::string host(rm.get_host());
        
        std::string scheme(rm.get_scheme());
        if (scheme != "gram" && scheme != "any")
        {
            SAGA_OSSTREAM strm;
            strm << "Could not initialize job for [" << inst_data->rm_ << "]. " 
            << "Only any:// and gram:// schemes are supported.";
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                               saga::adaptors::AdaptorDeclined); 
        }
        
        if (host.empty())
        {
            SAGA_OSSTREAM strm;
            strm << "Could not initialize job for [" << inst_data->rm_ << "]. " 
            << "URL doesn't define a hostname.";
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                               saga::adaptors::AdaptorDeclined); 
        }
    }
    else 
    {
        SAGA_OSSTREAM strm;
        strm << "Could not initialize job for [" << inst_data->rm_ << "]. " 
        << "Resource manager URL seems to be empty.";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                           saga::adaptors::AdaptorDeclined);         
    }

    
    pipe_to_gass_stderr_[0] = -1;
    pipe_to_gass_stderr_[1] = -1;
    
    pipe_to_gass_stdout_[0] = -1;
    pipe_to_gass_stdout_[1] = -1;
    
    // Inital job state is 'Unknown' since the job is not started yet.
    update_state(saga::job::Unknown);
    
    if (inst_data->init_from_jobid_) 
    {
        // Job was constructed by the get_job factory method with a JobID. 
        // This means that we have to connect to an existing job. If we can 
        // connect to the job, we have to:
        //   - set the current state
        //   - try to reconstruct the job description
        
        std::string error_text("The adaptor couldn't query the job's "
                               "status for the following reason: ");
        
        // saga_error_tuple et = { (saga::error)saga::adaptors::Success, "" };
        
        
        saga::job::state old_state = saga::job::Unknown;
        saga::job::state new_state = 
        connector::get_job_state(old_state, inst_data->jobid_);
        
        
        saga::adaptors::attribute attr (this);
        
        std::string rm = inst_data->rm_.get_url();
        std::vector<std::string> hosts;
        hosts.push_back(saga::url(rm).get_host());
        attr.set_vector_attribute (saga::job::attributes::execution_hosts, hosts); 
        attr.set_attribute (saga::job::attributes::jobid, inst_data->jobid_);
        
        update_state(new_state);
        submitted_ = true;
    }      
    else
    {
        // From now on the job is in 'New' state - ready to run!
        update_state(saga::job::New);
        
        // In case the "interactive" attribute is set, we have to set up
        // the local GASS server to handle I/O redirection.
        saga::job::description jd = inst_data->jd_;
        
        // if the interactive flag is not set: set it to false
        if(!jd.attribute_exists(saga::job::attributes::description_interactive))
        {
            jd.set_attribute(saga::job::attributes::description_interactive,
                             saga::attributes::common_false);
        }
        
        if (jd.get_attribute(saga::job::attributes::description_interactive) ==
            saga::attributes::common_true)
        {
            // Create pipes for stdout & stderr (GRAM doesn't support stdin redirection)
            if (pipe (pipe_to_gass_stderr_) == -1) 
            {
                error_text.append ("Couldn't create STDERR pipe because: ");
                error_text.append ( (strerror(errno)) );
                SAGA_ADAPTOR_THROW (error_text, (saga::error)saga::NoSuccess);
            }
            
            if (pipe (pipe_to_gass_stdout_) == -1) 
            {
                error_text.append ("Couldn't create STDOUT pipe because: ");
                error_text.append ( (strerror(errno)) );
                SAGA_ADAPTOR_THROW (error_text, (saga::error)saga::NoSuccess);
            }
            
            // Next, we need to start two local GASS servers for I/O redirection
            std::string server_listen_url_stdout;
            std::string server_listen_url_stderr;
            
            try {
                gass_server_[0] = common::saga_gass_server();
                server_listen_url_stdout = 
                gass_server_[0].start(pipe_to_gass_stdout_);
                
                gass_server_[1] = common::saga_gass_server();
                server_listen_url_stderr = 
                gass_server_[1].start(pipe_to_gass_stderr_);
            }
            catch( ... ) // Should catch instance of int...
            {
                error_text.append ("Couldn't start local GASS server because: ");
                // FIXME: because.... what?
                SAGA_ADAPTOR_THROW (error_text, saga::NoSuccess);
            }
            
            
            // Add the GASS URL as stdout URL.
            server_listen_url_stdout += "/dev/saga";
            server_listen_url_stderr += "/dev/saga";
            
            jd.set_attribute ("Output", server_listen_url_stdout);
            jd.set_attribute ("Error",  server_listen_url_stderr);
        }
        
        // Job was constructed by the create_job factory method with a 
        // job desc. This means that we have to create initialize the job's 
        // attributes and metrics with default values. 
        saga::adaptors::attribute attr (this);
        
        std::string rm = inst_data->rm_.get_url();
        std::vector<std::string> hosts;
        hosts.push_back(saga::url(rm).get_host());
        attr.set_vector_attribute (saga::job::attributes::execution_hosts, hosts); 
        
        std::time_t current = 0;
        std::time (&current);
        attr.set_attribute (saga::job::attributes::created, ctime (&current));   
        
        attr.set_attribute (saga::job::attributes::jobid,              "unknown");
        attr.set_attribute (saga::job::attributes::started,            "unknown");
        attr.set_attribute (saga::job::attributes::finished,           "unknown");
        attr.set_attribute (saga::job::attributes::working_directory,  "unknown");
        //attr.set_attribute (saga::job::attributes::exitcode,           "unknown");
        //attr.set_attribute (saga::job::attributes::termsig,            "unknown");
    }
}

////////// class: job_cpi_impl ////////////////////////////////////////////////
////////// ~job_cpi_impl (DESTRUCTOR) /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

globus_gram_job_adaptor::job_cpi_impl::~job_cpi_impl (void)
{
    //
}

////////// class: job_cpi_impl ////////////////////////////////////////////////
////////// run_job ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void globus_gram_job_adaptor::job_cpi_impl::sync_run (saga::impl::void_t&)

{
    saga_error_tuple et = { (saga::error)saga::adaptors::Success, "" };
    std::string error_text("Unable run the job because: ");
    
    instance_data inst_data (this);
    saga::job::description jd = inst_data->jd_;  // Job description
    std::string           rm = inst_data->rm_.get_url();  // RM 'host' string
    
    saga::job::state state;
    sync_get_state(state);
    
    // If the job is not in 'New' state, we can't run it.
    if (saga::job::New != state) 
    {
        error_text.append("The job has already been started!");
        SAGA_ADAPTOR_THROW(error_text, saga::IncorrectState);
    }
    
    // If the jd has the "Interactive" flag set
    
    // Try to submit the job to the Globus GRAM jobmanager. If it works - 
    // fine. If not, throw an exception - job submission failed!
    std::string jobID("");
    saga::job::state new_state;
    et = connector::submit_job (jobID, new_state, rm, jd);
    if (et.error != (saga::error)saga::adaptors::Success) 
    {
        //error_text.append("Couldn't submit the job to '"+rm+"' because: ");
        error_text.append(et.desc_informal);
        SAGA_ADAPTOR_THROW (error_text, et.error);
    }
    
    submitted_ = true; 
    //sleep(2); // FIXME: Fix that _somehow_...
    
    // Set the assigne job id
    saga::adaptors::attribute attr (this);
    attr.set_attribute (saga::job::attributes::jobid, jobID);
    
    // Add the job to the job services' list of known jobs.
    adaptor_data_type data(this);
    data->register_job(rm, jobID, jd);
    
    // Set the new job status // FIXME (ow): doesn't work :(
    //
    // saga::job::state old_state = get_saga_job_state ();
    //  
    // et = connector::get_job_state(old_state, new_state, jobID);
    //  
    // if (et.error != (saga::error)saga::adaptors::Success) 
    // {
    //     error_text.append(et.desc_informal);
    //     SAGA_ADAPTOR_THROW (error_text, et.error);
    // }
    // update_state(new_state);
    
    // Set to running for now.  Any later checks will show what
    // happened.
    update_state(saga::job::Running);
}

////////// class: job_cpi_impl ////////////////////////////////////////////////
////////// sync_get_state /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void globus_gram_job_adaptor::
job_cpi_impl::sync_get_state (saga::job::state& state)
{
    if (submitted_)
    {
        // Since the job is submitted, we can ask the GRAM job manager
        // for the job's current state
        
        std::string error_text("The adaptor couldn't query the job's "
                               "status for the following reason: ");
        
        // saga_error_tuple et = { (saga::error)saga::adaptors::Success, "" };
        
        saga::adaptors::attribute attr (this);
        std::string job_id = attr.get_attribute (saga::job::attributes::jobid);
        
        
        
        saga::job::state old_state = get_saga_job_state ();
        saga::job::state new_state = 
        connector::get_job_state(old_state, job_id);
        
        update_state(new_state);
    }
    
    saga::monitorable monitor (this->proxy_);
    saga::metric m (monitor.get_metric(saga::metrics::task_state));
    state = saga::adaptors::job_state_value_to_enum(
                                                    m.get_attribute(saga::attributes::metric_value));
}

////////// class: job_cpi_impl ////////////////////////////////////////////////
////////// update_state ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void globus_gram_job_adaptor::
job_cpi_impl::update_state(saga::job::state newstate)
{
    saga::monitorable monitor (this->proxy_);
    saga::adaptors::metric m (monitor.get_metric(saga::metrics::task_state));
    m.set_attribute(saga::attributes::metric_value, 
                    saga::adaptors::job_state_enum_to_value(newstate));
}

///////////////////////////////////////////////////////////////////////////////
//
void globus_gram_job_adaptor::
job_cpi_impl::sync_get_job_id (std::string& jobid)
{
    saga::attribute attr (this->proxy_);
    jobid = attr.get_attribute(saga::job::attributes::jobid);
}

///////////////////////////////////////////////////////////////////////////////
//
void globus_gram_job_adaptor::
job_cpi_impl::sync_cancel (saga::impl::void_t&, double timeout)
{        
    saga::adaptors::attribute attr (this);
    std::string job_id = attr.get_attribute (saga::job::attributes::jobid);
    
    try {
        saga::job::state s; this->sync_get_state(s);
        if(s == saga::job::New) {
            throw exception("job is in saga::job::New state.",
                            saga::IncorrectState);
        }
        else if(s == saga::job::Running) {
            connector::cancel_job (job_id);
            update_state(saga::job::Canceled);
        }
    }
    catch(globus_gram_job_adaptor::exception const & e)
    {
        SAGA_OSSTREAM strm;
        strm << "Could not cancel job [" << job_id << "]. " 
        << e.GlobusErrorText();
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError()); 
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void globus_gram_job_adaptor::
job_cpi_impl::sync_wait (bool& ret, double wait)
{
    saga::adaptors::attribute attr (this);
    std::string job_id = attr.get_attribute (saga::job::attributes::jobid);
    
    double wait_count = 0.0;
    saga::job::state s; 
    ret = false;
    
    try {
        this->sync_get_state(s);
        if(s == saga::job::New) {
            throw exception("job is in saga::job::New state.",
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
    catch(globus_gram_job_adaptor::exception const & e)
    {
        SAGA_OSSTREAM strm;
        strm << "Could not wait for job [" << job_id << "]. " 
        << e.GlobusErrorText();
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError()); 
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void globus_gram_job_adaptor::
job_cpi_impl::sync_suspend (saga::impl::void_t&)
{
    instance_data inst_data (this);
    
    if (submitted_)
    {
        saga::adaptors::attribute attr (this);
        std::string job_id = attr.get_attribute (saga::job::attributes::jobid);
        
        try {
            connector::signal_job(GLOBUS_GRAM_PROTOCOL_JOB_SIGNAL_SUSPEND, 
                                  "", job_id);
            update_state(saga::job::Suspended);
        }
        catch(globus_gram_job_adaptor::exception const & e)
        {
            SAGA_OSSTREAM strm;
            strm << "Could not suspend job [" << job_id << "]. " 
            << e.GlobusErrorText();
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError()); 
        }
    }
}

////////// class: job_cpi_impl ////////////////////////////////////////////////
////////// sync_resume() //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void globus_gram_job_adaptor::
job_cpi_impl::sync_resume (saga::impl::void_t&)
{
    instance_data inst_data (this);
    saga::job::description jd = inst_data->jd_;
    std::string           rm = inst_data->rm_.get_url();
    std::string    gram_host = saga::url(rm).get_host();
    
    SAGA_ADAPTOR_THROW("not yet implemented", saga::NotImplemented);
    //FIXME: implementation
}

////////// class: job_cpi_impl ////////////////////////////////////////////////
////////// sync_get_stdin() ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void globus_gram_job_adaptor::
job_cpi_impl::sync_get_stdin(saga::job::ostream& ostrm)
{
    SAGA_OSSTREAM strm;
    strm << "get_stdin() is not implemented. Input streams are NOT supported by GRAM.";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::NotImplemented);
}

////////// class: job_cpi_impl ////////////////////////////////////////////////
////////// sync_get_stdout() //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void globus_gram_job_adaptor::
job_cpi_impl::sync_get_stdout(saga::job::istream& istrm)
{
    std::string error_text("Couldn't connect to the job's STDOUT stream for "
                           "the following reason: ");
    
    instance_data inst_data (this);
    saga::job::description jd = inst_data->jd_;
    
    if (!jd.attribute_exists(saga::job::attributes::description_interactive) ||
        jd.get_attribute(saga::job::attributes::description_interactive) !=
        saga::attributes::common_true)
    {
        error_text.append ("Job was not started with the interactive flag,"
                           " STDOUT redirection is not available.");
        SAGA_ADAPTOR_THROW (error_text, saga::IncorrectState);
    }
    
    if (pipe_to_gass_stdout_[0] == -1) 
    {
        error_text.append ("Pipe handle to the GASS server is invalid.");
        SAGA_ADAPTOR_THROW (error_text, saga::NoSuccess);
    }
    
    mutex_type::scoped_lock lock(mtx_);
    
    
    globus_gram_job_adaptor_istream 
    gass_strm(this, pipe_to_gass_stdout_[0]);
    
    
    istrm = gass_strm;
}

////////// class: job_cpi_impl ////////////////////////////////////////////////
////////// sync_get_stderr() //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void globus_gram_job_adaptor::
job_cpi_impl::sync_get_stderr(saga::job::istream& errstrm)
{
    std::string error_text ("Couldn't connect to the job's STDERR stream for "
                            "the following reason: ");
    
    instance_data inst_data (this);
    saga::job::description jd = inst_data->jd_;
    
    if (!jd.attribute_exists(saga::job::attributes::description_interactive) ||
        jd.get_attribute(saga::job::attributes::description_interactive) !=
        saga::attributes::common_true)
    {
        error_text.append ("This job was not started with the interactive flag,"
                           " STDERR redirection is not available.");
        SAGA_ADAPTOR_THROW (error_text, saga::IncorrectState);
    }
    
    if (pipe_to_gass_stderr_[0] == -1) 
    {
        error_text.append ("The current pipe handle to GASS server is invalid.");
        SAGA_ADAPTOR_THROW (error_text, saga::NoSuccess);
    }
    
    mutex_type::scoped_lock lock(mtx_);
    
    globus_gram_job_adaptor_istream 
    gass_strm(this, pipe_to_gass_stderr_[0]);
    
    errstrm = gass_strm;
}

////////// class: job_cpi_impl ////////////////////////////////////////////////
////////// get_saga_job_state() ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

saga::job::state globus_gram_job_adaptor::
job_cpi_impl::get_saga_job_state (void)
{
    saga::monitorable monitor (this->proxy_);
    
    saga::metric m (monitor.get_metric(saga::metrics::task_state));
    
    saga::job::state state = saga::adaptors::job_state_value_to_enum(
                                                                     m.get_attribute(saga::attributes::metric_value));
    
    return (state);
}

////////// class: job_cpi_impl ////////////////////////////////////////////////
////////// register_post_staging() ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void globus_gram_job_adaptor::job_cpi_impl::register_post_staging()
{
    mutex_type::scoped_lock l(mtx_);
    saga::monitorable monitor (this->proxy_);
    staging_cookie_ = monitor.add_callback(saga::metrics::task_state, 
                                           boost::bind(&job_cpi_impl::do_post_staging, this, _1, _2, _3));
}


////////// class: job_cpi_impl ////////////////////////////////////////////////
////////// do_pre_staging() ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void globus_gram_job_adaptor::
job_cpi_impl::do_pre_staging (saga::job::description jd)
{
    if (!jd.attribute_exists(saga::job::attributes::description_file_transfer))
        return;
    
    // get the staging specifications
    std::vector<std::string> specs (
                                    jd.get_vector_attribute(saga::job::attributes::description_file_transfer));
    
    std::vector<std::string>::iterator end = specs.end();
    for (std::vector<std::string>::iterator it = specs.begin(); it != end; ++it)
    {
        using namespace saga::adaptors;
        std::string left_url, right_url;
        file_transfer_operator mode;
        if (!parse_file_transfer_specification(*it, left_url, mode, right_url))
        {
            SAGA_ADAPTOR_THROW(
                               "job_cpi_impl::do_pre_staging: "
                               "ill formatted file transfer specification: " + *it,
                               saga::BadParameter);
        }
        
        if (copy_local_remote == mode) {
            //copy the file
            //SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_DEBUG) {
            //  std::cerr << "globus GRAM Adaptor: X.509 context found pointing to user proxy at " 
            //        << userproxy << std::endl;

            //saga::filesystem::file f(left_url);
            //f.copy(right_url, saga::filesystem::Overwrite);
        }
        else if (append_local_remote == mode) {
            // append to remote file
            /*saga::filesystem::file in(left_url, saga::filesystem::Read|saga::filesystem::Binary);
             saga::filesystem::file out(right_url, 
             saga::filesystem::ReadWrite|saga::filesystem::Append|saga::filesystem::Binary);
             
             saga::off_t size = in.get_size();
             if (size > 10000) size = 10000;
             std::auto_ptr<char> buffer (new char[size]);
             saga::ssize_t bytes = in.read(saga::buffer(buffer.get(), size));
             while (bytes != 0) {
             out.write(saga::buffer(buffer.get(), bytes));
             bytes = in.read(saga::buffer(buffer.get(), size));
             }*/
        }
    }
}


////////// class: job_cpi_impl ////////////////////////////////////////////////
////////// do_post_staging() //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool globus_gram_job_adaptor::
job_cpi_impl::do_post_staging (saga::object, saga::metric, saga::context)
{
    // make sure post staging is done in Done state only
    {
        saga::job::state state;
        this->sync_get_state(state);
        
        if (saga::job::Failed == state) {
            mutex_type::scoped_lock l(mtx_);
            staging_cookie_ = 0;
            return false;   // we are done, don't call callback anymore
        }
        if (saga::job::Done != state)
            return true;    // continue calling this callback
    }
    
    // get the job description of this job    
    saga::job::description jd;
    {
        instance_data data(this);
        jd = data->jd_;
    }
    
    if (!jd.attribute_exists(saga::job::attributes::description_file_transfer)) {
        mutex_type::scoped_lock l(mtx_);
        staging_cookie_ = 0;
        return false;           // unregister callback
    }
    
    // get the staging specifications
    std::vector<std::string> specs (
                                    jd.get_vector_attribute(saga::job::attributes::description_file_transfer));
    
    std::vector<std::string>::iterator end = specs.end();
    for (std::vector<std::string>::iterator it = specs.begin(); it != end; ++it)
    {
        using namespace saga::adaptors;
        std::string left_url, right_url;
        file_transfer_operator mode;
        if (!parse_file_transfer_specification(*it, left_url, mode, right_url))
        {
            SAGA_ADAPTOR_THROW(
                               "job_cpi_impl::do_post_staging: "
                               "ill formatted file transfer specification: " + *it,
                               saga::BadParameter);
        }
        
        if (copy_remote_local == mode) {
            // copy the file
            //saga::filesystem::file f(right_url);
            //f.copy(left_url, saga::filesystem::Overwrite);
        }
        else if (append_remote_local == mode) {
            // append to local file
            //saga::filesystem::file in(right_url, saga::filesystem::Read|saga::filesystem::Binary);
            //saga::filesystem::file out(left_url, 
            //                           saga::filesystem::ReadWrite|saga::filesystem::Append|saga::filesystem::Binary);
            
            //saga::off_t size = in.get_size();
            //if (size > 10000) size = 10000;
            //std::auto_ptr<char> buffer (new char[size]);
            //saga::ssize_t bytes = in.read(saga::buffer(buffer.get(), size));
            //while (bytes != 0) {
            //    out.write(saga::buffer(buffer.get(), bytes));
            //    bytes = in.read(saga::buffer(buffer.get(), size));
            //}
        }
    }
    
    mutex_type::scoped_lock l(mtx_);
    staging_cookie_ = 0;
    return false;         // un-register this metric callback
}

