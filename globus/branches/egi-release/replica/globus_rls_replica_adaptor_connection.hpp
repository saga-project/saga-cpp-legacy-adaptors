//  Copyright (c) 2007-2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_GLOBUS_RLS_REPLICA_CONNECTION_HPP
#define ADAPTORS_GLOBUS_RLS_REPLICA_CONNECTION_HPP

#include <string>
#include <saga/saga/exception.hpp>
#include <saga/saga/adaptors/utils/ini/ini.hpp>
#include <saga/saga/error.hpp>

extern "C" {
	#include <globus_rls_client.h>
}

#define MAX_ATTR_LENGTH 2048 

#define THROW_IF_INVALID(LFNName)										\
try {                                                                   \
    RLSConnection * RLSHandle =											\
        adaptorData->getConnectionHandle(instanceData->location_);		\
    RLSHandle->LFNExistsThrow(lfn_url.get_path());						\
}																		\
catch(globus_rls_replica_adaptor::exception const & e)					\
{																		\
    SAGA_OSSTREAM strm;													\
    strm << "Could not open logical file [" << instanceData->location_ <<	\
    "]. " << e.RLSErrorText();											\
    SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError());	\
}																		\

namespace globus_rls_replica_adaptor
{
	//// Helper functions ////////////////////////////////////////////////////
	//
	namespace helper 
	{
	
	inline void parse_find_pattern(saga::impl::v1_0::cpi const* target, 
        std::string const& pattern, std::string& kpat, std::string& vpat)
    {
        std::string::size_type p = pattern.find_first_of("=");
        if (std::string::npos == p) {
			std::cerr << "PARSE ERROR: Invalid pattern!" << std::endl;
			// FIXME: make this work
            //SAGA_ADAPTOR_THROW_VERBATIM(target, 
            ///    "Pattern has invalid format: " + pattern, saga::BadParameter);
        }
        kpat = pattern.substr(0, p);
        vpat = pattern.substr(p+1);
		
		// validate kpat: RLS doesn't support wildcards here!
		std::string::size_type p_q = kpat.find_first_of("?");
		std::string::size_type p_a = kpat.find_first_of("*");
		if ((std::string::npos != p_q) || (std::string::npos != p_a)) {
			std::cerr << "PARSE ERROR: Invalid key pattern!" << std::endl;
			// FIXME: make this work
            //SAGA_ADAPTOR_THROW_VERBATIM(target, 
            ///    "Pattern has invalid format: " + pattern, saga::BadParameter);
        }
    }
	
	inline int globus_result_t_to_rls_ec(globus_result_t &result)
	{
		int rc; 
		result = globus_rls_client_error_info(result, &rc, NULL, 0, true);
		return rc;
	}	
	
	inline saga::error globus_io_to_saga_error(globus_result_t &result, 
											   std::string &messageS,
											   std::string &messageL)
	{
		saga::error se = saga::NoSuccess;
		SAGA_OSSTREAM msgSstrm;
		int rc;
		result = globus_rls_client_error_info(result, &rc, NULL, 0, true);
		if(rc == GLOBUS_RLS_GLOBUSERR)
		{
			globus_object_t * err = globus_error_get(result);
			messageL = globus_error_print_chain(err);
			if(!messageL.empty()) {
			
				if(messageL.find("Valid credentials could not be found") 
				!= std::string::npos) {
					se = saga::AuthorizationFailed;
					msgSstrm << "Valid Globus X509 credentials COULD NOT BE "
						<< " FOUND in any of the possible locations specified"
						<< " by the credential search order! ";
				}
				
				else if(messageL.find("expired") != std::string::npos) {
					se = saga::AuthorizationFailed;
					msgSstrm << "The Globus X509 credentials are EXPIRED! ";
				}
				
				else if(messageL.find("Unable to verify remote side's")
					!= std::string::npos) {
					se = saga::AuthorizationFailed;
					msgSstrm << "BAD (or wrong) Globus X509 credentials! ";
				}
                
                else if(messageL.find("Connection refused") 
                    != std::string::npos) {
                    se = saga::NoSuccess;
                    msgSstrm << "Connection refused! ";
                }
			}
		}
		msgSstrm << "(Set SAGA_VERBOSE >= 5 to get detailed Globus errors)";
		messageS.append(SAGA_OSSTREAM_GETSTRING(msgSstrm));
		return se;
	}
	}
	//
	//////////////////////////////////////////////////////////////////////////
	
	//// Custom exception class //////////////////////////////////////////////
	//
	class exception : public std::exception {
	
	private:
    
		int			RLSErrorCode_;
		std::string RLSErrorString_;
		saga::error SAGAError_;
		
    public:
    
		exception (globus_result_t & RLSResult) 
		{
            // extract RLS error text and error code
            char buf[MAXERRMSG];
            RLSResult = globus_rls_client_error_info(RLSResult, &RLSErrorCode_, 
                buf, MAXERRMSG, true);
            RLSErrorString_.append(buf);
        
            // we need some extra stuff to handle Globus errors that do not
            // come from RLS directly, e.g. from XIO or GSS. This is used
            // only in the GLOBUS_RLS_GLOBUSERR switch case. 
            std::string nonRLSMessageS(""); // short message
            std::string nonRLSMessageL(""); // long "original" message
            switch(RLSErrorCode_) 
            {
            
            // nasty globus IO, GSS, etc. errors require special treatment ///
            case GLOBUS_RLS_GLOBUSERR : 
                SAGAError_ = helper::globus_io_to_saga_error(RLSResult, 
                                                             nonRLSMessageS,
                                                             nonRLSMessageL);
                RLSErrorString_ = nonRLSMessageS;
                SAGA_VERBOSE (SAGA_VERBOSE_LEVEL_DEBUG)
                {
                    RLSErrorString_ = nonRLSMessageL;
                }
                break;
            
            // RLS errors that can be associated with a timeout //////////////
            case GLOBUS_RLS_TIMEOUT : 
            case GLOBUS_RLS_TOO_MANY_CONNECTIONS :
                SAGAError_ = saga::Timeout;
                break;
            
            // RLS errors that are really unexpected and *shoudln't* occur ///
            case GLOBUS_RLS_INVHANDLE :				// (invalid RLS handle)
            case GLOBUS_RLS_NOMEMORY :				
            case GLOBUS_RLS_OVERFLOW :  
            case GLOBUS_RLS_DBERROR :				// (MySQL/PostgreSQL error)
            case GLOBUS_RLS_UNSUPPORTED :   
            case GLOBUS_RLS_INV_OBJ_TYPE : 
            case GLOBUS_RLS_BADMETHOD :				// (RPC error)
                SAGAError_ = saga::NoSuccess;
                break;
                
            case GLOBUS_RLS_BADURL:					// invalid URL
                SAGAError_ = saga::IncorrectURL;
                break;
                
            case GLOBUS_RLS_LFN_NEXIST:				// LFN doesn't exist
            case GLOBUS_RLS_ATTR_NEXIST:			// Attribute doesn't exist
            case GLOBUS_RLS_MAPPING_NEXIST:			// LFN -> PFN mapping 
            case GLOBUS_RLS_PFN_NEXIST:				// PFN

                SAGAError_ = saga::DoesNotExist;
                break;
            
            case GLOBUS_RLS_LFN_EXIST:				// LFN exists
            case GLOBUS_RLS_ATTR_EXIST:				// Attribute exists
            case GLOBUS_RLS_MAPPING_EXIST:			// LFN -> PFN mapping 
                SAGAError_ = saga::AlreadyExists;
                break;
            
            default:								// hmm - something went really wrong
                SAGAError_ = saga::NoSuccess;
                break;
            }
		}

		~exception (void) throw () { }
    
		saga::error SAGAError() const throw ()
		{
			return SAGAError_;
		}
	
		const char * RLSErrorText() const throw () 
		{ 
			return RLSErrorString_.c_str();
		}
    
		int RLSErrorCode() const throw () 
		{ 
			return RLSErrorCode_;
		}

	};
	//
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//
	class RLSConnection {
	
	private:

		globus_rls_handle_t  * RLSHandle;
        
        // used to preallocate vector space for the listLFN method. it will be 
        // set to a sufficient value after the method was called for the first
        // time.
        size_t lfnListSize;
        
	public:
	
		RLSConnection(std::string host_url);
		~RLSConnection();
	   
	    //// lfn inspection methods
		//
		bool LFNExists						 (std::string LFNName);
		
		bool LFNExistsThrow					 (std::string LFNName);
		
		std::vector<saga::url> LFNGetPFNList (std::string LFNName);
        
        std::vector<saga::url> LFNList       (std::string POSIXPattern);
		
		bool LFNtoPFNMappingExists           (std::string LFNName,
                                              std::string PFNName);
		
		//// modify lfn -> pfn mapping
		//
		void LFNAddPFN						 (std::string LFNName, 
                                              std::string PFNName);
		void LFNRemovePFN					 (std::string LFNName,
                                              std::string PFNName);
		
		//// attribute handling methods
		//
		std::vector<std::string> LFNAttributeList(std::string LFNName);
		
		bool        LFNAttributeExists      (std::string LFNName,
                                             std::string AttrName);
		std::string LFNAttributeGet         (std::string LFNName, 
                                             std::string AttrName);
		void        LFNAttributeRemove      (std::string LFNName, 
                                             std::string AttrName);
		void        LFNAttributeModify      (std::string LFNName, 
                                             std::string AttrName,
                                             std::string AttrValue);
		void        LFNAttributeCreate      (std::string LFNName, 
                                             std::string AttrName,
                                             std::string AttrValue);	
	private:
		
		int getPreferencesTimeout();
	};
	//
	//////////////////////////////////////////////////////////////////////////
	
}

#endif
