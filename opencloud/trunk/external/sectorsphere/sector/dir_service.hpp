#include <fsclient.h>
#include <dcclient.h>
#include <util.h>
#include <iostream>
#include <saga/saga/adaptors/utils/utils.hpp>
#include <vector>
#include <secsp_err.hpp>
#include "file_service.hpp"

namespace saga_sector
{
   
   class dir_service 
   {

      private:

         std::string   path ;
         int           mode ; 
         int           last_error ; 
         file_service  fserv_ ; 
         bool          op ; 

      public:

         /******************************Contructors********************************/
         dir_service (void) ; 
         ~dir_service(void) ; 


         /*************************Dir Operations***********************************/
         int open             ( std::string, int smode ) ; 
         void close           ( void ) ; 
         int  get_dir_size    ( saga::off_t &size ) ; 
         int copy             ( std::string dest , std::string &err ) ; 
         int move             ( std::string dest, std::string &err ) ; 
         int remove           ( std::string &err ) ; 
         int list             ( std::vector <saga::url> & list , std::string pattern="" ) ; 
         int get_num_entries  ( std::size_t &num ) ; 
         int mkdir            ( std::string & dir ) ; 

         /*************************Error Handling***********************************/
         void get_error_msg( std::string & err , int const &code ) ; 
  };

}

