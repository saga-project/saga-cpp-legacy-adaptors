#include <saga/saga.hpp>
#include <fsclient.h>
#include <dcclient.h>
#include <util.h>
#include <iostream>
#include <auth/sector_auth.hpp>
#include <sector/dir_service.hpp>
#include <sphere/job_service.hpp>

namespace saga_sectorsphere
{


   class service
   {
      private:

        static const char  client_conf_name [] ; 
        static const char  client_tool_name [] ; 

        // For authentication
        authenticator auth  ; 

        // Sector file operations
        saga_sector::file_service fservice_ ; 

        // Sector dir operations
        saga_sector::dir_service dservice_ ; 

	//Sphere service
	saga_sphere::job_service jserv_ ; 

    public:

      service(void) ; 
      ~service(void) ; 
      
      inline saga_sectorsphere::authenticator& get_authenticator(void) 
      {
         authenticator &ref = auth ; 
         return ref ; 
      }

      inline saga_sector::file_service& get_sector_file_service(void)
      {
         saga_sector::file_service &f = fservice_ ; 
         return f ; 
      }

      inline saga_sector::dir_service& get_sector_dir_service(void)
      {
         saga_sector::dir_service &d = dservice_ ; 
         return d ; 
      }

      inline saga_sphere::job_service& get_sphere_service(void)
      {
         saga_sphere::job_service &j = jserv_ ; 
	 return j ; 
      }

      inline void set_sector_path( std::string path )
      {
         std::string conf = path + client_conf_name ; 
         auth.set_client_conf( conf ) ; 
         std::string tools = path + client_tool_name ; 
         fservice_.set_tools_path( tools ) ; 
      }

  };
}

