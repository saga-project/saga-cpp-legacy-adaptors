#include <fsclient.h>
#include <dcclient.h>
#include <util.h>
#include <iostream>
#include <vector>
#include <saga/saga/adaptors/utils/utils.hpp>
#include <secsp_err.hpp>

namespace saga_sphere
{
   
   class job_service
   {
      private:

         /* Path to DLL containing the function 
	  */
	 std::string         dll_path ;

	 /* The name of the UDF function itself 
	  */
         std::string         func ; 

         /* List of input files 
	  */
	 std::vector <std::string> input_fileset ; 

         /* Output files path 
	  */
	 std::string output_path ; 

	 /* Input Stream 
	  */
         SphereStream        input ;  

	 /* Output Stream 
	  */
         SphereStream        output ; 

         /* The Sphere process executing the DLL 
	  */
         SphereProcess       proc ;  

	 /* Arguments to UDF 
    	  */
	 void * arg ; 
         int    size ; 
	 int    rows ; 

         bool have_output ; 

	 saga::job::state st_ ; 
	 
      public:
         
         /******************************Contructors********************************/
         
         job_service (void) ; 
         ~job_service(void) ; 
         job_service( std::string const &path, std::string const &func , std::vector<std::string> file_list , 
                      std::string const &output_path ) ; 

         /******************************Job Operations*****************************/
	 int run_job( void ) ; 
	 int cancel_job( void ) ; 
	 int wait      ( const double &timeout ) ; 

	 int get_job_state    ( saga::job::state &st ) ; 

         void set_dll_path    ( std::string const &dll ) ; 
         void set_arg         ( void * ptr, size_t s ) ; 
         void set_func_name   ( std::string const &func ) ; 
         void set_num_rows    ( int const &rows ) ; 
         void set_input_files ( std::vector<std::string> const &in ) ; 
         void set_output_path ( std::string const &out ) ; 

         inline int initialize_input_stream( )
         {
            if( input_fileset.size() == 0 )
	    {
	       return saga_sectorsphere::error::SAGA_SECSP_E_NO_SPHERE_INPUT ; 
	    }
	    return (input.init( input_fileset )) ; 
	 }

	 inline int initialize_output_stream( )
	 {
	    if( !output_path.empty() )
	    {
	       output.setOutputPath( output_path , func )  ; 
               output.init( 256 ) ; 
               return saga_sectorsphere::error::SAGA_SECSP_E_SUCCESS ; 
	    }

	    return saga_sectorsphere::error::SAGA_SECSP_E_NO_SPHERE_OUTPUT ; 
	 }

         void get_state    ( saga::job::state &st ) ; 
	 int  upload       ( std::string const &dir, std::vector<std::string> &files ) ; 

         void get_error_msg( std::string & err, int const &code ) ; 
  };

}

