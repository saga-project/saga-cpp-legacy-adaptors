//  Copyright (c) 2010 Poznan Supercomputing and Networking Center (mamonski@man.poznan.pl)
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE or copy at
//   http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_OGF_DRMAA_WRAPPER_HPP
#define ADAPTORS_OGF_DRMAA_WRAPPER_HPP

#include <string>

#include <saga/saga.hpp>

#include "drmaa.h"
////////////////////////////////////////////////////////////////////////
namespace psnc_drmaa
{

  class drmaa
  {

	    typedef int (*drmaa_init_function_t)(const char *, char *, size_t );
	    typedef int (*drmaa_exit_function_t)(char *, size_t );
	    typedef int (*drmaa_allocate_job_template_function_t)(drmaa_job_template_t **, char *, size_t);
	    typedef int (*drmaa_delete_job_template_function_t)(drmaa_job_template_t *, char *, size_t);
	    typedef int (*drmaa_set_attribute_function_t)(drmaa_job_template_t *, const char *, const char *, char *, size_t);
	    typedef int (*drmaa_get_attribute_function_t)(drmaa_job_template_t *, const char *, char *, size_t , char *, size_t);
	    typedef int (*drmaa_set_vector_attribute_function_t)(drmaa_job_template_t *, const char *, const char *[], char *, size_t);
	    typedef int (*drmaa_get_vector_attribute_function_t)(drmaa_job_template_t *, const char *, drmaa_attr_values_t **, char *, size_t);
	    typedef int (*drmaa_run_job_function_t)(char *, size_t, const drmaa_job_template_t *, char *, size_t);
	    typedef int (*drmaa_control_function_t)(const char *, int, char *, size_t);
	    typedef int (*drmaa_job_ps_function_t)(const char *, int *, char *, size_t);
	    typedef int (*drmaa_wait_function_t)(const char *, char *, size_t, int *, signed long, drmaa_attr_values_t **, char *, size_t);
	    typedef int (*drmaa_wifexited_function_t)(int *, int, char *, size_t);
	    typedef int (*drmaa_wexitstatus_function_t)(int *exit_status, int, char *, size_t);
	    typedef int (*drmaa_wifsignaled_function_t)(int *signaled, int, char *, size_t);
	    typedef int (*drmaa_wtermsig_function_t)(char *signal, size_t signal_len, int, char *, size_t);
	    typedef int (*drmaa_wcoredump_function_t)(int *core_dumped, int, char *, size_t);
	    typedef int (*drmaa_wifaborted_function_t)(int *aborted, int, char *, size_t);
	    typedef const char (*drmaa_strerror_function_t)(int);
	    typedef int (*drmaa_get_contact_function_t)(char *, size_t , char *, size_t);
	    typedef int (*drmaa_version_function_t)(unsigned int *, unsigned int *, char *, size_t);
	    typedef int (*drmaa_get_DRM_system_function_t)(char *, size_t, char *, size_t);
	    typedef int (*drmaa_get_DRMAA_implementation_function_t)(char *, size_t, char *, size_t);

		typedef struct
		{
			drmaa_init_function_t init;
			drmaa_exit_function_t exit;
			drmaa_allocate_job_template_function_t allocate_job_template;
			drmaa_delete_job_template_function_t delete_job_template;
			drmaa_set_attribute_function_t set_attribute;
			drmaa_get_attribute_function_t get_attribute;
			drmaa_set_vector_attribute_function_t set_vector_attribute;
			drmaa_get_vector_attribute_function_t get_vector_attribute;
			drmaa_run_job_function_t run_job;
			drmaa_control_function_t control;
			drmaa_job_ps_function_t job_ps;
			drmaa_wait_function_t wait;
			drmaa_wifexited_function_t wifexited;
			drmaa_wexitstatus_function_t wexitstatus;
			drmaa_wifsignaled_function_t wifsignaled;
			drmaa_wtermsig_function_t wtermsig;
			drmaa_wcoredump_function_t wcoredump;
			drmaa_wifaborted_function_t wifaborted;
			drmaa_strerror_function_t strerror;
			drmaa_get_contact_function_t get_contact;
			drmaa_version_function_t version;
			drmaa_get_DRM_system_function_t get_DRM_system;
			drmaa_get_DRMAA_implementation_function_t get_DRMAA_implementation;
		} drmaa_api;

    private:


	  void control_job(std::string job_id, int operation_code);
	  void throw_drmaa_exception(int retcode, const char *msg, const char *errbuf);
	  saga::job::state state;
	  void *handle_;
	  drmaa_api api_;
	  int initialized_;

    public:

	  drmaa();
	  ~drmaa();


      void init(std::string drmaa_path, std::string contact_string);

      saga::job::state get_state(std::string job_id);

      std::string run_job(saga::job::description job_description);

      bool wait_job(std::string job_id, int timeout);

      void kill_job(std::string job_id);
      void suspend_job(std::string job_id);
      void resume_job(std::string job_id);
      void hold_job(std::string job_id);
      void release_job(std::string job_id);

  };

} // namespace psnc_drmaa

#endif // ADAPTORS_OGF_DRMAA_WRAPPER_HPP

