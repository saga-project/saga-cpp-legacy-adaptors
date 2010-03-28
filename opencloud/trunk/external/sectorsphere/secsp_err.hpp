#ifndef __SAGASECSP_ERROR__
#define __SAGASECSP_ERROR__


#include <fsclient.h>
#include <dcclient.h>

namespace saga_sectorsphere
{

   class error
   {
      
      public: 

         static const char str_SUCCESS       []  ; 
         static const char str_UNKNOWN       []  ; 
         static const char str_PERMISSION    []  ; 
         static const char str_EXIST         []  ; 
         static const char str_NOEXIST       []  ; 
         static const char str_BUSY          []  ; 
         static const char str_LOCALFILE     []  ; 
         static const char str_SECURITY      []  ; 
         static const char str_NOCERT        []  ; 
         static const char str_ACCOUNT       []  ; 
         static const char str_PASSWORD      []  ; 
         static const char str_ACL           []  ; 
         static const char str_INITCTX       []  ; 
         static const char str_CONNECTION    []  ; 
         static const char str_RESOURCE      []  ; 
         static const char str_TIMEDOUT      []  ; 
         static const char str_INVALID       []  ; 
         static const char str_SUPPORT       []  ; 
         static const char str_BAD_ARGS      []  ; 
         static const char str_NOT_SUPPORTED []  ; 
         static const char str_NO_USERNAME   []  ; 
         static const char str_NO_PASSWORD   []  ; 
         static const char str_NO_SERVER     []  ; 
         static const char str_NO_PORT       []  ; 
         static const char str_NO_SEC_CERT   []  ; 
         static const char str_NO_CONF_FILE  []  ; 
         static const char str_DIR_IS_FILE   []  ;

         
         static const int SAGA_SECSP_E_BAD_ARGS      ; 
         static const int SAGA_SECSP_E_SUCCESS       ; 
         static const int SAGA_SECSP_E_NOT_SUPPORTED ; 
         static const int SAGA_SECSP_E_DIR_IS_FILE   ; 

         static const int SAGA_SECSP_E_NO_USERNAME ; 
         static const int SAGA_SECSP_E_NO_PASSWORD ; 
         static const int SAGA_SECSP_E_NO_SERVER ; 
         static const int SAGA_SECSP_E_NO_PORT ; 
         static const int SAGA_SECSP_E_NO_SECURITY_CERT ; 
         static const int SAGA_SECSP_E_NO_CONF_FILE ; 


         static const int SAGA_SECSP_E_NO_JOB_OPERATOR  ; 
         static const int SAGA_SECSP_E_NO_SPHERE_OUTPUT ; 
         static const int SAGA_SECSP_E_NO_SPHERE_INPUT  ; 
         static const int SAGA_SECSP_E_INVALID_OUTPUT_PATH ;
         static const int SAGA_SECSP_E_JOB_NOT_STARTED  ; 


         static void get_err_msg( std::string &err, int const &code ) ; 
   };
    
}
#endif

