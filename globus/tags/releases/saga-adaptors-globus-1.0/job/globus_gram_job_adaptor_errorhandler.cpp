//  Copyright (c) 2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <gssapi.h>
#include <globus_gram_client.h>

#include "globus_gram_protocol.h"
#include "globus_gram_job_adaptor_errorhandler.hpp"

#include <iostream>
#include <sstream>

using namespace globus_gram_job_adaptor;

///////////////////////////////////////////////////////////////////////////////
// CONSTRUCTOR
//
errorhandler::errorhandler()
{
    _init_generic_LUT(); 
}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC: get_saga_exception_for
//
saga_error_tuple errorhandler::
get_saga_exception_for (std::string method_name, int gram_error) 
{
    saga_error_tuple et = {(saga::error)saga::NoSuccess, ""}; 

    anonymous::_error_dictionary_t::iterator dict_pos;
    anonymous::_error_map_t::iterator map_pos;
    
    dict_pos = _generic_LUT.find (gram_error);
    map_pos  = _context_error_map.find (method_name);
    
    // First, we try to find an error mapping in the generic lookup table
    if( dict_pos != _generic_LUT.end() ) {
        // We found a generic error mapping -> create exception object
        et = _generic_LUT[gram_error];
    }
    
    // Second, we look for a more specific error mapping in 'method_name' LUT 
    else if( map_pos != _context_error_map.end() ) {
        // We found an error map for 'method_name'
        dict_pos = ((*map_pos).second).find (gram_error);
        if( dict_pos != ((*map_pos).second).end() ) {
            // We found a specific error mapping -> create exception object
            et = ((*map_pos).second) [gram_error];
        }
    }
    
    // Problem: There's no generic or specific mapping for 'gram_error' that's
    // not good! Construct a generic exception. Encourage the user to send a 
    // bug report to us :) 
    else {
        et.error = (saga::error)saga::NoSuccess;
        et.desc_informal = "There's no mapping for this error.";
    }
    
    // this should always work since we're only dealing with GRAM errors...
    if(gram_error == 999999999)
    {
        et.desc_informal = "[Globus GRAM] Job submitted but in unknown state. Usually this happens if the GRAM backend (PBS/LSF/...) bails out. Check the log files on the GRAM host.";
    }
    else
    {
    	std::stringstream oss;
    	oss << "[Globus GRAM] " <<  globus_gram_protocol_error_string(gram_error)  
        	<< " (" << gram_error << ")";    
    	et.desc_informal = oss.str();
    }
    
    return et; 
}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE: _init_context_error_map
//
void errorhandler::_init_context_error_map()
{
    // Register all specific lookup tables here
    _context_error_map["sync_create_job"] = _sync_create_job_LUT;
}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE: _init_generic_LUT
//
void errorhandler::_init_generic_LUT()
{
    //saga_error_tuple et = {(saga::error)saga::adaptors::Unexpected, ""}; 
    
    // LOWLEVEL/UNHANDLED SHIT...
    saga_error_tuple et = {saga::BadParameter, ""};
    
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_VERSION_MISMATCH] = et;
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_DRYRUN] = et;  
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_PROTOCOL_FAILED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_HTTP_FRAME_FAILED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_HTTP_UNFRAME_FAILED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_HTTP_PACK_FAILED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_HTTP_UNPACK_FAILED] = et; 
    
    // Fehler die auftreten, BEVOR der JOB auf dem Gatekeeper landet...
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_SERVICE_NOT_FOUND] = et; 
    
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_UNIMPLEMENTED] = et;
    
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_BAD_GATEKEEPER_CONTACT] = et;
   
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_CONNECTION_FAILED] = et; 
    
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_GATEKEEPER_MISCONFIGURED] = et; 
    
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_CONTACTING_JOB_MANAGER] = et; 
    
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_JOB_CONTACT_NOT_FOUND] = et; 

    
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_PARAMETER_NOT_SUPPORTED] = et;  
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_REQUEST] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_NO_RESOURCES] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_BAD_DIRECTORY] = et; 
    
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_EXECUTABLE_NOT_FOUND] = et; 
    
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_STDIN_NOT_FOUND] = et;
    
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INSUFFICIENT_FUNDS] = et; 
    
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_USER_CANCELLED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_SYSTEM_CANCELLED] = et; 
    
   

    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_NULL_SPECIFICATION_TREE] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_JM_FAILED_ALLOW_ATTACH] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_JOB_EXECUTION_FAILED] = et; 
 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_BAD_SCRIPT_ARG_FILE] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_ARG_FILE_CREATION_FAILED] = et; 

    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_JOBTYPE_NOT_SUPPORTED] = et; 
 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_TEMP_SCRIPT_FILE_FAILED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_USER_PROXY_NOT_FOUND] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_OPENING_USER_PROXY] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_JOB_CANCEL_FAILED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_MALLOC_FAILED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_DUCT_INIT_FAILED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_DUCT_LSP_FAILED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_UNSUPPORTED_PARAMETER] = et; 

    

    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_STAGING_EXECUTABLE] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_STAGING_STDIN] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_BAD_ARGUMENTS] = et; 
    
    
     
    ///////////////////////////////////////////////////////////////////////////
    // CATEGORY: saga::AuthorizationFailed
    et.error = saga::NoSuccess;
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_OPENING_JOBMANAGER_SCRIPT] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_CREATING_PIPE] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_FCNTL_FAILED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_STDOUT_FILENAME_FAILED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_STDERR_FILENAME_FAILED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_FORKING_EXECUTABLE] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_EXECUTABLE_PERMISSIONS] = et; 
    
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_OPENING_STDOUT] = et; 

    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_OPENING_STDERR] = et; 
    
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_OPENING_CACHE_USER_PROXY] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_OPENING_CACHE] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INSERTING_CLIENT_CONTACT] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_CLIENT_CONTACT_NOT_FOUND] = et; 
    
    
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_CONDOR_ARCH] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_CONDOR_OS] = et; 



    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_JOB_QUERY_DENIAL] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_CALLBACK_NOT_FOUND] = et; 

    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_POE_NOT_FOUND] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_MPIRUN_NOT_FOUND] = et; 

    
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_JM_SCRIPT_NOT_FOUND] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_JM_SCRIPT_PERMISSIONS] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_SIGNALING_JOB] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_UNKNOWN_SIGNAL_TYPE] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_GETTING_JOBID] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_WAITING_FOR_COMMIT] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_COMMIT_TIMED_OUT] = et; 

    

    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RESTART_FAILED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_NO_STATE_FILE] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_READING_STATE_FILE] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_WRITING_STATE_FILE] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_OLD_JM_ALIVE] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_TTL_EXPIRED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_SUBMIT_UNKNOWN] = et; 
    
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_WRITING_REMOTE_IO_URL] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_STDIO_SIZE] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_JM_STOPPED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_USER_PROXY_EXPIRED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_JOB_UNSUBMITTED] = et; 
    
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_STAGE_IN_FAILED] = et; 
    

    
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_OPENING_VALIDATION_FILE] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_READING_VALIDATION_FILE] = et; 
    
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_STAGE_OUT_FAILED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_DELEGATION_FAILED] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_LOCKING_STATE_LOCK_FILE] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_NULL_PARAMETER] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_STILL_STREAMING] = et; 
    
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_LAST] = et; 
    
    ///////////////////////////////////////////////////////////////////////////
    // CATEGORY: saga::AuthorizationFailed
    et.error = saga::AuthorizationFailed;
    et.desc_informal += "(did you forget grid-proxy-init)"; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_AUTHORIZATION] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_AUTHORIZATION_DENIED] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_AUTHORIZATION_SYSTEM_FAILURE] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_AUTHORIZATION_DENIED_JOB_ID] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_AUTHORIZATION_DENIED_EXECUTABLE] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_UNDEFINED_EXE] = et; 
    
    ///////////////////////////////////////////////////////////////////////////
    // EXCEPTIONAL STUFF
    et.error = saga::NoSuccess;
    _generic_LUT[999999999] = et;
    
    ///////////////////////////////////////////////////////////////////////////
    // RSL RELATED STUFF
    // 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_ZERO_LENGTH_RSL] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_BAD_RSL_ENVIRONMENT] = et; 
    //_generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_EVALUATION_FAILED] = et;
    
    // All RSL attribute errors
    et.error = saga::BadParameter;
    
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_BAD_RSL] = et;
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_ARGUMENTS] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_CACHE] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_COUNT] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_DIRECTORY] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_DRYRUN] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_ENVIRONMENT] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_EXECUTABLE] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_FILE_CLEANUP] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_FILE_STAGE_IN] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_FILE_STAGE_IN_SHARED] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_FILE_STAGE_OUT] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_GASS_CACHE] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_HOST_COUNT] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_JOBTYPE] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_MAX_CPU_TIME] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_MAX_MEMORY] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_MAX_WALL_TIME] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_MAXTIME] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_MIN_MEMORY] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_MYJOB] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_PARADYN] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_PROJECT] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_PROXY_TIMEOUT] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_QUEUE] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_REMOTE_IO_URL] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_RESERVATION_HANDLE] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_RESTART] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_SAVE_STATE] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_SCHEDULER_SPECIFIC] = et;
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_SCRATCH] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_START_TIME] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_STDERR] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_STDERR_POSITION] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_STDIN] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_STDOUT] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_STDOUT_POSITION] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_TWO_PHASE_COMMIT] = et; 
#ifdef GLOBUS_GRAM_PROTOCOL_ERROR_RSL_USER_NAME
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_RSL_USER_NAME] = et; 
#endif
    
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_JOB_CONTACT] = et; //!!!!
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_UNDEFINED_ATTRIBUTE] = et;  //!??!
        
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_ATTR] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_CACHE] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_COMMIT] = et; //!!!!
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_COUNT] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_GRAM_MYJOB] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_HOST_COUNT] = et; 

    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_JOB_MANAGER_TYPE] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_JOB_QUERY] = et; //!!!!!
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_JOBSTATE] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_JOBTYPE] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_MAX_CPU_TIME] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_MAX_MEMORY] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_MAX_WALL_TIME] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_MAXTIME] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_MIN_MEMORY] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_PARADYN] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_PROJECT] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_PROXY_TIMEOUT] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_QUEUE] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_RESTART_ATTRIBUTE] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_SAVE_STATE] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_SCHEDULER_SPECIFIC] = et;
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_SCRATCH] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_SCRIPT_REPLY] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_SCRIPT_STATUS] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_STDERR_POSITION] = et; //!!!
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_STDIO_UPDATE_ATTRIBUTE] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_STDOUT_POSITION] = et; //!!!
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_SUBMIT_ATTRIBUTE] = et; 
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_TWO_PHASE_COMMIT] = et; //!!!!!
    _generic_LUT[GLOBUS_GRAM_PROTOCOL_ERROR_INVALID_USER_NAME] = et; 

}

void errorhandler::_init_sync_create_job_LUT()
{
    _sync_create_job_LUT.clear();
}
