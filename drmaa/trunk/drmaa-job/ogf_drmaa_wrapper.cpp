//  Copyright (c) 2010 Poznan Supercomputing and Networking Center (mamonski@man.poznan.pl)
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE or copy at
//   http://www.boost.org/LICENSE_1_0.txt)


#include <dlfcn.h>

#include "drmaa.h"
#include "ogf_drmaa_wrapper.hpp"

#include <saga/saga.hpp>
#include <saga/impl/exception.hpp>

namespace sja = saga::job::attributes;


namespace psnc_drmaa
{


//private methods
    void drmaa::throw_drmaa_exception(int retcode, const char *msg, const char *errbuf)
    {
    	SAGA_OSSTREAM strm;

    	strm << msg <<  ": " << errbuf << " (retcode = " << retcode << ")";

    	std::string message = SAGA_OSSTREAM_GETSTRING(strm);

    	SAGA_LOG_ERROR(message);

    	throw message;
    }

	void drmaa::control_job(std::string job_id, int operation_code)
	{
		char errbuf[DRMAA_ERROR_STRING_BUFFER] = "Internal error";
		int ret;

		SAGA_LOG_INFO("-> control_job(" + job_id + "," + boost::lexical_cast<std::string>(operation_code) + ")");
		if ((ret = api_.control(job_id.c_str(), operation_code, errbuf, sizeof(errbuf) -1 )) != DRMAA_ERRNO_SUCCESS)
			throw_drmaa_exception(ret, "drmaa_control_job failed", errbuf);
	}

//init
	drmaa::drmaa()
	{
		SAGA_LOG_INFO("-> drmaa()");
		handle_ = 0;
		memset(&api_, 0, sizeof(api_));
		initialized_ = 0;
	}

	drmaa::~drmaa()
	{
		char errbuf[DRMAA_ERROR_STRING_BUFFER] = "Internal Error";

		SAGA_LOG_INFO("-> ~drmaa()");

		if (initialized_) {
			api_.exit(errbuf, sizeof(errbuf) - 1);
		}

		if (handle_) {
			dlclose(handle_);
		}

	}

	void drmaa::init(std::string drmaa_path, std::string contact_string)
	{
		state = saga::job::New;
		char errbuf[DRMAA_ERROR_STRING_BUFFER] = "Internal Error";
		char impl[DRMAA_DRMAA_IMPL_BUFFER] = "";
		char drms_string[DRMAA_DRM_SYSTEM_BUFFER] = "";
		int ret = DRMAA_ERRNO_SUCCESS;
		unsigned int major, minor;

		SAGA_LOG_INFO("-> init(" + drmaa_path + (contact_string != "" ? ")" : "," + contact_string + ")"));

		handle_ = dlopen(drmaa_path.c_str(), RTLD_NOW | RTLD_LOCAL);

		if (!handle_) {
			const char *msg = dlerror();
			if (!msg)
				msg = drmaa_path.c_str();
			SAGA_LOG_ERROR("Could not load DRMAA library: %s" + std::string(msg));
			goto fault;
		}

		if ((api_.init = (drmaa_init_function_t)dlsym(handle_, "drmaa_init")) == 0) goto fault;
		if ((api_.exit = (drmaa_exit_function_t)dlsym(handle_, "drmaa_exit")) == 0) goto fault;
		if ((api_.allocate_job_template = (drmaa_allocate_job_template_function_t)dlsym(handle_, "drmaa_allocate_job_template")) == 0) goto fault;
		if ((api_.delete_job_template = (drmaa_delete_job_template_function_t)dlsym(handle_, "drmaa_delete_job_template")) == 0) goto fault;
		if ((api_.set_attribute = (drmaa_set_attribute_function_t)dlsym(handle_, "drmaa_set_attribute")) == 0) goto fault;
		if ((api_.get_attribute = (drmaa_get_attribute_function_t)dlsym(handle_, "drmaa_get_attribute")) == 0) goto fault;
		if ((api_.set_vector_attribute = (drmaa_set_vector_attribute_function_t)dlsym(handle_, "drmaa_set_vector_attribute")) == 0) goto fault;
		if ((api_.get_vector_attribute = (drmaa_get_vector_attribute_function_t)dlsym(handle_, "drmaa_get_vector_attribute")) == 0) goto fault;
		if ((api_.run_job = (drmaa_run_job_function_t)dlsym(handle_, "drmaa_run_job")) == 0) goto fault;
		if ((api_.control = (drmaa_control_function_t)dlsym(handle_, "drmaa_control")) == 0) goto fault;
		if ((api_.job_ps = (drmaa_job_ps_function_t)dlsym(handle_, "drmaa_job_ps")) == 0) goto fault;
		if ((api_.wait = (drmaa_wait_function_t)dlsym(handle_, "drmaa_wait")) == 0) goto fault;
		if ((api_.wifexited = (drmaa_wifexited_function_t)dlsym(handle_, "drmaa_wifexited")) == 0) goto fault;
		if ((api_.wexitstatus = (drmaa_wexitstatus_function_t)dlsym(handle_, "drmaa_wexitstatus")) == 0) goto fault;
		if ((api_.wifsignaled = (drmaa_wifsignaled_function_t)dlsym(handle_, "drmaa_wifsignaled")) == 0) goto fault;
		if ((api_.wtermsig = (drmaa_wtermsig_function_t)dlsym(handle_, "drmaa_wtermsig")) == 0) goto fault;
		if ((api_.wcoredump = (drmaa_wcoredump_function_t)dlsym(handle_, "drmaa_wcoredump")) == 0) goto fault;
		if ((api_.wifaborted = (drmaa_wifaborted_function_t)dlsym(handle_, "drmaa_wifaborted")) == 0) goto fault;
		if ((api_.strerror = (drmaa_strerror_function_t)dlsym(handle_, "drmaa_strerror")) == 0) goto fault;
		if ((api_.get_contact = (drmaa_get_contact_function_t)dlsym(handle_, "drmaa_get_contact")) == 0) goto fault;
		if ((api_.version = (drmaa_version_function_t)dlsym(handle_, "drmaa_version")) == 0) goto fault;
		if ((api_.get_DRM_system = (drmaa_get_DRM_system_function_t)dlsym(handle_, "drmaa_get_DRM_system")) == 0) goto fault;
		if ((api_.get_DRMAA_implementation = (drmaa_get_DRMAA_implementation_function_t)dlsym(handle_, "drmaa_get_DRMAA_implementation")) == 0) goto fault;

		SAGA_LOG_INFO("-> drmaa_init()");

		if ((ret = api_.init(NULL, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
			goto fault;

		initialized_ = 1;

		if ((ret = api_.version(&major, &minor, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
			goto fault;

		if (major != 1 || minor != 0) {
			SAGA_LOG_ERROR("Unsupported DRMAA version: " + boost::lexical_cast<std::string>(major) + "." + boost::lexical_cast<std::string>(minor));
			goto fault;
		}

		if ((ret = api_.get_DRMAA_implementation(impl, sizeof(impl) - 1, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
			goto fault;

		if ((ret = api_.get_DRM_system(drms_string, sizeof(drms_string) - 1, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
			goto fault;

		SAGA_LOG_INFO("DRM system: " + std::string(drms_string) + ", DRMAA implementation: " + std::string(impl));

		return;
	fault:
		SAGA_LOG_ERROR("Failed to set up DRMAA: " +  std::string(errbuf));
		if (initialized_)
			api_.exit(errbuf, sizeof(errbuf) - 1);
		if (handle_)
			dlclose(handle_);

		throw_drmaa_exception(ret, "Failed to setup DRMAA library", errbuf);
	}

//other public methods
	saga::job::state drmaa::get_state(std::string job_id)
	{
		char errbuf[DRMAA_ERROR_STRING_BUFFER] = "Internal Error";
		int ret, ps = DRMAA_PS_UNDETERMINED;

		SAGA_LOG_INFO("-> get_state(" + job_id + ")");

		ret = api_.job_ps(job_id.c_str(), &ps, errbuf, sizeof(errbuf) - 1);

		if (ret != DRMAA_ERRNO_SUCCESS)
			throw_drmaa_exception(ret, "drmaa_job_ps failed", errbuf);
		//TODO: INVALID_JOB?


		SAGA_LOG_INFO("DRMAA status " + boost::lexical_cast<std::string>(ps));

		switch (ps)
		{
			case DRMAA_PS_QUEUED_ACTIVE:
			case DRMAA_PS_SYSTEM_ON_HOLD:
			case DRMAA_PS_USER_ON_HOLD:
			case DRMAA_PS_USER_SYSTEM_ON_HOLD:
			case DRMAA_PS_RUNNING:
			case DRMAA_PS_SYSTEM_SUSPENDED:
			case DRMAA_PS_USER_SUSPENDED:
			case DRMAA_PS_USER_SYSTEM_SUSPENDED:
				state = saga::job::Running;
				break;
			case DRMAA_PS_UNDETERMINED:
				SAGA_LOG_ERROR("Unable to determine job state");
				break;
			case DRMAA_PS_DONE:
				state = saga::job::Done;
				break;
			case DRMAA_PS_FAILED:
				state = saga::job::Failed;
				break;
			default:
				SAGA_LOG_ERROR("Invalid DRMAA job state " + boost::lexical_cast<std::string>(ps));
		}

		SAGA_LOG_INFO("<- get_state: "+ state);

		return state;
	}

	std::string drmaa::run_job(saga::job::description job_description)
	{
		char errbuf[DRMAA_ERROR_STRING_BUFFER] = "Internal Error";
		char drmaa_jobid[DRMAA_JOBNAME_BUFFER] = "";
		drmaa_job_template_t *job_template = NULL;
		const char **args = NULL;
		const char **env = NULL;
		int ret;


		SAGA_LOG_INFO("-> run_job(" + job_description.get_attribute(sja::description_executable)+ ")");

		try {
			if ((ret = api_.allocate_job_template(&job_template, errbuf, sizeof(errbuf) -1)) != DRMAA_ERRNO_SUCCESS) {
				throw_drmaa_exception(ret, "Allocating job template failed", errbuf);
			}

			if ((ret = api_.set_attribute(job_template, DRMAA_REMOTE_COMMAND, job_description.get_attribute(sja::description_executable).c_str(), errbuf, sizeof(errbuf) -1)) != DRMAA_ERRNO_SUCCESS) {
				throw_drmaa_exception(ret, "Setting DRMAA_REMOTE_COMMAND failed", errbuf);
			}


			if (job_description.attribute_exists(sja::description_arguments)) {
				std::vector <std::string>args_vector = job_description.get_vector_attribute(sja::description_arguments);

				args = (const char **)calloc(args_vector.size() + 1, sizeof(char*)); //TODO: check

				for (unsigned int i = 0; i < args_vector.size(); i++) {
					args[i] = (char *)args_vector[i].c_str();
				}

				if ((ret = api_.set_vector_attribute(job_template, DRMAA_V_ARGV, args, errbuf, sizeof(errbuf) -1)) != DRMAA_ERRNO_SUCCESS) {
					throw_drmaa_exception(ret, "Setting DRMAA_V_ARGS failed", errbuf);
				}
			}

			if (job_description.attribute_exists(sja::description_input)) {
				if ((ret = api_.set_attribute(job_template, DRMAA_INPUT_PATH, job_description.get_attribute(sja::description_input).c_str(), errbuf, sizeof(errbuf) -1)) != DRMAA_ERRNO_SUCCESS) {
					throw_drmaa_exception(ret, "Setting DRMAA_INPUT_PATH failed", errbuf);
				}
			}

			if (job_description.attribute_exists(sja::description_output)) {
				if ((ret = api_.set_attribute(job_template, DRMAA_OUTPUT_PATH, job_description.get_attribute(sja::description_output).c_str(), errbuf, sizeof(errbuf) -1)) != DRMAA_ERRNO_SUCCESS) {
					throw_drmaa_exception(ret, "Setting DRMAA_OUTPUT_PATH failed", errbuf);
				}
			}

			if (job_description.attribute_exists(sja::description_error)) {
				if ((ret = api_.set_attribute(job_template, DRMAA_ERROR_PATH, job_description.get_attribute(sja::description_error).c_str(), errbuf, sizeof(errbuf) -1)) != DRMAA_ERRNO_SUCCESS) {
					throw_drmaa_exception(ret, "Setting DRMAA_ERROR_PATH failed", errbuf);
				}
			}

			if (job_description.attribute_exists(sja::description_working_directory)) {
				if ((ret = api_.set_attribute(job_template, DRMAA_WD, job_description.get_attribute(sja::description_working_directory).c_str(), errbuf, sizeof(errbuf) -1)) != DRMAA_ERRNO_SUCCESS) {
					throw_drmaa_exception(ret, "Setting DRMAA_WD failed", errbuf);
				}
			}

			if (job_description.attribute_exists(sja::description_environment)) {
				throw_drmaa_exception(DRMAA_ERRNO_INTERNAL_ERROR, "Setting DRMAA_V_ENV failed", errbuf);
			}

			if (job_description.attribute_exists(sja::description_wall_time_limit)) {
				if ((ret = api_.set_attribute(job_template, DRMAA_WCT_HLIMIT, job_description.get_attribute(sja::description_wall_time_limit).c_str(), errbuf, sizeof(errbuf) -1)) != DRMAA_ERRNO_SUCCESS) {
					throw_drmaa_exception(ret, "Setting DRMAA_WCT failed", errbuf);
				}
			}

			if ((ret = api_.run_job(drmaa_jobid, sizeof(drmaa_jobid) - 1, job_template, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS) {
				throw_drmaa_exception(ret, "run_job failed", errbuf);
			}


			api_.delete_job_template(job_template, errbuf, sizeof(errbuf) - 1);
			job_template = NULL;

			if (args) {
				free(args);
				args = NULL;
			}

			if (env) {
				free(env);
				env = NULL;
			}

			std::string job_id(drmaa_jobid);

			SAGA_LOG_INFO("<- run_job: " + job_id);
			return job_id;
		} catch (...) {
			if (job_template != NULL)
				api_.delete_job_template(job_template, errbuf, sizeof(errbuf) - 1);

			if (args) {
				free(args);
				args = NULL;
			}

			if (env) {
				free(env);
				env = NULL;
			}
			throw;
		}
	}

	bool drmaa::wait_job(std::string job_id, int timeout)
	{
		char jobid[DRMAA_JOBNAME_BUFFER] = "";
		char errbuf[DRMAA_ERROR_STRING_BUFFER] = "Internal Error";
		drmaa_attr_values_t *rusage = NULL;
		int status = 0;
		int ret;

		SAGA_LOG_INFO("-> wait_job(" + job_id + "," + boost::lexical_cast<std::string>(timeout) + ")");

		ret = api_.wait(job_id.c_str(), jobid, sizeof(jobid) - 1, &status, timeout, &rusage, errbuf, sizeof(errbuf) - 1);

		if (ret == DRMAA_ERRNO_EXIT_TIMEOUT) {
			return false;
		} else if (ret == DRMAA_ERRNO_SUCCESS || ret == DRMAA_ERRNO_NO_RUSAGE) {
			//TODO free rusage
			return true;
		} else {
			throw_drmaa_exception(ret, "wait failed", errbuf);
		}


		return true;
	}


	void drmaa::kill_job(std::string job_id)
	{
		SAGA_LOG_INFO("-> kill_job(" + job_id + ")");
		control_job(job_id, DRMAA_CONTROL_TERMINATE);
	}

	void drmaa::suspend_job(std::string job_id)
	{
		SAGA_LOG_INFO("-> suspend_job(" + job_id + ")");
		control_job(job_id, DRMAA_CONTROL_SUSPEND);
	}

	void drmaa::resume_job(std::string job_id)
	{
		SAGA_LOG_INFO("-> resume_job(" + job_id + ")");
		control_job(job_id, DRMAA_CONTROL_RESUME);
	}

	void drmaa::hold_job(std::string job_id)
	{
		SAGA_LOG_INFO("-> hold_job(" + job_id + ")");
		control_job(job_id, DRMAA_CONTROL_HOLD);
	}

	void drmaa::release_job(std::string job_id)
	{
		SAGA_LOG_INFO("-> release_job(" + job_id + ")");
		control_job(job_id, DRMAA_CONTROL_RELEASE);
	}


}
