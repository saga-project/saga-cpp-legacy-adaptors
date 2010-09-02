//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_AWS_JOB_SERVICE_HPP
#define ADAPTORS_AWS_JOB_SERVICE_HPP

// stl includes
#include <string>
#include <iosfwd>

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga engine includes
#include <saga/impl/engine/proxy.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/saga/adaptors/adaptor_data.hpp>

// saga package includes
#include <saga/impl/packages/job/job_service_cpi.hpp>

// adaptor includes
#include "aws_job_adaptor.hpp"


////////////////////////////////////////////////////////////////////////
namespace aws_job
{
  class job_service_cpi_impl 
    : public saga::adaptors::v1_0::job_service_cpi <job_service_cpi_impl>
  {
    private:
      typedef saga::adaptors::v1_0::job_service_cpi <job_service_cpi_impl> 
              base_cpi;

      // adaptor data
      typedef saga::adaptors::adaptor_data <adaptor> adaptor_data;

      // instance id and ip
      std::string vm_id_;
      std::string vm_ip_;

      saga::context      ctx_;          // context used to access this job service
      saga::url          ssh_url_;      // rm url for ssh job service
      saga::context      ssh_context_;  // context to use for ssh ops
      saga::session      ssh_session_;  // session to use for ssh ops
      std::string        user_;         // user id on the VN instance
      std::string        userkey_;      // private key to access the VM instance
      std::string        usercert_;     // public  key to access the VM instance

      TR1::shared_ptr <saga::job::service> js_;  // ssh job service, does the real work

      std::map <std::string, std::string> ini_;
      std::map <std::string, std::string> env_;

      void dump_context (saga::context c);


    public:
      // constructor of the job_service cpi
      job_service_cpi_impl  (proxy                           * p, 
                             cpi_info const                  & info,
                             saga::ini::ini const            & glob_ini, 
                             saga::ini::ini const            & adap_ini,
                             TR1::shared_ptr <saga::adaptor>   adaptor);

      // destructor of the job_service cpi
      ~job_service_cpi_impl (void);

      // CPI functions
      void sync_create_job (saga::job::job            & ret, 
                            saga::job::description      jd);
      void sync_run_job    (saga::job::job            & ret, 
                            std::string                 cmd, 
                            std::string                 host, 
                            saga::job::ostream        & in, 
                            saga::job::istream        & out, 
                            saga::job::istream        & err);
      void sync_list       (std::vector <std::string> & ret);
      void sync_get_job    (saga::job::job            & ret,
                            std::string                 jobid);
      void sync_get_self   (saga::job::self           & ret);
      void sync_get_url    (saga::url                 & ret);



  };  // class job_service_cpi_impl

} // namespace aws_job
////////////////////////////////////////////////////////////////////////

#endif // ADAPTORS_AWS_JOB_SERVICE_HPP

