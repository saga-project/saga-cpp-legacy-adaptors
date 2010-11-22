//  Copyright (c) 2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "globus_gram_job_adaptor_connector.hpp"
#include "globus_gram_job_adaptor_errorhandler.hpp"

#include "./saga_gass_server/globus_gram_job_adaptor_gass_server_ez.h"
#include "globus_gram_protocol.h"

#include <saga/impl/exception.hpp>
#include <saga/saga/adaptors/file_transfer_spec.hpp>

#define DBG_PRFX    "### GLOBUS GRAM2 ADAPTOR ### "


#include <boost/lexical_cast.hpp>

using namespace globus_gram_job_adaptor;

namespace {
    
    ////////////////////////////////////////////////////////////////////////////
    //
    inline bool split_environment(std::string const& env, std::string& key, 
                                  std::string & value)
    {
        std::string::size_type pos = env.find_first_of("=");
        if (std::string::npos == pos)
            return false;
        
        key = env.substr(0, pos);
        value = env.substr(pos+1);
        
        return true;
    }
    
    ////////////////////////////////////////////////////////////////////////////
    // 'itoa' - print integers in a given base 2-16 (default 10)
    // http://www.cs.princeton.edu/courses/archive/fall96/cs126/examples/itoa.c
    //
    inline int convert(int n, int b, char str[], int i) {
        if (n/b > 0)
            i = convert(n/b, b, str, i);
        str[i++] = "0123456789ABCDEF"[n%b];
        return i;
    }
    
    inline int itoa(int n, int b, char str[]) {
        int i = convert(n, b, str, 0);
        str[i] = '\0';
        return i;
    }
    
    
    enum
    {
        GLOBUSRUN_ARG_INTERACTIVE           = 1,
        GLOBUSRUN_ARG_QUIET                 = 2,
        GLOBUSRUN_ARG_DRYRUN                = 4,
        GLOBUSRUN_ARG_PARSE_ONLY            = 8,
        GLOBUSRUN_ARG_AUTHENTICATE_ONLY     = 16,
        GLOBUSRUN_ARG_USE_GASS              = 32,
        GLOBUSRUN_ARG_ALLOW_READS           = 64,
        GLOBUSRUN_ARG_ALLOW_WRITES          = 128,
        GLOBUSRUN_ARG_IGNORE_CTRLC          = 256,
        GLOBUSRUN_ARG_BATCH                 = 512,
        GLOBUSRUN_ARG_STATUS                = 1024,
        GLOBUSRUN_ARG_LIST                  = 2048,
        GLOBUSRUN_ARG_BATCH_FAST            = 4096
    };
    
}

///////////////////////////////////////////////////////////////////////////////
//
void connector::globus_l_globusrun_gram_callback_func(void *user_arg,
                                                      char *job_contact,
                                                      int state,
                                                      int errorcode)
{
    globus_i_globusrun_gram_monitor_t *monitor;
    
    monitor = (globus_i_globusrun_gram_monitor_t *) user_arg;
    
    (void)globus_mutex_lock(&monitor->mutex);
    monitor->submit_done = GLOBUS_TRUE;
    
    if(monitor->job_contact != NULL &&
       (strcmp(monitor->job_contact, job_contact) != 0))
    {
        (void)globus_mutex_unlock(&monitor->mutex);
        return;
    }
    
    monitor->job_state = state;
    
    switch(state)
    {
      case GLOBUS_GRAM_PROTOCOL_JOB_STATE_PENDING:
          if(monitor->verbose)
          {
              globus_libc_printf("GLOBUS_GRAM_PROTOCOL_JOB_STATE_PENDING\n");
          }
          break;
      case GLOBUS_GRAM_PROTOCOL_JOB_STATE_STAGE_IN:
          if(monitor->verbose)
          {
              globus_libc_printf("GLOBUS_GRAM_PROTOCOL_JOB_STATE_STAGE_IN\n");
          }
          break;
      case GLOBUS_GRAM_PROTOCOL_JOB_STATE_STAGE_OUT:
          if(monitor->verbose)
          {
              globus_libc_printf("GLOBUS_GRAM_PROTOCOL_JOB_STATE_STAGE_OUT\n");
          }
          break;
      case GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE:
          if(monitor->verbose)
          {
              globus_libc_printf("GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE\n");
          }
          break;
      case GLOBUS_GRAM_PROTOCOL_JOB_STATE_FAILED:
          if(monitor->verbose)
          {
              globus_libc_printf("GLOBUS_GRAM_PROTOCOL_JOB_STATE_FAILED\n");
          }
          monitor->done = GLOBUS_TRUE;
          monitor->failure_code = errorcode;
          break;
      case GLOBUS_GRAM_PROTOCOL_JOB_STATE_DONE:
          if(monitor->verbose)
          {
              globus_libc_printf("GLOBUS_GRAM_PROTOCOL_JOB_STATE_DONE\n");
          }
          monitor->done = GLOBUS_TRUE;
          break;
      case GLOBUS_GRAM_PROTOCOL_JOB_STATE_UNSUBMITTED:
          if(monitor->verbose)
          {
              globus_libc_printf("GLOBUS_GRAM_PROTOCOL_JOB_STATE_UNSUBMITTED\n");
          }
          monitor->done = GLOBUS_TRUE;
          break;
    }
    
    (void)globus_cond_signal(&monitor->cond);
    (void)globus_mutex_unlock(&monitor->mutex);
} 




///////////////////////////////////////////////////////////////////////////////
//
void connector::init_connector (gss_cred_id_t &credential)
{    
    OM_uint32 major_status, minor_status;
    major_status = gss_acquire_cred ( &minor_status,
                                     GSS_C_NO_NAME,
                                     GSS_C_INDEFINITE,
                                     GSS_C_NO_OID_SET,
                                     GSS_C_BOTH,
                                     &credential,
                                     NULL,
                                     NULL );
    
    // GSS uses a different error handling so we cannot use our errorhandler
    // class to process it. Or... if we know the errorcode we want to provoke,
    // we can _fake_ it: a failed acquire_cred should always result in a 
    // saga::AuthenticationFailed...
    if ( major_status != GSS_S_COMPLETE ) {
        throw globus_gram_job_adaptor::exception(
                                                 "Credentials are invalid or do not exist (grid-proxy-init?)",
                                                 saga::AuthorizationFailed);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void connector::release_connector (gss_cred_id_t &credential)
{
    
    // Cleanup: Release GSS credentials
    OM_uint32 major_status, minor_status;
    major_status = gss_release_cred( &minor_status, &credential);
    
    if ( major_status != GSS_S_COMPLETE ) {
        throw globus_gram_job_adaptor::exception(
                                                 "Couldn't release credentials.", saga::NoSuccess);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void connector::ping_jobmanager (const std::string & contact)
{    
    gss_cred_id_t credential;
    init_connector(credential);
    
    std::string gram_url = utility::translate_saga_to_gram_url(contact);
    globus_result_t rc = globus_gram_client_ping (gram_url.c_str()); 
    if ( GLOBUS_SUCCESS != rc ) 
        throw globus_gram_job_adaptor::exception(rc);
    
    release_connector(credential);
}

////////// class: connector ///////////////////////////////////////////////////
////////// saga_error_tuple submit_job ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

saga_error_tuple connector::submit_job (std::string & ret_jobid,
                                        saga::job::state  & ret_state,
                                        const std::string & contact,
                                        const saga::job::description & jd)
{
    errorhandler eh;
    saga_error_tuple et = { (saga::error)saga::adaptors::Success, "" };
    unsigned long options = 0UL;
    
    // translate the saga::job_secription into a Globus RSL string
    
    std::string rsl = utility::create_rsl_from_description (jd);
    
    SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_DEBUG) {
        std::cerr << DBG_PRFX << "RSL: " << rsl << std::endl;
    }
    
    // convert the saga url to a GRAM url
    std::string  gram_url = utility::translate_saga_to_gram_url(contact);
    
    options = GLOBUSRUN_ARG_BATCH;
    if(jd.attribute_exists(saga::job::attributes::description_interactive)) {
        if(saga::attributes::common_true == jd.get_attribute(saga::job::attributes::description_interactive)) {
            options |= GLOBUSRUN_ARG_INTERACTIVE;
        }
    }
    
    
    
    char *callback_contact = GLOBUS_NULL;
    globus_i_globusrun_gram_monitor_t monitor;
    globus_bool_t verbose = (options & GLOBUSRUN_ARG_QUIET);
    globus_bool_t send_commit = GLOBUS_FALSE;
    int err, tmp1, tmp2;
    
    // initialize the monitor struct
    monitor.done = GLOBUS_FALSE;
    monitor.failure_code = 0;
    monitor.verbose=verbose;
    monitor.job_state = 0;
    monitor.job_contact = NULL;
    monitor.submit_done = GLOBUS_FALSE;
    
    // initialize the mutex
    globus_mutex_init(&monitor.mutex, GLOBUS_NULL);
    (void)globus_cond_init(&monitor.cond, GLOBUS_NULL);
    
    err = globus_gram_client_callback_allow(
                                            globus_l_globusrun_gram_callback_func,
                                            (void *) &monitor,
                                            &callback_contact);
    
    if(err != GLOBUS_SUCCESS) {
        SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_ERROR) {
            std::cerr << DBG_PRFX  
            << "Initializing GRAM Callback failed because: "
            << globus_gram_protocol_error_string(err)
            << "(" << err << ")" << std::endl;
        }
        
        // clean up 
        if(monitor.job_contact != GLOBUS_NULL)
            globus_gram_client_job_contact_free(monitor.job_contact);
        return eh.get_saga_exception_for ("submit_job", err);
    }
    
    (void)globus_mutex_lock(&monitor.mutex);
    {
        err = globus_gram_client_job_request(gram_url.c_str(),
                                             rsl.c_str(),
                                             GLOBUS_GRAM_PROTOCOL_JOB_STATE_ALL,
                                             callback_contact,
                                             &monitor.job_contact);
    }
     
    if(err == GLOBUS_SUCCESS)
    {
        while (!monitor.submit_done)
        {
          globus_cond_wait(&monitor.cond, &monitor.mutex);
          err = monitor.failure_code;
        }
    }

    (void)globus_mutex_unlock(&monitor.mutex);
    
    if(err == GLOBUS_GRAM_PROTOCOL_ERROR_WAITING_FOR_COMMIT) 
    {
        SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_DEBUG) {
            std::cerr << DBG_PRFX 
            << globus_gram_protocol_error_string(err)
            << "(" << err << ")" 
            << " trying to send COMMIT signal." << std::endl;
        }
        send_commit = GLOBUS_TRUE;
        err = globus_gram_client_job_signal(
                                            monitor.job_contact,
                                            GLOBUS_GRAM_PROTOCOL_JOB_SIGNAL_COMMIT_REQUEST,
                                            "commit", &tmp1, &tmp2);
    }
    
    if(err != GLOBUS_SUCCESS) 
    {
        if(callback_contact) {
            globus_gram_client_callback_disallow(callback_contact);
            globus_free(callback_contact);
        }
        if((err == GLOBUS_GRAM_PROTOCOL_ERROR_DRYRUN) &&
           (options & GLOBUSRUN_ARG_DRYRUN))
        {
            SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_DEBUG) {
                std::cerr << DBG_PRFX 
                << "Dryrun successfull" << std::endl;
            }
            err=0;
        }
        else {
            SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_ERROR) {
                std::cerr << DBG_PRFX << "GRAM2 job submission failed because "
                << globus_gram_protocol_error_string(err)
                << "(" << err << ")" << std::endl;
            }
        }
        if ((options & GLOBUSRUN_ARG_BATCH) && monitor.job_contact) {
            globus_libc_printf("%s\n",monitor.job_contact);
        }
        
        // clean up & leave this town...
        if(monitor.job_contact != GLOBUS_NULL)
            globus_gram_client_job_contact_free(monitor.job_contact);
        return eh.get_saga_exception_for ("submit_job", err);
    }
    else {
        SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_DEBUG) {
            std::cerr << DBG_PRFX 
            << "GRAM2 job submission successful: " 
            << monitor.job_contact << std::endl;
        }
    }
    
    (void)globus_mutex_lock(&monitor.mutex);
    
    // If we're running in fast batch mode, and don't need to allow GASS
    // reads for staging, we will exit immediately without waiting for any
    // job state callbacks.
    //
    //if((options & (GLOBUSRUN_ARG_BATCH | GLOBUSRUN_ARG_ALLOW_READS
    //    | GLOBUSRUN_ARG_BATCH_FAST)) == (GLOBUSRUN_ARG_BATCH|GLOBUSRUN_ARG_BATCH_FAST))
    if((options & GLOBUSRUN_ARG_BATCH) || (options & GLOBUSRUN_ARG_BATCH_FAST))
    {
        monitor.done = GLOBUS_TRUE;
    }
    
    while(!monitor.done)
    {
        /* If we're running in batch mode and need to allow for GASS reads
         * we have to wait until the job is submitted and finished staging
         */
        if ((options & GLOBUSRUN_ARG_BATCH) &&
            (monitor.job_state != 0 &&
             monitor.job_state != GLOBUS_GRAM_PROTOCOL_JOB_STATE_UNSUBMITTED
             &&
             monitor.job_state != GLOBUS_GRAM_PROTOCOL_JOB_STATE_STAGE_IN))
        {
            monitor.done = GLOBUS_TRUE;
            continue;
        }
        (void)globus_cond_wait(&monitor.cond, &monitor.mutex);
        
        // WE COULD TRY TO DO SOME CTRL+C HANDLING HERE. BUT THAT'S SOMEWHAT RISKY
        // SINCE WE'RE INSIDE A LIBRARY. BUT THIS IS WHERE IT SHOULD GO.
    }
    
    (void)globus_mutex_unlock(&monitor.mutex);
    
    // If we're using two phase commits then we need to send commit end
    // signal if the job is DONE
    //
    err = GLOBUS_SUCCESS;
    if (monitor.job_state == GLOBUS_GRAM_PROTOCOL_JOB_STATE_DONE ||
        monitor.job_state == GLOBUS_GRAM_PROTOCOL_JOB_STATE_FAILED)
    {
        if(send_commit == GLOBUS_TRUE) {
            err = globus_gram_client_job_signal(monitor.job_contact,
                                                GLOBUS_GRAM_PROTOCOL_JOB_SIGNAL_COMMIT_END,
                                                "commit",
                                                &tmp1,
                                                &tmp2);
        }
        if (monitor.job_state == GLOBUS_GRAM_PROTOCOL_JOB_STATE_FAILED) {
            err = monitor.failure_code;
        }
    }
    else {
        err = monitor.failure_code;
    }
    
    if (options & GLOBUSRUN_ARG_BATCH) {
        globus_gram_client_job_callback_unregister(
                                                   monitor.job_contact,
                                                   callback_contact,
                                                   &tmp1,
                                                   &tmp2);
    }
    
    globus_gram_client_callback_disallow(callback_contact);
    globus_free(callback_contact);
    
    globus_mutex_destroy(&monitor.mutex);
    (void)globus_cond_destroy(&monitor.cond);
    
    if((err == GLOBUS_GRAM_PROTOCOL_ERROR_DRYRUN) &&
       (options & GLOBUSRUN_ARG_DRYRUN))
    {
        // In case of a dry run this seems to be ok ;-)
        SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_DEBUG) {
            std::cerr << DBG_PRFX << "Dryrun successfull" << std::endl;
        }
    }
    else if(err != GLOBUS_SUCCESS) {
        SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_ERROR) {
            std::cerr << DBG_PRFX << "GRAM Job failed because "
            << globus_gram_protocol_error_string(err)
            << "(" << err << ")" << std::endl;
            
        }
        // clean up ...
        if(monitor.job_contact != GLOBUS_NULL)
            globus_gram_client_job_contact_free(monitor.job_contact);
        et = eh.get_saga_exception_for ("submit_job", err);
    }
    else {
        ret_jobid = std::string(monitor.job_contact);
        ret_state = utility::translate_gram_to_saga_job_state (saga::job::New, 
                                                               monitor.job_state);
        SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_DEBUG) {
            std::cerr << "jobID: " << ret_jobid
            << " state: " << ret_state << std::endl;
        }
        
        /*if(ret_state == saga::job::Unknown)
        {
        	et = eh.get_saga_exception_for ("submit_job", 999999999);	
        }*/
    }
    
    // If nothing has happend so far, we're fine!
    return et; // = { (saga::error)saga::adaptors::Success, "" };
}

///////////////////////////////////////////////////////////////////////////////
//
saga::job::state connector::get_job_state (saga::job::state old_state,
                                           const std::string & job_contact)
{
    // no need to check again if state is final anyway.
    if(old_state == saga::job::Done || old_state == saga::job::Failed ||
       old_state == saga::job::Canceled )    
        return old_state;
    
    gss_cred_id_t credential;
    
    init_connector(credential);
    
    int state = GLOBUS_GRAM_PROTOCOL_JOB_STATE_UNSUBMITTED;
    int error = 0;
    
    globus_result_t rc = 
    globus_gram_client_job_status ( job_contact.c_str(),
                                   &state,
                                   &error );  // FIXME: check type
    
    if ( GLOBUS_SUCCESS != rc ) {
        // this is a nasty hack, but that's how the Globus guys
        // do it in globusrun.c
        if (GLOBUS_GRAM_PROTOCOL_ERROR_CONTACTING_JOB_MANAGER == rc) {
            state = GLOBUS_GRAM_PROTOCOL_JOB_STATE_DONE;
        }
        else {
            // every other protocol error is hopefully a _real_ error
            throw globus_gram_job_adaptor::exception(rc);
        }
    }
    
    saga::job::state ret_state = 
    utility::translate_gram_to_saga_job_state (old_state, state); 
    
    release_connector(credential);
    
    return ret_state; 
}

///////////////////////////////////////////////////////////////////////////////
//
void connector::cancel_job (const std::string & job_contact)
{
    gss_cred_id_t credential;
    
    init_connector(credential);
    
    globus_result_t rc = globus_gram_client_job_cancel (job_contact.c_str());
    if ( GLOBUS_SUCCESS != rc ) 
        throw globus_gram_job_adaptor::exception(rc);
    
    release_connector(credential);
}

///////////////////////////////////////////////////////////////////////////////
//
void connector::signal_job (globus_gram_protocol_job_signal_t signal,
                            const std::string & signal_args,             
                            const std::string & job_contact)
{    
    gss_cred_id_t credential;
    
    // initialize GSS/GRAM
    init_connector(credential);
    
    int errorcode = GLOBUS_SUCCESS; 
    int new_state = 0;
    
    globus_result_t rc = 
    globus_gram_client_job_signal (job_contact.c_str(),
                                   signal,
                                   signal_args.c_str(),
                                   &new_state, 
                                   &errorcode);
    
    if ( GLOBUS_SUCCESS != rc ) 
        throw globus_gram_job_adaptor::exception(rc);  
    
    release_connector(credential);
}

///////////////////////////////////////////////////////////////////////////////
//
saga::job::state utility::translate_gram_to_saga_job_state (const saga::job::state & old_state,
                                                            int gram_state)
{
    // NOTE: 
    // - For a description of GRAM job states 
    //   see globus_gram_protocol_constants.h 
    // - For a description of SAGA job states
    //   see /saga/saga/packages/job/job.hpp  
    
    saga::job::state saga_state = saga::job::Unknown;
    
    switch ( gram_state )
    {
      case GLOBUS_GRAM_PROTOCOL_JOB_STATE_PENDING     :  /*   1 */ 
          saga_state = saga::job::Running;              /*   1 */
          break;
          
      case GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE      :  /*   2 */ 
          saga_state = saga::job::Running;              /*   1 */
          break;
          
      case GLOBUS_GRAM_PROTOCOL_JOB_STATE_FAILED      :  /*   4 */ 
          saga_state = saga::job::Failed;               /*   2 */
          break;
          
      case GLOBUS_GRAM_PROTOCOL_JOB_STATE_DONE        :  /*   8 */ 
          saga_state = saga::job::Done;                 /*   3 */
          break;
          
      case GLOBUS_GRAM_PROTOCOL_JOB_STATE_UNSUBMITTED :  /*  32 */ 
          saga_state = old_state;                       /*   ? */
          break;
          
      case GLOBUS_GRAM_PROTOCOL_JOB_STATE_SUSPENDED   :  /*  16 */ 
          saga_state = saga::job::Suspended;            /*   5 */
          break;
          
      case GLOBUS_GRAM_PROTOCOL_JOB_STATE_STAGE_IN    :  /*  64 */ 
          saga_state = saga::job::Running;              /*   1 */
          break;
          
      case GLOBUS_GRAM_PROTOCOL_JOB_STATE_STAGE_OUT   :  /* 128 */ 
          saga_state = saga::job::Running;              /*   1 */
          break;
          
      default:
          saga_state = saga::job::Unknown;          
          break;
    }
    
    return saga_state;
}


///////////////////////////////////////////////////////////////////////////////
//
std::string utility::create_rsl_from_description (const saga::job::description & jd)
{
    using namespace saga::job;
    
    saga::attribute attr (jd);
    std::string rsl = "&"; // That's how an RSL string starts
    
    typedef std::map<const std::string,  std::string> translation_table_t;
    translation_table_t tt; 
    
    tt[attributes::description_executable] =              "executable";
    tt[attributes::description_working_directory] =       "directory";
    tt[attributes::description_input] =                   "stdin";
    tt[attributes::description_output] =                  "stdout";
    tt[attributes::description_error] =                   "stderr";
    
    tt[attributes::description_total_cpu_time] =          "maxCpuTime";
    tt[attributes::description_wall_time_limit] =         "maxWallTime";
    
    tt[attributes::description_total_physical_memory] =   "minMemory";
    tt[attributes::description_spmd_variation] =          "jobType";
    tt[attributes::description_number_of_processes] =     "count";
    tt[attributes::description_queue] =                   "queue";
    
    // RLS 2.4: (hostCount=value)
    // Only applies to clusters of SMP computers, such as newer IBM SP systems. 
    // Defines the number of nodes ("pizza boxes") to distribute the "count" 
    // processes across.
    if(attr.attribute_exists(attributes::description_number_of_processes) &&
       attr.attribute_exists(attributes::description_processes_per_host))
    {
        int nop = atoi(attr.get_attribute(attributes::description_number_of_processes).c_str());
        int pph = atoi(attr.get_attribute(attributes::description_processes_per_host).c_str());
        int hc = (int)nop/pph;
        char hc_c[50];
        ::itoa(hc, 10, hc_c);
        
        rsl.append("(hostCount=");
        rsl.append(" \""+std::string(hc_c)+"\")");
    }
    
    
    translation_table_t::const_iterator it;
    
    // Process all mappings from the translation table above
    for( it = tt.begin(); it != tt.end(); ++it )
    {
        if( attr.attribute_exists((*it).first) )
        {
            // only add if attribute is non-empty
            std::string val = attr.get_attribute((*it).first);
            if (!val.empty()) {
                rsl.append("(");
                rsl.append((*it).second);
                rsl.append("=");
                rsl.append(val);
                rsl.append(")");
            }
        }
    }
    
    // We only use the first entry of the job_project vector. RSL doesn't 
    // support multiple values.
    if (jd.attribute_exists(attributes::description_job_project))
    {
        std::vector<std::string> project_names = 
        jd.get_vector_attribute(attributes::description_job_project);
        
        if( project_names.size() > 0 ) 
        {
            rsl.append("(project=");
            rsl.append(" \""+project_names[0]+"\")");
        }
    }    
    
    // Process the (vector type) arguments
    if (jd.attribute_exists(attributes::description_arguments))
    {
        std::vector<std::string> arguments = 
        jd.get_vector_attribute(attributes::description_arguments);
        
        if( arguments.size() > 0 ) 
        {
            rsl.append("(arguments=");
            
            std::vector<std::string>::iterator end = arguments.end();
            for (std::vector<std::string>::iterator it = arguments.begin(); 
                 it != end; ++it)
            {
                rsl.append(" \""+(*it)+"\"");
            }
            
            rsl.append(")");
        }
    }
    
    // parse environment vector
    std::vector<std::string> env;
    if (jd.attribute_exists(saga::job::attributes::description_environment))
    {
        std::string env_str("");
        
        env = jd.get_vector_attribute(saga::job::attributes::description_environment);
        
        std::vector<std::string>::iterator end = env.end();
        for (std::vector<std::string>::iterator it = env.begin(); 
             it != end; ++it)
        {
            std::string key, value;
            if (!split_environment(*it, key, value)) 
            {
                globus_gram_job_adaptor::exception(
                                   "Bogus formatting of a environment entry: ':" + *it + "'",
                                   saga::BadParameter);
            }
            env_str.append("("+key+" "+value+")");
        }
        
        if(env.size() > 0) {
            rsl.append("(environment=");
            rsl.append(env_str);
            rsl.append(")");
        }
    }
    
    // Implement pre/post staging directive
    if (jd.attribute_exists(saga::job::attributes::description_file_transfer)) {
        // get the staging specifications
        
        std::vector<std::string> specs (
                                        jd.get_vector_attribute(saga::job::attributes::description_file_transfer));
        
        std::vector<std::string>::iterator end = specs.end();
        
        std::string file_stage_in("");
        std::string file_stage_out("");
        
        for (std::vector<std::string>::iterator it = specs.begin(); it != end; ++it)
        {
            using namespace saga::adaptors;
            std::string left_url, right_url;
            file_transfer_operator mode;
            if (!parse_file_transfer_specification(*it, left_url, mode, right_url))
            {
                throw globus_gram_job_adaptor::exception(
                                                         "job_cpi_impl::do_pre_staging: Ill-formatted file transfer specification.", 
                                                         saga::BadParameter);
            }
            
            if (copy_local_remote == mode) {
                file_stage_in.append("("+left_url+" "+right_url+")");                
            }
            else if (append_local_remote == mode) {
                throw globus_gram_job_adaptor::exception(
                                                         "job_cpi_impl::do_pre_staging: Appending stage-in files is not supported by GRAM.", 
                                                         saga::NotImplemented);
            }
            else if (copy_remote_local == mode) {
                file_stage_out.append("("+left_url+" "+right_url+")");                
            }
            else if (append_remote_local == mode) {
                throw globus_gram_job_adaptor::exception(
                                                         "job_cpi_impl::do_pre_staging: Appending stage-out files is not supported by GRAM.", 
                                                         saga::NotImplemented);
            }
            
        }
        
        if(file_stage_in.size() > 0) {
            rsl.append("(file_stage_in=");
            rsl.append(file_stage_in);
            rsl.append(")");
        }
        
        if(file_stage_out.size() > 0) {
            rsl.append("(file_stage_out=");
            rsl.append(file_stage_out);
            rsl.append(")");
        }
    }
    
    return rsl;
}

///////////////////////////////////////////////////////////////////////////////
//
std::string utility::translate_saga_to_gram_url (const std::string &saga_url)
{        
    saga::url url(saga_url);
    
    if ((url.get_host()).empty() == true) {
        // if we don't have a hostname, we can try localhost
        url.set_host("localhost");
    }
    
    if (!url.get_scheme().empty()  && url.get_scheme() != "gram" && 
        url.get_scheme() != "any") {
        SAGA_OSSTREAM strm;
        strm << "URL scheme ['"+url.get_scheme()+"://'" << "] is not supported. "; 
        SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm), 
                        saga::adaptors::AdaptorDeclined);
    }
    
    // If we've reached this point the given SAGA URL is suitable to be
    // translated to a GRAM URL. This is basically a 1:1 translation except
    // that the 'Scheme' part of the GRAM URL always has to be empty
    
    std::string gram_url = url.get_host();
    if (url.get_port() > 0) {
        gram_url.append(":");
        gram_url.append(boost::lexical_cast<std::string>(url.get_port()));
    }
    
    std::string p (url.get_path());
    if (p.size() > 1) { // only append if the path length is > 1 (meaning /x...)
        gram_url.append(p);
    }
    
    return gram_url;
}

