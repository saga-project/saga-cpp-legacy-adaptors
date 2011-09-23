//  Copyright (c) 2009 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_GLOBUS_GRAM_JOB_UTILS_HPP
#define ADAPTORS_GLOBUS_GRAM_JOB_UTILS_HPP

#include "globus_common.h"
#include "globus_error.h"
#include "globus_gsi_cert_utils.h"
#include "globus_gsi_system_config.h"
#include "globus_gsi_proxy.h"
#include "globus_gsi_credential.h"
#include "globus_openssl.h"

namespace globus_gram_job_adaptor {

////////////////////////////////////////////////////////////////////////////////
//
inline std::string globus_get_detailed_error(globus_result_t result)
{
  globus_object_t * error_obj;
  error_obj = globus_error_get(result);
  
  char * error_string = NULL;
  error_string = globus_error_print_chain(error_obj);
  
  std::string error(error_string);
  
  if(error_string)
  {
    globus_libc_free(error_string);
    globus_object_free(error_obj);
  }
  
  return error;
}
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
inline void check_x509_globus_cert(saga::context & context,
                                 std::vector<saga::context> & context_list,
                                 std::vector<std::string> & context_error_list)
{
      std::string error;
      
      if (context.attribute_exists (saga::attributes::context_type) &&
          context.get_attribute (saga::attributes::context_type) == "x509")
      {
        if(!context.attribute_exists (saga::attributes::context_userproxy)) {
          error += "x.509 user proxy attribute not set.";
          context_error_list.push_back(error);
          return;
        }
        
        std::string userproxy(context.get_attribute (saga::attributes::context_userproxy));
        error += "x.509 user proxy @ " + userproxy + ": ";
        SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_DEBUG) {
          std::cerr << "globus GRAM Adaptor: X.509 context found pointing to user proxy at " 
                    << userproxy << std::endl;
        }
        
        // Using boost::filesystem is much faster then using the VOMSWrapper.
        // In case the file doesn't exist, we don't want to wast anymore time.
        if(!boost::filesystem::exists(userproxy)) {
            error += "No such file or directory";
            context_error_list.push_back(error);
            return;
        }
        else
        {
          // DO PROXY CHECKS USING THE GLOBUS GSI API
          globus_gsi_cred_handle_t proxy_cred = NULL;
          
          globus_result_t result = globus_gsi_cred_handle_init(&proxy_cred, NULL);
          if(result != GLOBUS_SUCCESS)
          {
            error += globus_get_detailed_error(result);
            context_error_list.push_back(error);
            return;
          }
      
          result = globus_gsi_cred_read_proxy(proxy_cred, userproxy.c_str());
          if(result != GLOBUS_SUCCESS)
          {
            error += globus_get_detailed_error(result);
            context_error_list.push_back(error);
            globus_gsi_cred_handle_destroy(proxy_cred);
            return;
          }
          
          // validity (in seconds)
          time_t lifetime;
          result = globus_gsi_cred_get_lifetime(proxy_cred, &lifetime);
          if(result != GLOBUS_SUCCESS)
          {
            error += globus_get_detailed_error(result);
            context_error_list.push_back(error);
            globus_gsi_cred_handle_destroy(proxy_cred);
            return;
          }
    
          if(lifetime <= 0) {
            error += "Certificate is expired. Please check.";
            context_error_list.push_back(error);
            globus_gsi_cred_handle_destroy(proxy_cred);
            return;
          }
          
          globus_gsi_cred_handle_destroy(proxy_cred);
          
          // SUCCESS
          SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_INFO) {
          std::cerr << "Globus GRAM Adaptor: Certificate seems to be present." 
                    << std::endl;
          }
          context_list.push_back (context);
         }
        } 
      } 

} // namespace

#endif //ADAPTORS_GLOBUS_GRAM_JOB_UTILS_HPP
