#include <fsclient.h>
#include <dcclient.h>
#include <util.h>
#include <iostream>
#include <saga/saga/adaptors/utils/utils.hpp>
#include <secsp_err.hpp>

namespace saga_sector
{
   
   class file_service
   {


      /* Utility names to perform file operations
       * through Sector
       */
      static const char  mv       [] ; 
      static const char  upload   [] ; 
      static const char  download [] ;
      static const char  mkdir    [] ;   
      static const char  cp       [] ;
      static const char  rm       [] ; 


      private:

         /* Friend class, the directory service, needs 
          * access to some functionality provided by the
          * private methods of file service - i.e. 
          * resolving wildcards in file names. 
          */
         friend class dir_service ; 

         std::string         path ;
         bool                op ; 
         int                 mode ; 
         int                 saga_mode ; 
         int                 whence ; 
         //int                 last_error ; 
         SectorFile          sfile ; 
         std::string         tools_path ; 

         /* Internal helper methods 
          */
         bool execute_command     ( std::string exec, std::vector <std::string>  args, std::string &err ) ; 
         int  get_wildcard_entries( std::vector <std::string> &filtered , std::string str , bool &wc ) ; 
         void set_file_name       ( std::string const & name ) ; 

      public:

         
         /******************************Contructors********************************/
         
         file_service( ) ; 
         ~file_service(void) ; 
         file_service( std::string const &filename ) ; 

         /*************************Configuration information************************/

         /* Tools path is the path to where the Sector
          * utilities are located.
          */
         void set_tools_path( std::string tools )  ; 


         /*************************File Operations***********************************/

         int  open         ( std::string, int smode ) ; 
         void close        (void) ; 
         int  read         ( char * buffer, saga::ssize_t len ) ; 
         int  write        ( const char * buffer, saga::ssize_t len ) ;  
         int  seek         ( int64_t offset, saga::filesystem::seek_mode w, int64_t &out ) ;
         int  get_file_size( saga::off_t &size ) ;
         int  is_dir       ( bool &is_dir ) ; 
         int  copy         ( std::string dest ) ; 
         int  move         ( std::string dest ) ; 
         int  remove       ( void ) ; 

         void get_error_msg( std::string & err, int const &code ) ; 

  };

}

