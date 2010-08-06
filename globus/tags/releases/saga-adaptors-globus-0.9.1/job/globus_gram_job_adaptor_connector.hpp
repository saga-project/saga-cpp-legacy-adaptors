//  Copyright (c) 2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_CONNECTOR_HPP
#define ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_CONNECTOR_HPP

extern "C" {
#include <gssapi.h>
#include <globus_gram_protocol.h>
#include <globus_gram_client.h>
#include <globus_io.h>
}

#include "globus_gram_job_adaptor_errorhandler.hpp"

#include <saga/saga.hpp>
#include <saga/saga/packages/job/job_description.hpp>

namespace globus_gram_job_adaptor {
    
    //// Custom exception class //////////////////////////////////////////////
	//
	class exception : public std::exception {
        
	private:
        
		int			GlobusErrorCode_;
		std::string GlobusErrorString_;
		saga::error SAGAError_;
		
    public:
        
        exception (std::string error_text, saga::error saga_error)
        : GlobusErrorString_(error_text), SAGAError_(saga_error)
        {
            
        }
        
		exception (globus_result_t & Result) 
		{
            // extract RLS error text and error code
            std::string buf;
            buf = globus_gram_protocol_error_string(Result);
            SAGAError_ = saga::NoSuccess;
            
            GlobusErrorString_.append(buf);
                        
            switch((int)Result) 
            {
              case(GLOBUS_GRAM_PROTOCOL_ERROR_CONNECTION_FAILED):
              case(GLOBUS_GRAM_PROTOCOL_ERROR_CONTACTING_JOB_MANAGER):
              case(GLOBUS_GRAM_PROTOCOL_ERROR_JOB_CONTACT_NOT_FOUND):
                  SAGAError_ = saga::DoesNotExist;
                  break;
                  
              case(GLOBUS_GRAM_PROTOCOL_ERROR_AUTHORIZATION): 
              case(GLOBUS_GRAM_PROTOCOL_ERROR_AUTHORIZATION_DENIED): 
              case(GLOBUS_GRAM_PROTOCOL_ERROR_AUTHORIZATION_SYSTEM_FAILURE): 
              case(GLOBUS_GRAM_PROTOCOL_ERROR_AUTHORIZATION_DENIED_JOB_ID): 
              case(GLOBUS_GRAM_PROTOCOL_ERROR_AUTHORIZATION_DENIED_EXECUTABLE): 
              case(GLOBUS_GRAM_PROTOCOL_ERROR_UNDEFINED_EXE):
                  SAGAError_ = saga::AuthorizationFailed;
                  break;
                  
              default:							
                  SAGAError_ = saga::NoSuccess;
                  break;
            }
        }
        
        ~exception (void) throw () { }
        
        saga::error SAGAError() const throw ()
        {
            return SAGAError_;
        }
        
        const char * GlobusErrorText() const throw () 
        { 
            return GlobusErrorString_.c_str();
        }
        
        int GlobusErrorCode() const throw () 
        { 
            return GlobusErrorCode_;
        }
        
    };
    //
    //////////////////////////////////////////////////////////////////////////
    
    
    /**
     *  Structure encapsulating Globus mutex locks used for passing them 
     *  along with other parameters as one argument (user_arg) to the 
     *  callback functions. 
     */
    typedef struct globus_i_globusrun_gram_monitor_s
    {
        globus_bool_t  done;
        globus_mutex_t mutex;
        globus_cond_t  cond;
        
        globus_bool_t  verbose;
        unsigned long  job_state;
        int            failure_code;
        char *         job_contact;
    } globus_i_globusrun_gram_monitor_t;
    
    /**
     *  A static 'toolbox' class containing convenience methods - mostly for 
     *  translating GRAM codes into SAGA codes and vice versa.
     */
    class utility {
        
    public:
        
        /**
         *  Translate a Globus SAGA URL to a Globus GRAM URL
         *
         *  @param gram_url
         *      A string reference to store the resulting GRAM URL
         *
         *  @param saga_url
         *      A string containing the SAGA URL to translate
         *
         *  @return
         *      A tuple containing a saga:error and an error string or
         *      {(saga::error)saga::adaptors::Success, ""} in case the call succeeded
         */          
        static std::string 
        translate_saga_to_gram_url (const std::string &saga_url);
        
        /**
         *  Translate a Globus GRAM job state to a SAGA job state.
         *
         *  @param saga_state
         *      The resulting SAGA job state
         *
         *  @param gram_state
         *      An integer representing the Globus GRAM job state to translate
         *
         *  @return
         *      A tuple containing a saga:error and an error string or
         *      {(saga::error)saga::adaptors::Success, ""} in case the call succeeded
         */  
        static saga::job::state 
        translate_gram_to_saga_job_state (const saga::job::state& old_state,
                                          int gram_state);
        
        static std::vector <std::string> split_commandline (std::string const & input);
        
        /**
         *  Translate a SAGA job description to a Globus RSL string
         *
         *  @param rsl
         *      A string reference to store the resulting Globus RSL
         *
         *  @param jd
         *      The SAGA job description object to translate
         *
         *  @return
         *      A tuple containing a saga:error and an error string or
         *      {(saga::error)saga::adaptors::Success, ""} in case the call succeeded
         */  
        static std::string 
        create_rsl_from_description (const saga::job::description & jd);
    };
    
    /**
     *  A class grouping all Globus GRAM operations. This class has only static 
     *  member functions, since GRAM doesn't support persistent connections
     *  like GridFTP.
     */
    class connector {
        
    private:
        
        /**
         *  Callback function used by various Globus GRAM API calls. This
         *  method is the blocking version. See globus_ftp_client API 
         *  documentation at: http://globus.org/api/c-globus-4.0/
         */
        static void
        globus_l_globusrun_gram_callback_func ( void *user_arg,
                                               char *job_contact,
                                               int state,
                                               int errorcode);
        
        
        /**
         *  This method initializes Globus GRAM stuff: loading the GRAM
         *  modules and getting the X509 credentials.
         *
         *  @param credential
         *      The credential handle to initialize
         *
         *  @return
         *      A tuple containing a saga:error and an error string or
         *      {(saga::error)saga::adaptors::Success, ""} in case the call succeeded
         */
        static void init_connector (gss_cred_id_t &credential);
        
        /**
         *  This method cleans up Globus GRAM stuff: unloading the GRAM
         *  modules and releasing the X509 credentials. 
         *
         *  @param credential
         *      The credential handle to un-initialize
         *
         *  @return
         *      A tuple containing a saga:error and an error string or
         *      {(saga::error)saga::adaptors::Success, ""} in case the call succeeded
         */
        static void release_connector (gss_cred_id_t &credential);
        
    public:
        
        static void client_modules_activate();
        
        static void client_modules_deactivate();
        
        /**
         *  <DESCRIPTION>
         *
         *  @param jm_contact
         *      <DESCRIPTION>
         *
         *  @return
         *      A tuple containing a saga:error and an error string or
         *      {(saga::error)saga::adaptors::Success, ""} in case the call succeeded
         */
        static void ping_jobmanager (const std::string& jm_contact);
        
        /**
         *  DESCRIPTION
         *
         *  @param ret_jobid
         *      DESCRIPTION
         *  @param jm_contact
         *      DESCRIPTION
         *  @param jd
         *      DESCRIPTION
         *
         *  @return
         *      A tuple containing a saga:error and an error string or
         *      {(saga::error)saga::adaptors::Success, ""} in case the call succeeded
         */        
        static saga_error_tuple submit_job (std::string &ret_jobid,
                                            saga::job::state &ret_state,
                                            const std::string &jm_contact,
                                            const saga::job::description &jd);
        
        ///////////////////////////////////////////////////////////////////////
        
        
        static saga_error_tuple submit_job2(std::string &ret_jobid,
                                            saga::job::state &ret_state,
                                            const std::string &jm_contact,
                                            const saga::job::description &jd);
        
        
        
        ///////////////////////////////////////////////////////////////////////
        
        /**
         *  DESCRIPTION
         *
         *  @param ret_state
         *      DESCRIPTION
         *  @param job_contact
         *      DESCRIPTION
         *
         *  @return
         *      A tuple containing a saga:error and an error string or
         *      {(saga::error)saga::adaptors::Success, ""} in case the call succeeded
         */
        static saga::job::state get_job_state (saga::job::state  old_state,
                                               const std::string &job_contact);
        
        /**
         *  DESCRIPTION
         *
         *  @param job_contact
         *      DESCRIPTION
         *
         *  @return
         *      A tuple containing a saga:error and an error string or
         *      {(saga::error)saga::adaptors::Success, ""} in case the call succeeded
         */
        static void cancel_job (const std::string &job_contact);
        
        /**
         *  DESCRIPTION
         *
         *  @param signal
         *      DESCRIPTION
         *  @param signal_args
         *      DESCRIPTION
         *  @param job_contact
         *      DESCRIPTION
         *
         *  @return
         *      A tuple containing a saga:error and an error string or
         *      {(saga::error)saga::adaptors::Success, ""} in case the call succeeded
         */
        static void signal_job (globus_gram_protocol_job_signal_t signal,
                                            const std::string &signal_args,
                                            const std::string &job_contact);
    };
}

#endif //ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_CONNECTOR_HPP

