//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

// system includes
#include <string.h>

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/attribute.hpp>
#include <saga/saga/adaptors/file_transfer_spec.hpp>

// saga engine includes
#include <saga/impl/config.hpp>
#include <saga/impl/exception_list.hpp>

// saga package includes
#include <saga/saga/packages/job/adaptors/job_self.hpp>
#include <saga/saga/packages/job/job_description.hpp>

// adaptor includes
#include "ogf_drmaa_job.hpp"

namespace sja = saga::job::attributes;

////////////////////////////////////////////////////////////////////////
namespace ogf_drmaa_job
{
  // constructor
  job_cpi_impl::job_cpi_impl (proxy                           * p, 
                              cpi_info const                  & info,
                              saga::ini::ini const            & glob_ini, 
                              saga::ini::ini const            & adap_ini,
                              TR1::shared_ptr <saga::adaptor>   adaptor)
    : base_cpi (p, info, adaptor, cpi::Noflags)
    , session_ (p->get_session ())
    , state_   (saga::job::New)
  {
    instance_data     idata (this);
    adaptor_data_type adata (this);

    saga::url contact_url = idata->rm_;

    SAGA_LOG_INFO("url: " + contact_url.get_url ());

    // check if URL is usable
    if ( ! contact_url.get_scheme ().empty ()    &&
           contact_url.get_scheme () != "drmaa"    &&
           contact_url.get_scheme () != "any"    )
    {
      SAGA_OSSTREAM strm;
      strm << "Could not initialize job service for [" << contact_url << "]. "
           << "Only these schemas are supported: any://, drmaa://, or none.";

      SAGA_ADAPTOR_THROW (SAGA_OSSTREAM_GETSTRING (strm), 
                          saga::adaptors::AdaptorDeclined);
    }

    // TODO: load drmaa && drmaa_init
    SAGA_LOG_INFO("getting DRMAA singleton");

    drmaa_ = &(saga::adaptors::utils::get_singleton<psnc_drmaa::drmaa>());

    if ( idata->init_from_jobid_ )
    {
      SAGA_ADAPTOR_THROW ("Job reconnect is not yet implemented", saga::NotImplemented);
    }
    else
    {
      // init from job description
      jd_ = idata->jd_;
      state_ = saga::job::New;
      
      if ( ! jd_.attribute_exists (sja::description_executable) )
      {
        SAGA_ADAPTOR_THROW ("job description misses executable", saga::BadParameter);
      }
    }

    // FIXME: register metrics etc.
  }


  // destructor
  job_cpi_impl::~job_cpi_impl (void)
  {
	  //TODO: delete JT, drmaa_exit ?
	SAGA_LOG_INFO("releasing DRMAA singleton");
  }


  //  SAGA API functions
  void job_cpi_impl::sync_get_state (saga::job::state & ret)
  {
    adaptor_data_type adata (this);

    ret = drmaa_->get_state(jobid_);

  }

  void job_cpi_impl::sync_get_description (saga::job::description & ret)
  {
    ret = jd_;
  }

  void job_cpi_impl::sync_get_job_id (std::string & ret)
  {
    ret = jobid_;
  }

  void job_cpi_impl::sync_get_stdin (saga::job::ostream & ret)
  {
    // not available in DRMAA
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_get_stdout (saga::job::istream & ret)
  {
    // not available in DRMAA
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_get_stderr (saga::job::istream & ret)
  {
    // not available in DRMAA
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_checkpoint (saga::impl::void_t & ret)
  {
    // not available in DRMAA
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_migrate (saga::impl::void_t     & ret, 
                                   saga::job::description   jd)
  {
    // not available in DRMAA
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_signal (saga::impl::void_t & ret, 
                                  int            signal)
  {
    // not available in DRMAA
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }


  //  suspend the child process 
  void job_cpi_impl::sync_suspend (saga::impl::void_t & ret)
  {
	drmaa_->suspend_job(jobid_);
  }

  //  suspend the child process 
  void job_cpi_impl::sync_resume (saga::impl::void_t & ret)
  {
    drmaa_->resume_job(jobid_);
  }


  //////////////////////////////////////////////////////////////////////
  // inherited from the task interface
  void job_cpi_impl::sync_run (saga::impl::void_t & ret)
  {
    if ( state_ != saga::job::New )
    {
      SAGA_ADAPTOR_THROW ("can run only 'New' jobs", saga::IncorrectState);
    }

    jobid_   =  drmaa_->run_job(jd_);

    SAGA_LOG_INFO("Successfully submitted job: " + jobid_);;
  }

  void job_cpi_impl::sync_cancel (saga::impl::void_t & ret, 
                                  double timeout)
  {
    try
    {
      drmaa_->kill_job(jobid_);
    }
    catch ( const char * msg )
    {
      SAGA_ADAPTOR_THROW (msg, saga::NoSuccess);
    }
  }

  //  wait for the child process to terminate
  void job_cpi_impl::sync_wait (bool   & ret, 
                                double   timeout)
  {
    adaptor_data_type adata(this);
    drmaa_->wait_job(jobid_, (int)timeout);
  }

  // TODO: add state polling and metrics support

} // namespace ogf_drmaa_job
////////////////////////////////////////////////////////////////////////


#if 0

/* $Id: submission_drmaa.c 310 2010-10-13 14:04:19Z mamonski $ */

/*
 * Copyright 2006-2009 SMOA Project Team
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/types.h>
#include <dlfcn.h>
#include <pwd.h>
#include <unistd.h>

#include <smoa-core/sm.h>
#include <smoa-core/logger.h>
#include <smoa-core/value.h>
#include <smoa-core/xmalloc.h>
#include <smoa-core/modules/modules.h>

#include <drmaa.h>
#include <include/submission_module.h>
#include <include/namespaces.h>
#include <xmlstructs/xmlstructs.h>

__SM_ID("$Id: submission_drmaa.c 310 2010-10-13 14:04:19Z mamonski $");

#define NATIVE_SPECIFICATION_ARID_GE      "-ar "
#define NATIVE_SPECIFICATION_ARID_LSF     "-U "
#define NATIVE_SPECIFICATION_ARID_TORQUE  "-W x=FLAGS:ADVRES:"
#define MAUI_SCHEDUL_INTERVAL             (30)

static SM_BOOL warn_if_no_transfer_support = SM_TRUE;
static SM_BOOL transfer_files_supported = SM_FALSE;
static unsigned int refresh_waited_jobs_period = 15;

static int setup(sm_module_t);
static void cleanup(sm_module_t);
static sm_value_t *submission_drmaa_submit(sm_module_t, sm_value_t *);
static sm_value_t *submission_drmaa_terminate(sm_module_t, sm_value_t *);
static sm_value_t *submission_drmaa_get_statuses(sm_module_t, sm_value_t *);
static sm_value_t *submission_drmaa_wait(sm_module_t);
static sm_value_t *submission_drmaa_get_drms(sm_module_t);

static submission_drms_t drms;
static char *drms_ns = NULL;

SUBMISSION_MODULE = {

	.header = {
		.entry_size = SUBMISSION_MODULE_ENTRY_SIZE,
		.api_type = SUBMISSION_MODULE_API,

		.name = "submission_drmaa",
		.descr = "DRMAA submission",

		.version_major = 0,
		.version_minor = 1,

		.setup = setup,
		.cleanup = cleanup,

		.multiple_instances_supported = SM_FALSE
	},

	.submit = submission_drmaa_submit,
	.terminate = submission_drmaa_terminate,
	.get_statuses = submission_drmaa_get_statuses,
	.wait = submission_drmaa_wait,
	.get_drms = submission_drmaa_get_drms
};

static void
remove_newlines(char *buf)
{
	char *ptr;

	if ((ptr = strrchr(buf, '\n')))
		*ptr = 0;
	if ((ptr = strrchr(buf, '\r')))
		*ptr = 0;
}

#define SET_ERRBUF() \
	do { \
		char __buf[64]; \
		if (!strlen(errbuf)) \
			strlcpy(errbuf, drmaa_strerror(ret), sizeof(errbuf)); \
		remove_newlines(errbuf); \
		snprintf(__buf, sizeof(__buf), " (DRMAA errno = %d)", ret); \
		strlcat(errbuf, __buf, sizeof(errbuf)); \
	} while (0)

static int
setup(sm_module_t this)
{
	const char *path = NULL;
	void *handle = NULL;
	char errbuf[DRMAA_ERROR_STRING_BUFFER] = "";
	char attr[DRMAA_ATTR_BUFFER] = "";
	char impl[DRMAA_DRMAA_IMPLEMENTATION_BUFFER] = "";
	char drms_string[DRMAA_DRM_SYSTEM_BUFFER] = "";
	drmaa_attr_names_t *attributes = NULL;
	int ret = DRMAA_ERRNO_SUCCESS;
	unsigned int major, minor;
	int initialized = 0;

	SM_DEBUG_ENTER();

	if (sm_conf_node_str(this->conf_ctxt, SM_FALSE, "/@path", &path) != SM_OK)
		goto fault;

	if (sm_conf_node_bool(this->conf_ctxt, SM_FALSE, "/conf:WarnIfNoTransferSupport", &warn_if_no_transfer_support) == SM_FAULT)
		goto fault;

	if (sm_conf_node_uint(this->conf_ctxt, SM_FALSE, "/conf:RefreshWaitedJobsPeriod", &refresh_waited_jobs_period) == SM_FAULT)
		goto fault;

	sm_debug("warn_if_no_transfer_support = %d", warn_if_no_transfer_support);
	sm_debug("refresh_waited_jobs_period = %d", refresh_waited_jobs_period);

	handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
	if (!handle) {
		const char *msg = dlerror();
		if (!msg)
			msg = path;
		sm_error("Could not load DRMAA library: %s", msg);
		goto fault;
	}

	sm_debug("-> drmaa_init()");

	if ((ret = drmaa_init(NULL, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
		goto fault;

	initialized = 1;

	if ((ret = drmaa_get_attribute_names(&attributes, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
		goto fault;

	while ((ret = drmaa_get_next_attr_name(attributes, attr, sizeof(attr) - 1)) == DRMAA_ERRNO_SUCCESS) {
		sm_debug("DRMAA supported attribute: %s", attr);

		if (!strcmp(attr, DRMAA_TRANSFER_FILES))
			transfer_files_supported = SM_TRUE;
	}

	drmaa_release_attr_names(attributes);
	attributes = NULL;

	if (ret != DRMAA_ERRNO_NO_MORE_ELEMENTS) {
		sm_error("This is most likely not a true 1.0 version of DRMAA implementation (DRMAA_ERRNO_NO_MORE_ELEMENTS not available)");

		/* not a DRMAA error */
		ret = DRMAA_ERRNO_SUCCESS;
		goto fault;
	}

	if ((ret = drmaa_version(&major, &minor, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
		goto fault;

	if (major != 1 || minor != 0) {
		sm_error("Unsupported DRMAA version: %d.%d", major, minor);
		goto fault;
	}

	if ((ret = drmaa_get_DRMAA_implementation(impl, sizeof(impl) - 1, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
		goto fault;

	if ((ret = drmaa_get_DRM_system(drms_string, sizeof(drms_string) - 1, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
		goto fault;

	sm_debug("DRM system: %s, DRMAA implementation: %s", drms_string, impl);

	{
		unsigned int i;
		/*hunt for characters not allowed in xsd:URI */
		for (i=0; i < strlen(drms_string); i++) {
			if (drms_string[i] == ' ')
				drms_string[i] = '-';
		}
	}

	if (strstr(drms_string, "GE")) {
		drms = SUBMISSION_DRMS_GE;
		drms_ns = sm_asprintf("%s/%s", SMC_NAMESPACE_DRMS_SGE, drms_string);
	} else if (strstr(drms_string, "LSF")) {
		drms = SUBMISSION_DRMS_LSF;
		strtok(drms_string, " ");
		strtok(NULL, " ");
		drms_ns = sm_asprintf("%s/%s", SMC_NAMESPACE_DRMS_LSF, strtok(NULL, " "));
	} else if (strstr(drms_string, "Torque")) {
		drms = SUBMISSION_DRMS_TORQUE;
		drms_ns = sm_asprintf("%s/%s", SMC_NAMESPACE_DRMS_TORQUE, drms_string);
	} else if (strstr(drms_string, "PBSPro")) {
		drms = SUBMISSION_DRMS_PBSPRO;
		drms_ns = sm_asprintf("%s/%s", SMC_NAMESPACE_DRMS_PBSPRO, drms_string);
	} else {
		drms = SUBMISSION_DRMS_UNKNOWN;
		drms_ns = sm_asprintf("%s/%s", SMC_NAMESPACE_DRMS_UNKNOWN, drms_string);
	}

	this->ctxt = handle;
	return SM_OK;

fault:
	if (ret != DRMAA_ERRNO_SUCCESS) {
		SET_ERRBUF();
		sm_error("Failed to set up DRMAA: %s", errbuf);
	}
	if (attributes)
		drmaa_release_attr_names(attributes);
	if (initialized)
		drmaa_exit(errbuf, sizeof(errbuf) - 1);
	if (handle)
		dlclose(handle);

	return SM_FAULT;
}

static void
cleanup(sm_module_t this)
{
	char errbuf[DRMAA_ERROR_STRING_BUFFER] = "";

	sm_free(drms_ns);

	SM_DEBUG_ENTER();

	if (this->ctxt) {
		drmaa_exit(errbuf, sizeof(errbuf) - 1);
		dlclose(this->ctxt);
		this->ctxt = NULL;
	}
}

#define CHECK_ELEMENT(element, ignored) \
	do { \
		struct soap_dom_attribute *__attr; \
		\
		if (!(element)) \
			break; \
		\
		if ((element)->__any) \
			return sm_ret_code_create(SUBMISSION_UNSUPPORTED_FEATURE_FAULT, "Unsupported " SM_STR(element) " element extension in submission_drmaa"); \
		\
		for (__attr = &(element)->__anyAttribute; __attr; __attr = __attr->next) { \
			if (!__attr->name) continue; \
			if (!strncmp(__attr->name, "xmlns", strlen("xmlns"))) continue; \
			if (*ignored && !strcmp(__attr->name, ignored)) continue; \
			return sm_ret_code_create(SUBMISSION_UNSUPPORTED_FEATURE_FAULT, "Unsupported " SM_STR(element) " attribute extension in submission_drmaa"); \
		} \
	} while (0);

#define CHECK_FIELD(element) \
	do { \
		if (element) \
			return sm_ret_code_create(SUBMISSION_UNSUPPORTED_FEATURE_FAULT, "Unsupported element: " SM_STR(element)); \
	} while (0);

static sm_ret_code_t
check_supported(_jsdl__JobDefinition *jsdl__JobDefinition)
{
	SM_MANDATORY_P(jsdl__JobDefinition);

	SM_DEBUG_ENTER();

	CHECK_ELEMENT(jsdl__JobDefinition, "id");
	CHECK_ELEMENT(jsdl__JobDefinition->jsdl__JobDescription, "");
	CHECK_ELEMENT(jsdl__JobDefinition->jsdl__JobDescription->jsdl__JobIdentification, "");
	CHECK_ELEMENT(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Application, "");
	CHECK_ELEMENT(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources, "");

	if (jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources) {
		CHECK_FIELD(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources->jsdl__CandidateHosts);
		CHECK_FIELD(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources->jsdl__FileSystem);
		CHECK_FIELD(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources->jsdl__ExclusiveExecution);
		CHECK_FIELD(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources->jsdl__OperatingSystem);
		CHECK_FIELD(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources->jsdl__CPUArchitecture);
		CHECK_FIELD(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources->jsdl__IndividualCPUSpeed);
		CHECK_FIELD(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources->jsdl__IndividualCPUTime);
		CHECK_FIELD(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources->jsdl__IndividualCPUCount);
		CHECK_FIELD(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources->jsdl__IndividualNetworkBandwidth);
		CHECK_FIELD(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources->jsdl__IndividualPhysicalMemory);
		CHECK_FIELD(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources->jsdl__IndividualVirtualMemory);
		CHECK_FIELD(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources->jsdl__IndividualDiskSpace);
		CHECK_FIELD(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources->jsdl__TotalCPUTime);
		CHECK_FIELD(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources->jsdl__TotalCPUCount);
		CHECK_FIELD(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources->jsdl__TotalPhysicalMemory);
		CHECK_FIELD(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources->jsdl__TotalVirtualMemory);
		CHECK_FIELD(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources->jsdl__TotalDiskSpace);
		CHECK_FIELD(jsdl__JobDefinition->jsdl__JobDescription->jsdl__Resources->jsdl__TotalResourceCount);
	}

	if (jsdl__JobDefinition->jsdl__JobDescription->jsdl__Application) {
		if (jsdl__JobDefinition->jsdl__JobDescription->jsdl__Application->jsdl_hpcpa__HPCProfileApplication)
			return sm_ret_code_create_ok();
	}

	return sm_ret_code_create(SUBMISSION_UNSUPPORTED_FEATURE_FAULT, "Only HPCPApplication supported in submission_drmaa module");
}

static drmaa_job_template_t *
jsdl_to_drmaa_job_template(const char *jsdlstr, const char *arid, int arduration, sm_value_t **result)
{
	char errbuf[DRMAA_ERROR_STRING_BUFFER] = "";
	drmaa_job_template_t *jt = NULL;
	sm_alloc_tracked_t tracked = NULL;
	_jsdl__JobDefinition jsdl;
	_jsdl_hpcpa__HPCProfileApplication *app = NULL;
	char transfer_files[16] = "";
	int i;
	int ret = DRMAA_ERRNO_SUCCESS;
	const char *working_dir = NULL;
	struct passwd *pw;
	char *native_spec = NULL;
	char *job_category = NULL;
	char *tmp;
	sm_ret_code_t retc;

	SM_MANDATORY_P(jsdlstr);
	SM_MANDATORY_P(result);

	SM_DEBUG_ENTER_STR(jsdlstr);

	*result = NULL;

	if (!(tracked = sm_gsoap_buffer_get(jsdlstr, &jsdl, jsdl__JobDefinition_USCOREType, "jsdl:JobDefinition", xmlstructs_namespaces))) {
		sm_error("Failed to parse JSDL");
		goto fault;
	}

	retc = check_supported(&jsdl);
	if (retc.code) {
		sm_value_free(result);
		*result = sm_value_create(1, SM_VALUE_RET_CODE, retc.code, retc.string);
		goto fault;
	}

	app = jsdl.jsdl__JobDescription->jsdl__Application->jsdl_hpcpa__HPCProfileApplication;

	if ((ret = drmaa_allocate_job_template(&jt, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
		goto fault;

	if (!(pw = getpwuid(getuid()))) {
		sm_perror("getpwuid");
		goto fault;
	}

	/* just an assertion... */
	if (app->jsdl_hpcpa__UserName && strcmp(app->jsdl_hpcpa__UserName->__item, pw->pw_name)) {
		sm_error("DRMAA process running as different user (%s) than the requested one (%s)", pw->pw_name, app->jsdl_hpcpa__UserName->__item);
		goto fault;
	}

	if (!app->jsdl_hpcpa__WorkingDirectory) {
		sm_error("jsdl_hpcpa__WorkingDirectory must always be set");
		goto fault;
	}

	SM_TRY_NOT(working_dir = sm_alloc_tracked_strdup(tracked, app->jsdl_hpcpa__WorkingDirectory->__item));

	if (jsdl.jsdl__JobDescription->jsdl__Resources) {
		if ((tmp = jsdl.jsdl__JobDescription->jsdl__Resources->jsdl_smoa_comp_factory__NativeSpecification))
			SM_TRY_NOT(native_spec = sm_alloc_tracked_strdup(tracked, tmp));
		if ((tmp = jsdl.jsdl__JobDescription->jsdl__Resources->jsdl_smoa_comp_factory__JobCategory))
			SM_TRY_NOT(job_category = sm_alloc_tracked_strdup(tracked, tmp));
	}

	if (arid) {
		const char *option = NULL;

		if (drms == SUBMISSION_DRMS_GE)
			option = NATIVE_SPECIFICATION_ARID_GE;
		else if (drms == SUBMISSION_DRMS_LSF)
			option = NATIVE_SPECIFICATION_ARID_LSF;
		else if (drms == SUBMISSION_DRMS_TORQUE) {
			/* make Torque + maui to behave simlar to the LSF and SGE */
			if (!native_spec || !strstr(native_spec, "walltime"))
				SM_TRY_NOT(native_spec = sm_alloc_tracked_asprintf(tracked, "%s -l walltime=%u", (native_spec ? native_spec : ""), (arduration > MAUI_SCHEDUL_INTERVAL ? arduration - MAUI_SCHEDUL_INTERVAL : arduration) ));

			option = NATIVE_SPECIFICATION_ARID_TORQUE;
		} else {
			sm_notice("Cannot request Advance Reservation. Unsupported DRM system.");
			goto fault;
		}

		SM_TRY_NOT(native_spec = sm_alloc_tracked_asprintf(tracked, "%s%s %s", option, arid, (native_spec ? native_spec : "")));
	}

	if (native_spec) {
		sm_debug("Setting DRMAA_NATIVE_SPECIFICATION: \"%s\"", native_spec);
		if ((ret = drmaa_set_attribute(jt, DRMAA_NATIVE_SPECIFICATION, native_spec, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
			goto fault;
	}

	if (job_category) {
		sm_debug("Setting DRMAA_JOB_CATEGORY: \"%s\"", job_category);
		if ((ret = drmaa_set_attribute(jt, DRMAA_JOB_CATEGORY, job_category, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
			goto fault;
	}

	if (jsdl.jsdl__JobDescription->jsdl__JobIdentification && (tmp = jsdl.jsdl__JobDescription->jsdl__JobIdentification->jsdl__JobName)) {
		sm_debug("Setting DRMAA_JOB_NAME: \"%s\"", tmp);
		if ((ret = drmaa_set_attribute(jt, DRMAA_JOB_NAME, tmp, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
			goto fault;
	}

	if (app->jsdl_hpcpa__Executable) {
		sm_debug("Setting DRMAA_REMOTE_COMMAND: \"%s\"", app->jsdl_hpcpa__Executable->__item);
		if ((ret = drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, app->jsdl_hpcpa__Executable->__item, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
			goto fault;
	}

	if (app->jsdl_hpcpa__Argument) {
		char **args;

		if (!(args = sm_alloc_tracked_calloc(tracked, sizeof(char *), app->__sizeArgument + 1)))
			goto fault;

		for (i = 0; i < app->__sizeArgument; i++) {
			sm_debug("Setting DRMAA_V_ARGV[%d]: \"%s\"", i, app->jsdl_hpcpa__Argument[i].__item);
			args[i] = app->jsdl_hpcpa__Argument[i].__item;
		}

		args[app->__sizeArgument] = NULL;

		if ((ret = drmaa_set_vector_attribute(jt, DRMAA_V_ARGV, (const char **) args, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
			goto fault;
	}

	if (app->jsdl_hpcpa__Input) {
		char *input;

		strlcat(transfer_files, "i", sizeof(transfer_files));

		if (app->jsdl_hpcpa__Input->__item[0] != '/') {
			if (!(input = sm_alloc_tracked_asprintf(tracked, ":%s/%s", working_dir, app->jsdl_hpcpa__Input->__item)))
				goto fault;
		} else {
			if (!(input = sm_alloc_tracked_asprintf(tracked, ":%s", app->jsdl_hpcpa__Input->__item)))
				goto fault;
		}

		sm_debug("Setting DRMAA_INPUT_PATH: \"%s\"", input);
		if ((ret = drmaa_set_attribute(jt, DRMAA_INPUT_PATH, input, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
			goto fault;
	}

	if (app->jsdl_hpcpa__Output) {
		char *output;

		strlcat(transfer_files, "o", sizeof(transfer_files));

		if (app->jsdl_hpcpa__Output->__item[0] != '/') {
			if (!(output = sm_alloc_tracked_asprintf(tracked, ":%s/%s", working_dir, app->jsdl_hpcpa__Output->__item)))
				goto fault;
		} else {
			if (!(output = sm_alloc_tracked_asprintf(tracked, ":%s", app->jsdl_hpcpa__Output->__item)))
				goto fault;
		}

		sm_debug("Setting DRMAA_OUTPUT_PATH: \"%s\"", output);
		if ((ret = drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, output, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
			goto fault;
	} else {
		sm_debug("Setting DRMAA_OUTPUT_PATH to /dev/null");
		if ((ret = drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, ":/dev/null", errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
			goto fault;
	}

	if (app->jsdl_hpcpa__Error) {
		char *error;

		strlcat(transfer_files, "e", sizeof(transfer_files));

		if (app->jsdl_hpcpa__Error->__item[0] != '/') {
			if (!(error = sm_alloc_tracked_asprintf(tracked, ":%s/%s", working_dir, app->jsdl_hpcpa__Error->__item)))
				goto fault;
		} else {
			if (!(error = sm_alloc_tracked_asprintf(tracked, ":%s", app->jsdl_hpcpa__Error->__item)))
				goto fault;
		}

		sm_debug("Setting DRMAA_ERROR_PATH: \"%s\"", error);
		if ((ret = drmaa_set_attribute(jt, DRMAA_ERROR_PATH, error, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
			goto fault;
	} else {
		sm_debug("Setting DRMAA_ERROR_PATH to /dev/null");
		if ((ret = drmaa_set_attribute(jt, DRMAA_ERROR_PATH, ":/dev/null", errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
			goto fault;
	}

	/* transfering files to execution files is useless as almost EVERY productive cluster has shared file system : TODO: make it configurable
	if (transfer_files_supported) {
		if (strlen(transfer_files)) {
			sm_debug("Setting DRMAA_TRANSFER_FILES: \"%s\"", transfer_files);
			if ((ret = drmaa_set_attribute(jt, DRMAA_TRANSFER_FILES, transfer_files, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
				goto fault;
		}
	} else {
		if (warn_if_no_transfer_support)
			sm_notice("DRMAA implementation does not support requesting file transfer from/to submit/execution host");
	}*/

	if (working_dir) {
		sm_debug("Setting DRMAA_WD: \"%s\"", working_dir);
		if ((ret = drmaa_set_attribute(jt, DRMAA_WD, working_dir, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
			goto fault;
	}

	/* XXX: GFD.133 says that it's 1/0 for DRMAA_BLOCK_EMAIL and y/n for DRMAA_JOIN_FILES */
	(void) drmaa_set_attribute(jt, DRMAA_BLOCK_EMAIL, "1", errbuf, sizeof(errbuf) - 1);

	for (i = 0; i < app->__sizeEnvironment; i++) {
		char **env;

		if (!(env = sm_alloc_tracked_calloc(tracked, sizeof(char *), app->__sizeEnvironment + 1)))
			goto fault;

		for (i = 0; i < app->__sizeEnvironment; i++) {
			if (!(env[i] = sm_alloc_tracked_asprintf(tracked, "%s=%s", app->jsdl_hpcpa__Environment[i].name, app->jsdl_hpcpa__Environment[i].__item)))
				break;

			sm_debug("Setting DRMAA_V_ENV[%d]: \"%s\"", i, env[i]);
		}

		env[app->__sizeEnvironment] = NULL;

		if (i != app->__sizeEnvironment)
			goto fault;

		if ((ret = drmaa_set_vector_attribute(jt, DRMAA_V_ENV, (const char **) env, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS)
			goto fault;
	}

	sm_alloc_tracked_free(&tracked);
	return jt;

fault:
	if (ret != DRMAA_ERRNO_SUCCESS) {
		SET_ERRBUF();

		switch (ret) {
			case DRMAA_ERRNO_INVALID_ATTRIBUTE_FORMAT:
			case DRMAA_ERRNO_INVALID_ARGUMENT:
			case DRMAA_ERRNO_INVALID_ATTRIBUTE_VALUE:
			case DRMAA_ERRNO_CONFLICTING_ATTRIBUTE_VALUES:
				*result = sm_value_create(1, SM_VALUE_RET_CODE, SUBMISSION_INVALID_REQUEST_MESSAGE_FAULT, errbuf);
				break;
			default:
				*result = sm_value_create(1, SM_VALUE_RET_CODE, SUBMISSION_INTERNAL_FAULT, errbuf);
				break;
		}
	}

	if (jt)
		drmaa_delete_job_template(jt, errbuf, sizeof(errbuf) - 1);

	sm_alloc_tracked_free(&tracked);
	return NULL;
}

static sm_value_t *
submission_drmaa_submit(sm_module_t this, sm_value_t *params)
{
	char errbuf[DRMAA_ERROR_STRING_BUFFER] = "";
	char jobid[DRMAA_JOBNAME_BUFFER] = "";
	sm_value_t *result = NULL;
	drmaa_job_template_t *jt = NULL;
	int ret;
	const char *arid = NULL;
	unsigned int arduration = 0;

	SM_MANDATORY_P(this);
	SM_MANDATORY_P(params);

	SM_DEBUG_ENTER();

	assert(SM_VALUE_IS_STRING(params[0]));

	if (sm_value_count(params) == 3) {
		assert(SM_VALUE_IS_STRING(params[1]));
		arid = params[1]->vstring;
		assert(SM_VALUE_IS_UINT32(params[2]));
		arduration = params[2]->vint32;
	}

	if (!(jt = jsdl_to_drmaa_job_template(params[0]->vstring, arid, arduration, &result)))
		goto fault;

	sm_debug("-> drmaa_run_job()");

	ret = drmaa_run_job(jobid, sizeof(jobid) - 1, jt, errbuf, sizeof(errbuf) - 1);

	if (ret != DRMAA_ERRNO_SUCCESS) {
		SET_ERRBUF();
		sm_debug("drmaa_run_job failed: %d:%s", ret, errbuf);
	}

	switch (ret) {
		case DRMAA_ERRNO_SUCCESS:
			sm_info("Submitted job: %s", jobid);
			result = sm_value_create(2, SM_VALUE_RET_CODE, SUBMISSION_OK, SM_VALUE_STRING, jobid);
			break;
		case DRMAA_ERRNO_TRY_LATER:
			sm_notice("DRM system didn't accept new submission, probably saturated");
			result = sm_value_create(1, SM_VALUE_RET_CODE, SUBMISSION_INTERNAL_FAULT, "Try again later");
			break;
		case DRMAA_ERRNO_DENIED_BY_DRM:
		case DRMAA_ERRNO_INVALID_ATTRIBUTE_FORMAT:
		case DRMAA_ERRNO_INVALID_ATTRIBUTE_VALUE:
		case DRMAA_ERRNO_INVALID_ARGUMENT:
			result = sm_value_create(1, SM_VALUE_RET_CODE, SUBMISSION_INVALID_REQUEST_MESSAGE_FAULT, errbuf);
			break;
		case DRMAA_ERRNO_AUTH_FAILURE:
			result = sm_value_create(1, SM_VALUE_RET_CODE, SUBMISSION_NOT_AUTHORIZED_FAULT, errbuf);
			break;
		default:
			result = sm_value_create(1, SM_VALUE_RET_CODE, SUBMISSION_INTERNAL_FAULT, errbuf);
			break;
	}

	drmaa_delete_job_template(jt, errbuf, sizeof(errbuf) - 1);
	return result;

fault:
	if (jt)
		drmaa_delete_job_template(jt, errbuf, sizeof(errbuf) - 1);
	if (result && !SM_VALUE_RET_CODE_OK(result[0]))
		return result;
	sm_value_free(&result);
	return sm_value_create(1, SM_VALUE_RET_CODE, SUBMISSION_INTERNAL_FAULT, "Internal fault in submission_drmaa module");
}

static sm_value_t *
submission_drmaa_terminate(sm_module_t this, sm_value_t *identifiers)
{
	char errbuf[DRMAA_ERROR_STRING_BUFFER] = "";
	sm_value_t *result = NULL;
	size_t i;
	size_t count;
	int ret;

	SM_MANDATORY_P(this);
	SM_MANDATORY_P(identifiers);

	SM_DEBUG_ENTER();

	count = sm_value_count(identifiers);

	for (i = 0; i < count; i++) {
		assert(SM_VALUE_IS_STRING(identifiers[i]));

		sm_debug("-> drmaa_control(%s, DRMAA_CONTROL_TERMINATE)", identifiers[i]->vstring);

		if ((ret = drmaa_control(identifiers[i]->vstring, DRMAA_CONTROL_TERMINATE, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS) {
			SET_ERRBUF();
			sm_debug("drmaa_control failed: %d:%s", ret, errbuf);

			switch (ret) {
				case DRMAA_ERRNO_AUTH_FAILURE:
					SM_TRY_NOT(sm_value_append(&result, 1, SM_VALUE_RET_CODE, SUBMISSION_NOT_AUTHORIZED_FAULT, errbuf));
					break;
				case DRMAA_ERRNO_INVALID_ARGUMENT:
				case DRMAA_ERRNO_INVALID_JOB:
					SM_TRY_NOT(sm_value_append(&result, 1, SM_VALUE_RET_CODE, SUBMISSION_UNKNOWN_ACTIVITY_IDENTIFIER_FAULT, identifiers[i]->vstring));
					break;
				default:
					SM_TRY_NOT(sm_value_append(&result, 1, SM_VALUE_RET_CODE, SUBMISSION_INTERNAL_FAULT, errbuf));
					break;
			}

		} else {
			SM_TRY_NOT(sm_value_append(&result, 1, SM_VALUE_RET_CODE, SUBMISSION_OK));
		}
	}

	return result;

fault:
	sm_value_free(&result);
	return sm_value_create(1, SM_VALUE_RET_CODE, SUBMISSION_INTERNAL_FAULT, "Internal fault in submission_drmaa module");
}

static sm_value_t *
submission_drmaa_get_statuses(sm_module_t this, sm_value_t *identifiers)
{
	char errbuf[DRMAA_ERROR_STRING_BUFFER] = "";
	sm_value_t *result = NULL;
	size_t i;
	size_t count;
	int ret;
	int undetermined_tries;

	SM_MANDATORY_P(this);
	SM_MANDATORY_P(identifiers);

	SM_DEBUG_ENTER();

	undetermined_tries = 0;

	count = sm_value_count(identifiers);

	for (i = 0; i < count; i++) {
		int ps;

		assert(SM_VALUE_IS_STRING(identifiers[i]));

again:
		sm_debug("-> drmaa_job_ps(%s)", identifiers[i]->vstring);

		if ((ret = drmaa_job_ps(identifiers[i]->vstring, &ps, errbuf, sizeof(errbuf) - 1)) != DRMAA_ERRNO_SUCCESS) {
			SET_ERRBUF();
			sm_debug("drmaa_job_ps(%s) failed: %d:%s", identifiers[i]->vstring, ret, errbuf);

			switch (ret) {
				case DRMAA_ERRNO_AUTH_FAILURE:
					SM_TRY_NOT(sm_value_append(&result, 1, SM_VALUE_RET_CODE, SUBMISSION_NOT_AUTHORIZED_FAULT, errbuf));
					break;
				case DRMAA_ERRNO_INVALID_ARGUMENT:
				case DRMAA_ERRNO_INVALID_JOB:
					SM_TRY_NOT(sm_value_append(&result, 1, SM_VALUE_RET_CODE, SUBMISSION_UNKNOWN_ACTIVITY_IDENTIFIER_FAULT, identifiers[i]->vstring));
					break;
				default:
					SM_TRY_NOT(sm_value_append(&result, 1, SM_VALUE_RET_CODE, SUBMISSION_INTERNAL_FAULT, errbuf));
					break;
			}

		} else {
			submission_status_t status = 0;
			int pretend_reaped = 0;

			switch (ps) {
				case DRMAA_PS_UNDETERMINED:
					/* XXX: Ugh, ugly! But what else can we do? */
					sm_debug("DRMAA_PS_UNDETERMINED (%d)", undetermined_tries);
					if (undetermined_tries++ > 5) {
						SM_TRY_NOT(sm_value_append(&result, 1, SM_VALUE_RET_CODE, SUBMISSION_INTERNAL_FAULT, "Could not determine the status of the job. Try again later."));
					} else {
						sleep(1);
						goto again;
					}
					break;
				case DRMAA_PS_QUEUED_ACTIVE:
					status = SUBMISSION_QUEUED;
					break;
				case DRMAA_PS_SYSTEM_ON_HOLD:
				case DRMAA_PS_USER_ON_HOLD:
				case DRMAA_PS_USER_SYSTEM_ON_HOLD:
					status = SUBMISSION_HELD;
					break;
				case DRMAA_PS_RUNNING:
					status = SUBMISSION_RUNNING;
					break;
				case DRMAA_PS_SYSTEM_SUSPENDED:
				case DRMAA_PS_USER_SUSPENDED:
				case DRMAA_PS_USER_SYSTEM_SUSPENDED:
					status = SUBMISSION_SUSPENDED;
					break;
				case DRMAA_PS_DONE:
				case DRMAA_PS_FAILED:
					sm_debug("Job is actually done. Pretending it was reaped.");
					pretend_reaped = 1;
					break;
				default:
					SM_ASSERT(0, "drmaa_job_ps(%s) returned unknown job status: %d", identifiers[i]->vstring, ps);
			}

			undetermined_tries = 0;

			if (pretend_reaped)
				SM_TRY_NOT(sm_value_append(&result, 1, SM_VALUE_RET_CODE, SUBMISSION_UNKNOWN_ACTIVITY_IDENTIFIER_FAULT, identifiers[i]->vstring));
			else
				SM_TRY_NOT(sm_value_append(&result, 2, SM_VALUE_RET_CODE, SUBMISSION_OK, SM_VALUE_INT32, status));
		}
	}

	return result;

fault:
	sm_value_free(&result);
	return sm_value_create(1, SM_VALUE_INT32, SUBMISSION_INTERNAL_FAULT, "Internal fault in submission_drmaa module");
}

static sm_value_t *
submission_drmaa_wait(sm_module_t this)
{
	char jobid[DRMAA_JOBNAME_BUFFER] = "";
	char errbuf[DRMAA_ERROR_STRING_BUFFER] = "";
	drmaa_attr_values_t *rusage = NULL;
	int status = 0;
	sm_value_t *result = NULL;
	int ret;
	int exited, signaled;

	SM_MANDATORY_P(this);

	SM_DEBUG_ENTER();

	while ((ret = drmaa_wait(DRMAA_JOB_IDS_SESSION_ANY, jobid, sizeof(jobid) - 1, &status, refresh_waited_jobs_period, &rusage, errbuf, sizeof(errbuf) - 1)) == DRMAA_ERRNO_EXIT_TIMEOUT)
		continue;

	sm_debug("%d <- drmaa_wait", ret);

	if (ret != DRMAA_ERRNO_SUCCESS && ret != DRMAA_ERRNO_NO_RUSAGE) {
		SET_ERRBUF();
		return sm_value_create(1, SM_VALUE_RET_CODE, SUBMISSION_INTERNAL_FAULT, errbuf);
	}

	if (drmaa_wifexited(&exited, status, errbuf, sizeof(errbuf) - 1) != DRMAA_ERRNO_SUCCESS) {
		sm_error("drmaa_wifexited failed: %d:%s", ret, errbuf);
		goto fault;
	}

	if (drmaa_wifsignaled(&signaled, status, errbuf, sizeof(errbuf) - 1) != DRMAA_ERRNO_SUCCESS) {
		sm_error("drmaa_wifsignaled failed: %d:%s", ret, errbuf);
		goto fault;
	}

	SM_TRY_NOT(result = sm_value_create(1, SM_VALUE_RET_CODE, SUBMISSION_OK));

	/* XXX: If all the implementations did drmaa_wifaborted() correctly we
	 * could use it as a hint to differentiate between Failed and Cancelled
	 * states on this level. But we don't do that. Only jobs terminated
	 * through our service are marked as Cancelled. If the job is
	 * terminated (while running) directly in DRMS, it is marked as Failed.
	 */

	if (exited) {
		int exit_status = 0;
		char buf[65536]; /*this buffer is so big because of the exec_host attribute of the torque DRMAA */

		(void) drmaa_wexitstatus(&exit_status, status, errbuf, sizeof(errbuf) - 1);

		sm_debug("Job %s has exited with exit status %d", jobid, exit_status);

		/* finished, jobid, exit_status, rusage */

		SM_TRY_NOT(sm_value_append(&result, 1, SM_VALUE_INT32, SUBMISSION_FINISHED));
		SM_TRY_NOT(sm_value_append(&result, 1, SM_VALUE_STRING, jobid));
		SM_TRY_NOT(sm_value_append(&result, 1, SM_VALUE_INT32, exit_status));

		if (rusage) {
			sm_value_t *rusagevalue = NULL;

			while ((ret = drmaa_get_next_attr_value(rusage, buf, sizeof(buf) - 1)) == DRMAA_ERRNO_SUCCESS) {
				sm_debug("rusage: %s", buf);

				if (!sm_value_append(&rusagevalue, 1, SM_VALUE_STRING, buf)) {
					sm_value_free(&rusagevalue);
					goto fault;
				}
			}

			if (!rusagevalue && ret != DRMAA_ERRNO_NO_MORE_ELEMENTS)
				sm_error("drmaa_get_next_attr_value failed: %s", drmaa_strerror(ret));

			if (rusagevalue && !sm_value_append(&result, 1, SM_VALUE_ARRAY, rusagevalue)) {
				sm_value_free(&rusagevalue);
				goto fault;
			} else if (!rusagevalue && !sm_value_append(&result, 1, SM_VALUE_NONE)) {
				goto fault;
			}
		} else
			SM_TRY_NOT(sm_value_append(&result, 1, SM_VALUE_NONE));

	} else {

		/* failed, jobid, core_dumped, termsig */

		SM_TRY_NOT(sm_value_append(&result, 1, SM_VALUE_INT32, SUBMISSION_FAILED));
		SM_TRY_NOT(sm_value_append(&result, 1, SM_VALUE_STRING, jobid));

		if (signaled) {
			char termsig[DRMAA_SIGNAL_BUFFER] = "";
			int core_dumped = 0;

			(void) drmaa_wtermsig(termsig, sizeof(termsig) - 1, status, errbuf, sizeof(errbuf) - 1);
			(void) drmaa_wcoredump(&core_dumped, status, errbuf, sizeof(errbuf) - 1);

			if (strlen(termsig))
				SM_TRY_NOT(sm_value_append(&result, 2, SM_VALUE_BOOL, core_dumped, SM_VALUE_STRING, termsig));
			else
				SM_TRY_NOT(sm_value_append(&result, 2, SM_VALUE_BOOL, core_dumped, SM_VALUE_NONE));
		} else
			SM_TRY_NOT(sm_value_append(&result, 2, SM_VALUE_BOOL, SM_FALSE, SM_VALUE_NONE));
	}

	assert(sm_value_count(result) == 5);
	return result;

fault:
	sm_value_free(&result);
	return sm_value_create(1, SM_VALUE_RET_CODE, SUBMISSION_INTERNAL_FAULT, "Internal fault in submission_drmaa module");
}

static sm_value_t *
submission_drmaa_get_drms(sm_module_t this sm_unused)
{
	SM_DEBUG_ENTER();

	return sm_value_create(3, SM_VALUE_RET_CODE, SUBMISSION_OK, SM_VALUE_INT32, drms, SM_VALUE_STRING, drms_ns);
}


#endif
