
// INSERT YOUR LICENSE BOILER PLATE HERE

#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

#include "ninfg_rpc.hpp"

extern "C" {
  #include <grpc.h>
}


///////////////////////////////////////////////////////////////////////////////
namespace ninfg_rpc
{
  /*
   * constructor
   */
  rpc_cpi_impl::rpc_cpi_impl (proxy * p, 
                              cpi_info       const & info, 
                              saga::ini::ini const & glob_ini, 
                              saga::ini::ini const & adap_ini,
                              TR1::shared_ptr <saga::adaptor> adaptor)
    :   base_cpi (p, info, adaptor, cpi::Noflags)
  {
    grpc_error_t err;

    std::cout << "Initializing" << std::endl;

    /* gridrpc initialization */
    err = grpc_initialize(NULL);
    if (err != GRPC_NO_ERROR && err != GRPC_ALREADY_INITIALIZED) {
      SAGA_ADAPTOR_THROW ("Failed to invoke grpc_initialize().", saga::NoSuccess);
    } 

    std::cout << "Creating an RPC handle" << std::endl;

    instance_data data (this);
    saga::url fn_url(data->funcname_);

    int port = fn_url.get_port();
    if (port == -1) { 
      err = grpc_function_handle_init(&rpc_handle,
                                      const_cast<char *>(fn_url.get_host().c_str()),
                                      const_cast<char *>(fn_url.get_path().substr(1).c_str()));
    } else {
      if (port > 0 && port < 65536) {
        char host_plus_port[fn_url.get_host().length() + 8];
        sprintf(host_plus_port, "%s:%d", fn_url.get_host().c_str(), fn_url.get_port());
        err = grpc_function_handle_init(&rpc_handle,
                                        host_plus_port,
                                        const_cast<char *>(fn_url.get_path().substr(1).c_str()));
      } else {
        SAGA_ADAPTOR_THROW ("Invalid port number.", saga::NoSuccess);
      }
    }

    if (err != GRPC_NO_ERROR) {
      SAGA_ADAPTOR_THROW ("Failed to invoke grpc_function_handle_init().", saga::NoSuccess);
    }

    /* FIXEME: should support the following url conversion.
      host name 
      host name:port number
      host name:port number/jobmanager  
      host name/jobmanager  
      host name:/jobmanager  
      host name::subject
      host name:port number:subject  
      host name/jobmanager:subject  
      host name:/jobmanager:subject  
      host name:port number/jobmanager:subject  
    */

  }

  /*
   * destructor
   */
  rpc_cpi_impl::~rpc_cpi_impl (void)
  {
    grpc_error_t err;

    std::cout << "Destructing the RPC handle" << std::endl;

    err = grpc_function_handle_destruct(&rpc_handle);
    if (err != GRPC_NO_ERROR) {
      SAGA_ADAPTOR_THROW ("Failed to invoke grpc_function_handle_destruct().", saga::NoSuccess);
    }

    std::cout << "Finalizing" << std::endl;

    err = grpc_finalize();
    if (err != GRPC_NO_ERROR) {
      SAGA_ADAPTOR_THROW ("Failed to invoke grpc_finalize().", saga::NoSuccess);
    }
  }

  /*
   * SAGA API functions
   */
  void rpc_cpi_impl::sync_call (saga::impl::void_t &, 
                                std::vector <saga::rpc::parameter> & args)
  {
    grpc_error_t err;
    int num_args = args.size();

    /* FIXME: grpc_arg_stack_*() are a Ninf-G special function but
              the issue of the arguments array is under discussion
              in the GridRPC-WG.
    */ 
    grpc_arg_stack_t *rpc_params = grpc_arg_stack_new(num_args);
    for (int i=0; i<num_args; i++) {
      grpc_arg_stack_push_arg(rpc_params, args.at(i).get_data());
    }

    err = grpc_call_arg_stack(&rpc_handle, rpc_params);
    if(err != GRPC_NO_ERROR) {
      SAGA_ADAPTOR_THROW ("Failed to invoke grpc_call().", saga::NoSuccess);
    }
  }

  void rpc_cpi_impl::sync_close (saga::impl::void_t &, 
                                 double timeout)
  {
    /* FIZME: should cancel the rpc session, if it is running? */
  }

  saga::task rpc_cpi_impl::async_call (std::vector <saga::rpc::parameter> & args)
  {
    return saga::adaptors::task ("rpc_cpi_impl::async_call",
                                 shared_from_this (),
                                 &rpc_cpi_impl::sync_call, 
                                 args);

  }

  saga::task rpc_cpi_impl::async_close (double timeout)
  {
    return saga::adaptors::task ("rpc_cpi_impl::async_close",
                                 shared_from_this (),
                                 &rpc_cpi_impl::sync_close, 
                                 timeout);
  }

}   // namespace ninfg_rpc
///////////////////////////////////////////////////////////////////////////////

