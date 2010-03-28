#include <saga/saga.hpp>
#include <fsclient.h>
#include <dcclient.h>
#include <util.h>
#include <iostream>
#include "secsp_err.hpp"

namespace saga_sectorsphere
{
     const char error::str_SUCCESS       []  = "SectorSphere: Success!" ; 
     const char error::str_UNKNOWN       []  = "SectorSphere: Unknown Error has occurred" ; 
     const char error::str_PERMISSION    []  = "SectorSphere: Permission Error" ; 
     const char error::str_EXIST         []  = "SectorSphere: Resource already exists" ; 
     const char error::str_NOEXIST       []  = "SectorSphere: Resource does not exist" ; 
     const char error::str_BUSY          []  = "SectorSphere: Resource is busy" ; 
     const char error::str_LOCALFILE     []  = "SectorSphere: Local File error" ; 
     const char error::str_SECURITY      []  = "SectorSphere: Security Error" ; 
     const char error::str_NOCERT        []  = "SectorSphere: No Certificate" ; 
     const char error::str_ACCOUNT       []  = "SectorSphere: User does not exist" ; 
     const char error::str_PASSWORD      []  = "SectorSphere: Incorrect password" ; 
     const char error::str_ACL           []  = "SectorSphere: Visit from Illegal IP Address" ; 
     const char error::str_INITCTX       []  = "SectorSphere: Failed to initialize SSL context" ; 
     const char error::str_CONNECTION    []  = "SectorSphere: Failed to establish connection" ; 
     const char error::str_RESOURCE      []  = "SectorSphere: No available resources" ; 
     const char error::str_TIMEDOUT      []  = "SectorSphere: Operation Timed out" ; 
     const char error::str_INVALID       []  = "SectorSphere: Invalid Parameter" ; 
     const char error::str_SUPPORT       []  = "SectorSphere: Operation not supported" ; 
     const char error::str_BAD_ARGS      []  = "SectorSphere: Bad Arguments" ; 
     const char error::str_NOT_SUPPORTED []  = "SectorSphere: Operation not supported" ; 
     const char error::str_NO_USERNAME   []  = "SectorSphere: No Username provided for authentication" ; 
     const char error::str_NO_PASSWORD   []  = "SectorSphere: No Password provided for authentication" ; 
     const char error::str_NO_SERVER     []  = "SectorSphere: No Server provided for authentication"   ; 
     const char error::str_NO_PORT       []  = "SectorSphere: No Server Port provided for authentication" ; 
     const char error::str_NO_SEC_CERT   []  = "SectorSphere: No Security Cert. provided for authentication" ; 
     const char error::str_NO_CONF_FILE  []  = "SectorSphere: No Client Config file available" ; 
     const char error::str_DIR_IS_FILE   []  = "SectorSphere: The specified directory name is a regular file." ; 

     /* General Errors
     */
     const int error::SAGA_SECSP_E_SUCCESS       = 0     ; 
     const int error::SAGA_SECSP_E_BAD_ARGS      = -3432 ; 
     const int error::SAGA_SECSP_E_NOT_SUPPORTED = -2354 ; 
     const int error::SAGA_SECSP_E_NO_CONF_FILE  = -899  ; 

     /* Authentication Errors
     */
     const int error::SAGA_SECSP_E_NO_USERNAME      = -3245 ;
     const int error::SAGA_SECSP_E_NO_PASSWORD      = -9023 ; 
     const int error::SAGA_SECSP_E_NO_SERVER        = -8764 ; 
     const int error::SAGA_SECSP_E_NO_PORT          = -8421 ; 
     const int error::SAGA_SECSP_E_NO_SECURITY_CERT = -7723 ; 
     const int error::SAGA_SECSP_E_DIR_IS_FILE      = -2132 ; 


     /* Sphere Errors 
      */
     const int error::SAGA_SECSP_E_NO_JOB_OPERATOR  = -4324 ; 
     const int error::SAGA_SECSP_E_NO_SPHERE_OUTPUT = -4394 ; 
     const int error::SAGA_SECSP_E_NO_SPHERE_INPUT  = -4304 ; 
     const int error::SAGA_SECSP_E_INVALID_OUTPUT_PATH  = -7623 ; 
     const int error::SAGA_SECSP_E_JOB_NOT_STARTED = -342 ; 
 


     void error::get_err_msg( std::string &err, int const &code ) 
     {

         switch( code ) 
         {

           case SAGA_SECSP_E_SUCCESS :
             break ; 

           case SAGA_SECSP_E_BAD_ARGS:
             err += error::str_BAD_ARGS ; 
             break ; 

           case SAGA_SECSP_E_NOT_SUPPORTED:
             err += error::str_NOT_SUPPORTED ; 
             break ; 

           case SAGA_SECSP_E_NO_USERNAME: 
	     err += error::str_NO_USERNAME ; 
	     break ; 

           case SAGA_SECSP_E_NO_PASSWORD:
	     err += error::str_NO_PASSWORD ; 
	     break ; 
	   
	   case SAGA_SECSP_E_NO_SERVER:
	     err += error::str_NO_SERVER; 
	     break ; 

	   case SAGA_SECSP_E_NO_PORT:
	     err += error::str_NO_PORT ; 
	     break ; 

	   case SAGA_SECSP_E_NO_SECURITY_CERT:
	     err += error::str_NO_SEC_CERT ; 
	     break ;

           case SAGA_SECSP_E_DIR_IS_FILE:
              err += error::str_DIR_IS_FILE ;  
	      break ; 

           default:
              err += SectorError::getErrorMsg( code ) ; 
         }

         return  ; 
     }
}

