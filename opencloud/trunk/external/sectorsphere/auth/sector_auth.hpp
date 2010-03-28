#include <saga/saga.hpp>
#include <fsclient.h>
#include <dcclient.h>
#include <util.h>
#include <iostream>
#include <string>
#include <secsp_err.hpp>


namespace saga_sectorsphere
{
  class authenticator
  {
    private:

      std::string                            user_name ; 
      std::string                            password ; 
      std::string                            server_address ; 
      int                                    server_port ; 
      std::string                            sec_cert ; 
      std::string                            client_conf ; 
      bool                                   connected ; 
      int do_auth (void) ; 

    public:

      // Constructors
      authenticator (void);
      ~authenticator (void);
      authenticator( std::string client_conf ) ; 
      authenticator (std::string username, std::string passwd, std::string server_address, int port ) ; 

      void clear           (void);
      void set_user        (std::string user) ; 
      void set_passwd      (std::string passwd ) ; 
      void set_server      (std::string server ) ; 
      void set_port        (int port ) ; 
      void set_sec_cert    (std::string cert ) ; 
      void set_client_conf (std::string conf ) ; 
      void get_client_conf (std::string &conf ) ; 
      void logout          (void) ; 
      int  authenticate    (void) ; 
      void get_error       ( std::string & err , int const &code ) ; 

  };
}

