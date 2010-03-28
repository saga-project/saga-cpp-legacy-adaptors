//  Copyright (c) 2005-2009 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(ADAPTOR_GRIDSAM_JOBSUBMISSION_HK20070912_0917AM)
#define ADAPTOR_GRIDSAM_JOBSUBMISSION_HK20070912_0917AM

#include <string>
#include <vector>
#include <map>
#include <stdexcept>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <saga/saga/url.hpp>
#include <saga/saga/job.hpp>

#include <saga/impl/config.hpp>
#include <saga/impl/exception_list.hpp>

#include "gsoap_helper.hpp"
#include "stubs/gridsam/gridsamJobSubmissionSOAPBindingProxy.h"

///////////////////////////////////////////////////////////////////////////////
namespace util
{
    ///////////////////////////////////////////////////////////////////////////
    class SubmitJobRequest 
    {
    public:
        SubmitJobRequest(soap* soap, std::string const& exename) 
          : registry_(soap),
            request_(registry_.create(soap_new__gridsam__submitJob, 
                                      soap_delete__gridsam__submitJob)),
            posix_app_(NULL)
        {
            // create and initialize job description instance
            gridsam__JobDescriptionType* jd = registry_.create(
                soap_new_gridsam__JobDescriptionType,
                soap_delete_gridsam__JobDescriptionType);

            // create and initialize the job definition instance
            jsdl__JobDefinition_USCOREType* jsdl_def = registry_.create(
                soap_new_jsdl__JobDefinition_USCOREType,
                soap_delete_jsdl__JobDefinition_USCOREType);

            // create JSDL job description instance
            jsdl__JobDescription_USCOREType* jsdl_jd = registry_.create(
                soap_new_jsdl__JobDescription_USCOREType,
                soap_delete_jsdl__JobDescription_USCOREType);
                
            // create JSDL resource description instance
            jsdl__Resources_USCOREType* jsdl_rd = registry_.create(
                soap_new_jsdl__Resources_USCOREType,
                soap_delete_jsdl__Resources_USCOREType);
                
            // create and initialize the application instance
            jsdl__Application_USCOREType* jsdl_app = registry_.create(
                soap_new_jsdl__Application_USCOREType,
                soap_delete_jsdl__Application_USCOREType);
                
            // create and initialize the POSIX application instance
            posix_app_ = registry_.create(
                soap_new_jsdlposix__POSIXApplication_USCOREType,
                soap_delete_jsdlposix__POSIXApplication_USCOREType);
                
            // create and initialize the executable file name
            jsdlposix__FileName_USCOREType* posix_exename = registry_.create(
                soap_new_jsdlposix__FileName_USCOREType,
                soap_delete_jsdlposix__FileName_USCOREType);
            posix_exename->__item = exename;
            
            // chain up all created elements
            posix_app_->jsdlposix__Executable = posix_exename;

            jsdl_jd->jsdl__Resources = jsdl_rd;
            jsdl_jd->jsdl__Application = jsdl_app;
            jsdl_def->jsdl__JobDescription = jsdl_jd;
            jd->jsdl__JobDefinition = jsdl_def;
            request_->gridsam__JobDescription.push_back(jd);
        }
        ~SubmitJobRequest() {}
        
        _gridsam__submitJob* get() { return request_; }
        
        // set the arguments in the request
        void set_arguments(std::vector<std::string> const& args)
        {
            if (args.empty())
                return;
                
            // create a proper array of arguments
            std::vector<std::string>::const_iterator end = args.end();
            for (std::vector<std::string>::const_iterator it = args.begin(); 
                 it != end; ++it)
            {
                jsdlposix__Argument_USCOREType* arg = registry_.create(
                    soap_new_jsdlposix__Argument_USCOREType,
                    soap_delete_jsdlposix__Argument_USCOREType);
                arg->__item = *it;
                posix_app_->jsdlposix__Argument.push_back(arg);
            }
        }

        // set the environment for the new job
        void set_environment(std::map<std::string, std::string> const& keyval)
        {
            if (keyval.empty())
                return;
                
            // create a proper array of arguments
            std::map<std::string, std::string>::const_iterator end = keyval.end();
            for (std::map<std::string, std::string>::const_iterator it = keyval.begin(); 
                 it != end; ++it)
            {
                jsdlposix__Environment_USCOREType* env = registry_.create(
                    soap_new_jsdlposix__Environment_USCOREType,
                    soap_delete_jsdlposix__Environment_USCOREType);
                env->name = (*it).first;
                env->__item = (*it).second;
                posix_app_->jsdlposix__Environment.push_back(env);
            }
        }

        // set working directory for the new job        
        void set_working_directory(std::string const& wd)
        {
            // create and initialize a new working directory item
            jsdlposix__DirectoryName_USCOREType* working_dir = 
                registry_.create(soap_new_jsdlposix__DirectoryName_USCOREType,
                    soap_delete_jsdlposix__DirectoryName_USCOREType);
            working_dir->__item = wd;
            posix_app_->jsdlposix__WorkingDirectory = working_dir;
        }
        
        // set the names of the redirected input/output files
        void set_stdin_file(std::string const& name)
        {
            jsdlposix__FileName_USCOREType* file = registry_.create(
                soap_new_jsdlposix__FileName_USCOREType,
                soap_delete_jsdlposix__FileName_USCOREType);
            file->__item = name;
            posix_app_->jsdlposix__Input = file;
        }
        
        void set_stdout_file(std::string const& name)
        {
            jsdlposix__FileName_USCOREType* file = registry_.create(
                soap_new_jsdlposix__FileName_USCOREType,
                soap_delete_jsdlposix__FileName_USCOREType);
            file->__item = name;
            posix_app_->jsdlposix__Output = file;
        }
        
        void set_stderr_file(std::string const& name)
        {
            jsdlposix__FileName_USCOREType* file = registry_.create(
                soap_new_jsdlposix__FileName_USCOREType,
                soap_delete_jsdlposix__FileName_USCOREType);
            file->__item = name;
            posix_app_->jsdlposix__Error = file;
        }
        
        // functionality for setting resource characteristics
        void set_TotalCPUCount(int count)
        {
            jsdl__RangeValue_USCOREType* val = registry_.create(
                soap_new_jsdl__RangeValue_USCOREType,
                soap_delete_jsdl__RangeValue_USCOREType);
            jsdl__Exact_USCOREType* exact = registry_.create(
                soap_new_jsdl__Exact_USCOREType,
                soap_delete_jsdl__Exact_USCOREType);
            
            exact->__item = count;
            val->Exact.push_back(exact);
            get_jsdl_resource_description()->jsdl__TotalCPUCount = val;
        }

        void set_TotalCPUTime(int count)
        {
            jsdl__RangeValue_USCOREType* val = registry_.create(
                soap_new_jsdl__RangeValue_USCOREType,
                soap_delete_jsdl__RangeValue_USCOREType);
            jsdl__Boundary_USCOREType* upper = registry_.create(
                soap_new_jsdl__Boundary_USCOREType,
                soap_delete_jsdl__Boundary_USCOREType);
            
            upper->__item = count;
            val->UpperBoundedRange = upper;
            get_jsdl_resource_description()->jsdl__TotalCPUTime = val;
        }

        void set_CandidateHosts(std::vector<std::string> const& args)
        {
            jsdl__CandidateHosts_USCOREType* hosts = registry_.create(
                soap_new_jsdl__CandidateHosts_USCOREType,
                soap_delete_jsdl__CandidateHosts_USCOREType);

            hosts->jsdl__HostName = args;
            get_jsdl_resource_description()->jsdl__CandidateHosts = hosts;
        }

        void set_TotalPhysicalMemory(double memory)
        {
            jsdl__RangeValue_USCOREType* val = registry_.create(
                soap_new_jsdl__RangeValue_USCOREType,
                soap_delete_jsdl__RangeValue_USCOREType);
            jsdl__Boundary_USCOREType* lower = registry_.create(
                soap_new_jsdl__Boundary_USCOREType,
                soap_delete_jsdl__Boundary_USCOREType);

            lower->__item = memory;
            val->LowerBoundedRange = lower;
            get_jsdl_resource_description()->jsdl__TotalPhysicalMemory = val;
        }

        void set_ProcessCountLimit(int count)
        {
            jsdlposix__Limits_USCOREType* val = registry_.create(
                soap_new_jsdlposix__Limits_USCOREType,
                soap_delete_jsdlposix__Limits_USCOREType);

            val->__item = boost::lexical_cast<std::string>(count);
            posix_app_->jsdlposix__ProcessCountLimit = val;
        }

        void set_WallTimeLimit(int limit)
        {
            jsdlposix__Limits_USCOREType* val = registry_.create(
                soap_new_jsdlposix__Limits_USCOREType,
                soap_delete_jsdlposix__Limits_USCOREType);

            val->__item = boost::lexical_cast<std::string>(limit);
            posix_app_->jsdlposix__WallTimeLimit = val;
        }

        void set_ThreadCountLimit(int count)
        {
            jsdlposix__Limits_USCOREType* val = registry_.create(
                soap_new_jsdlposix__Limits_USCOREType,
                soap_delete_jsdlposix__Limits_USCOREType);

            val->__item = boost::lexical_cast<std::string>(count);
            posix_app_->jsdlposix__ThreadCountLimit = val;
        }

        void set_job_name(std::string const& name)
        { 
            jsdl__JobDescription_USCOREType* jd = get_jsdl_job_description();
            if (NULL == jd->jsdl__JobIdentification)
            {
                jd = registry_.create(
                    soap_new_jsdl__JobDescription_USCOREType,
                    soap_delete_jsdl__JobDescription_USCOREType);
            }

            jsdl__JobIdentification_USCOREType* ident = jd->jsdl__JobIdentification;
            if (NULL == ident)
            {
                ident = registry_.create(
                    soap_new_jsdl__JobIdentification_USCOREType,
                    soap_delete_jsdl__JobIdentification_USCOREType);
                jd->jsdl__JobIdentification = ident;
            }

            if (NULL == ident->jsdl__JobName)
            {
                ident->jsdl__JobName = registry_.create(
                        soap_new_std__string, soap_delete_std__string);
            }
            *ident->jsdl__JobName = name;
        }
        void set_job_description(std::string const& desc)
        {
            jsdl__JobDescription_USCOREType* jd = get_jsdl_job_description();
            if (NULL == jd->jsdl__JobIdentification)
            {
                jd = registry_.create(
                    soap_new_jsdl__JobDescription_USCOREType,
                    soap_delete_jsdl__JobDescription_USCOREType);
            }

            jsdl__JobIdentification_USCOREType* ident = jd->jsdl__JobIdentification;
            if (NULL == ident)
            {
                ident = registry_.create(
                    soap_new_jsdl__JobIdentification_USCOREType,
                    soap_delete_jsdl__JobIdentification_USCOREType);
                jd->jsdl__JobIdentification = ident;
            }

            if (NULL == ident->jsdl__Description)
            {
                ident->jsdl__Description = registry_.create(
                        soap_new_std__string, soap_delete_std__string);
            }
            *ident->jsdl__Description = desc;
        }
        void set_job_annotation(std::vector<std::string> const& args)
        { 
            jsdl__JobDescription_USCOREType* jd = get_jsdl_job_description();
            if (NULL == jd->jsdl__JobIdentification)
            {
                jd = registry_.create(
                    soap_new_jsdl__JobDescription_USCOREType,
                    soap_delete_jsdl__JobDescription_USCOREType);
            }

            jsdl__JobIdentification_USCOREType* ident = jd->jsdl__JobIdentification;
            if (NULL == ident)
            {
                ident = registry_.create(
                    soap_new_jsdl__JobIdentification_USCOREType,
                    soap_delete_jsdl__JobIdentification_USCOREType);
                jd->jsdl__JobIdentification = ident;
            }

            ident->jsdl__JobAnnotation = args;
        }
        void set_job_project(std::vector<std::string> const& args)
        { 
            jsdl__JobDescription_USCOREType* jd = get_jsdl_job_description();
            if (NULL == jd->jsdl__JobIdentification)
            {
                jd = registry_.create(
                    soap_new_jsdl__JobDescription_USCOREType,
                    soap_delete_jsdl__JobDescription_USCOREType);
            }

            jsdl__JobIdentification_USCOREType* ident = jd->jsdl__JobIdentification;
            if (NULL == ident)
            {
                ident = registry_.create(
                    soap_new_jsdl__JobIdentification_USCOREType,
                    soap_delete_jsdl__JobIdentification_USCOREType);
                jd->jsdl__JobIdentification = ident;
            }

            ident->jsdl__JobProject = args;
        }

        void set_CPUArchitecture(std::string const& cpuarch)
        {
            struct soap_code_map {
                jsdl__ProcessorArchitectureEnumeration code;
                char const* const string;
            };
            
            static soap_code_map const processor_architectures[] =
            {
                { jsdl__ProcessorArchitectureEnumeration__sparc, 
                  saga::job::attributes::description_cpuarchitecture_sparc 
                },
                { jsdl__ProcessorArchitectureEnumeration__powerpc, 
                  saga::job::attributes::description_cpuarchitecture_powerpc 
                },
                { jsdl__ProcessorArchitectureEnumeration__x86, 
                  saga::job::attributes::description_cpuarchitecture_x86 
                },
                { jsdl__ProcessorArchitectureEnumeration__x86_USCORE32, 
                  saga::job::attributes::description_cpuarchitecture_x86_32 
                },
                { jsdl__ProcessorArchitectureEnumeration__x86_USCORE64, 
                  saga::job::attributes::description_cpuarchitecture_x86_64 
                },
                { jsdl__ProcessorArchitectureEnumeration__parisc, 
                  saga::job::attributes::description_cpuarchitecture_parisc 
                },
                { jsdl__ProcessorArchitectureEnumeration__mips, 
                  saga::job::attributes::description_cpuarchitecture_mips 
                },
                { jsdl__ProcessorArchitectureEnumeration__ia64, 
                  saga::job::attributes::description_cpuarchitecture_ia64 
                },
                { jsdl__ProcessorArchitectureEnumeration__arm, 
                  saga::job::attributes::description_cpuarchitecture_arm 
                },
                { jsdl__ProcessorArchitectureEnumeration__other, 
                  saga::job::attributes::description_cpuarchitecture_other 
                },
                { (jsdl__ProcessorArchitectureEnumeration)-1, 
                  NULL 
                }
            };

            int i = 0;
            for (/**/; NULL != processor_architectures[i].string; ++i)
            {
                if (cpuarch == processor_architectures[i].string)
                    break;
            } 
            if (NULL == processor_architectures[i].string) {
                throw std::runtime_error("SubmitJobRequest::set_CPUArchitecture: "
                    "couldn't match processor architecture name to a valid "
                    "enumerator value: '" + cpuarch + "'");
            }

            jsdl__CPUArchitecture_USCOREType* val = registry_.create(
                soap_new_jsdl__CPUArchitecture_USCOREType,
                soap_delete_jsdl__CPUArchitecture_USCOREType);

            val->jsdl__CPUArchitectureName = processor_architectures[i].code;
            get_jsdl_resource_description()->jsdl__CPUArchitecture = val;
        }

        void set_OperatingSystem(std::string const& ostype)
        {
            struct soap_code_map {
                jsdl__OperatingSystemTypeEnumeration code;
                char const* const string;
            };
            
            static soap_code_map const operating_system[] =
            {
                {   jsdl__OperatingSystemTypeEnumeration__Unknown, 
                    saga::job::attributes::detail::description_operating_system_unknown
                },
                {   jsdl__OperatingSystemTypeEnumeration__MACOS, 
                    saga::job::attributes::detail::description_operating_system_macos
                },
                {   jsdl__OperatingSystemTypeEnumeration__ATTUNIX, 
                    saga::job::attributes::detail::description_operating_system_attunix
                },
                {   jsdl__OperatingSystemTypeEnumeration__DGUX, 
                    saga::job::attributes::detail::description_operating_system_dgux
                },
                {   jsdl__OperatingSystemTypeEnumeration__DECNT, 
                    saga::job::attributes::detail::description_operating_system_decnt
                },
                {   jsdl__OperatingSystemTypeEnumeration__Tru64_USCOREUNIX, 
                    saga::job::attributes::detail::description_operating_system_true64_unix
                },
                {   jsdl__OperatingSystemTypeEnumeration__OpenVMS, 
                    saga::job::attributes::detail::description_operating_system_openvms
                },
                {   jsdl__OperatingSystemTypeEnumeration__HPUX, 
                    saga::job::attributes::detail::description_operating_system_hpux
                },
                {   jsdl__OperatingSystemTypeEnumeration__AIX, 
                    saga::job::attributes::detail::description_operating_system_aix
                },
                {   jsdl__OperatingSystemTypeEnumeration__MVS, 
                    saga::job::attributes::detail::description_operating_system_mvs
                },
                {   jsdl__OperatingSystemTypeEnumeration__OS400, 
                    saga::job::attributes::detail::description_operating_system_os400
                },
                {   jsdl__OperatingSystemTypeEnumeration__OS_USCORE2, 
                    saga::job::attributes::detail::description_operating_system_os_2
                },
                {   jsdl__OperatingSystemTypeEnumeration__JavaVM, 
                    saga::job::attributes::detail::description_operating_system_javavm
                },
                {   jsdl__OperatingSystemTypeEnumeration__MSDOS, 
                    saga::job::attributes::detail::description_operating_system_msdos
                },
                {   jsdl__OperatingSystemTypeEnumeration__WIN3x, 
                    saga::job::attributes::detail::description_operating_system_win3x
                },
                {   jsdl__OperatingSystemTypeEnumeration__WIN95, 
                    saga::job::attributes::detail::description_operating_system_win95
                },
                {   jsdl__OperatingSystemTypeEnumeration__WIN98, 
                    saga::job::attributes::detail::description_operating_system_win98
                },
                {   jsdl__OperatingSystemTypeEnumeration__WINNT, 
                    saga::job::attributes::detail::description_operating_system_winnt
                },
                {   jsdl__OperatingSystemTypeEnumeration__WINCE, 
                    saga::job::attributes::detail::description_operating_system_wince
                },
                {   jsdl__OperatingSystemTypeEnumeration__NCR3000, 
                    saga::job::attributes::detail::description_operating_system_ncr3000
                },
                {   jsdl__OperatingSystemTypeEnumeration__NetWare, 
                    saga::job::attributes::detail::description_operating_system_netware
                },
                {   jsdl__OperatingSystemTypeEnumeration__OSF, 
                    saga::job::attributes::detail::description_operating_system_osf
                },
                {   jsdl__OperatingSystemTypeEnumeration__DC_USCOREOS, 
                    saga::job::attributes::detail::description_operating_system_dc_os
                },
                {   jsdl__OperatingSystemTypeEnumeration__Reliant_USCOREUNIX, 
                    saga::job::attributes::detail::description_operating_system_reliant_unix
                },
                {   jsdl__OperatingSystemTypeEnumeration__SCO_USCOREUnixWare, 
                    saga::job::attributes::detail::description_operating_system_sco_unixware
                },
                {   jsdl__OperatingSystemTypeEnumeration__SCO_USCOREOpenServer, 
                    saga::job::attributes::detail::description_operating_system_sco_openserver
                },
                {   jsdl__OperatingSystemTypeEnumeration__Sequent, 
                    saga::job::attributes::detail::description_operating_system_sequent
                },
                {   jsdl__OperatingSystemTypeEnumeration__IRIX, 
                    saga::job::attributes::detail::description_operating_system_irix
                },
                {   jsdl__OperatingSystemTypeEnumeration__Solaris, 
                    saga::job::attributes::detail::description_operating_system_solaris
                },
                {   jsdl__OperatingSystemTypeEnumeration__SunOS, 
                    saga::job::attributes::detail::description_operating_system_sunos
                },
                {   jsdl__OperatingSystemTypeEnumeration__U6000, 
                    saga::job::attributes::detail::description_operating_system_u6000
                },
                {   jsdl__OperatingSystemTypeEnumeration__ASERIES, 
                    saga::job::attributes::detail::description_operating_system_aseries
                },
                {   jsdl__OperatingSystemTypeEnumeration__TandemNSK, 
                    saga::job::attributes::detail::description_operating_system_tandemnsk
                },
                {   jsdl__OperatingSystemTypeEnumeration__TandemNT, 
                    saga::job::attributes::detail::description_operating_system_tandemnt
                },
                {   jsdl__OperatingSystemTypeEnumeration__BS2000, 
                    saga::job::attributes::detail::description_operating_system_bs2000
                },
                {   jsdl__OperatingSystemTypeEnumeration__LINUX, 
                    saga::job::attributes::detail::description_operating_system_linux
                },
                {   jsdl__OperatingSystemTypeEnumeration__Lynx, 
                    saga::job::attributes::detail::description_operating_system_lynx
                },
                {   jsdl__OperatingSystemTypeEnumeration__XENIX, 
                    saga::job::attributes::detail::description_operating_system_xenix
                },
                {   jsdl__OperatingSystemTypeEnumeration__VM, 
                    saga::job::attributes::detail::description_operating_system_vm
                },
                {   jsdl__OperatingSystemTypeEnumeration__Interactive_USCOREUNIX, 
                    saga::job::attributes::detail::description_operating_system_interactive_unix
                },
                {   jsdl__OperatingSystemTypeEnumeration__BSDUNIX, 
                    saga::job::attributes::detail::description_operating_system_bsdunix
                },
                {   jsdl__OperatingSystemTypeEnumeration__FreeBSD, 
                    saga::job::attributes::detail::description_operating_system_freebsd
                },
                {   jsdl__OperatingSystemTypeEnumeration__NetBSD, 
                    saga::job::attributes::detail::description_operating_system_netbsd
                },
                {   jsdl__OperatingSystemTypeEnumeration__GNU_USCOREHurd, 
                    saga::job::attributes::detail::description_operating_system_gnu_hurd
                },
                {   jsdl__OperatingSystemTypeEnumeration__OS9, 
                    saga::job::attributes::detail::description_operating_system_os9
                },
                {   jsdl__OperatingSystemTypeEnumeration__MACH_USCOREKernel, 
                    saga::job::attributes::detail::description_operating_system_mach_kernel
                },
                {   jsdl__OperatingSystemTypeEnumeration__Inferno, 
                    saga::job::attributes::detail::description_operating_system_inferno
                },
                {   jsdl__OperatingSystemTypeEnumeration__QNX, 
                    saga::job::attributes::detail::description_operating_system_qnx
                },
                {   jsdl__OperatingSystemTypeEnumeration__EPOC, 
                    saga::job::attributes::detail::description_operating_system_epoc
                },
                {   jsdl__OperatingSystemTypeEnumeration__IxWorks, 
                    saga::job::attributes::detail::description_operating_system_ixworks
                },
                {   jsdl__OperatingSystemTypeEnumeration__VxWorks, 
                    saga::job::attributes::detail::description_operating_system_vxworks
                },
                {   jsdl__OperatingSystemTypeEnumeration__MiNT, 
                    saga::job::attributes::detail::description_operating_system_mint
                },
                {   jsdl__OperatingSystemTypeEnumeration__BeOS, 
                    saga::job::attributes::detail::description_operating_system_beos
                },
                {   jsdl__OperatingSystemTypeEnumeration__HP_USCOREMPE, 
                    saga::job::attributes::detail::description_operating_system_hp_mpe
                },
                {   jsdl__OperatingSystemTypeEnumeration__NextStep, 
                    saga::job::attributes::detail::description_operating_system_nextstep
                },
                {   jsdl__OperatingSystemTypeEnumeration__PalmPilot, 
                    saga::job::attributes::detail::description_operating_system_palmpilot
                },
                {   jsdl__OperatingSystemTypeEnumeration__Rhapsody, 
                    saga::job::attributes::detail::description_operating_system_rhapsody
                },
                {   jsdl__OperatingSystemTypeEnumeration__Windows_USCORE2000, 
                    saga::job::attributes::detail::description_operating_system_windows_2000
                },
                {   jsdl__OperatingSystemTypeEnumeration__Dedicated, 
                    saga::job::attributes::detail::description_operating_system_dedicated
                },
                {   jsdl__OperatingSystemTypeEnumeration__OS_USCORE390, 
                    saga::job::attributes::detail::description_operating_system_os_390
                },
                {   jsdl__OperatingSystemTypeEnumeration__VSE, 
                    saga::job::attributes::detail::description_operating_system_vse
                },
                {   jsdl__OperatingSystemTypeEnumeration__TPF, 
                    saga::job::attributes::detail::description_operating_system_tpf
                },
                {   jsdl__OperatingSystemTypeEnumeration__Windows_USCORER_USCOREMe, 
                    saga::job::attributes::detail::description_operating_system_windows_me
                },
                {   jsdl__OperatingSystemTypeEnumeration__Caldera_USCOREOpen_USCOREUNIX, 
                    saga::job::attributes::detail::description_operating_system_caldera_open_unix
                },
                {   jsdl__OperatingSystemTypeEnumeration__OpenBSD, 
                    saga::job::attributes::detail::description_operating_system_openbsd
                },
                {   jsdl__OperatingSystemTypeEnumeration__Not_USCOREApplicable, 
                    saga::job::attributes::detail::description_operating_system_not_applicable
                },
                {   jsdl__OperatingSystemTypeEnumeration__Windows_USCOREXP, 
                    saga::job::attributes::detail::description_operating_system_windows_xp
                },
                {   jsdl__OperatingSystemTypeEnumeration__z_USCOREOS, 
                    saga::job::attributes::detail::description_operating_system_z_os
                },
                {   jsdl__OperatingSystemTypeEnumeration__other,
                    saga::job::attributes::detail::description_operating_system_other
                },
                {   (jsdl__OperatingSystemTypeEnumeration)-1,
                    NULL
                },
            };

            int i = 0;
            for (/**/; NULL != operating_system[i].string; ++i)
            {
                if (ostype == operating_system[i].string)
                    break;
            } 
            if (NULL == operating_system[i].string) {
                throw std::runtime_error("SubmitJobRequest::set_OperatingSystem: "
                    "couldn't match operating system type name to a valid "
                    "enumerator value: '" + ostype + "'");
            }

            jsdl__OperatingSystem_USCOREType* os = registry_.create(
                soap_new_jsdl__OperatingSystem_USCOREType,
                soap_delete_jsdl__OperatingSystem_USCOREType);
            jsdl__OperatingSystemType_USCOREType* os_type = registry_.create(
                soap_new_jsdl__OperatingSystemType_USCOREType,
                soap_delete_jsdl__OperatingSystemType_USCOREType);

            os_type->jsdl__OperatingSystemName = operating_system[i].code;
            os->jsdl__OperatingSystemType = os_type;
            get_jsdl_resource_description()->jsdl__OperatingSystem = os;
        }

        void add_StageInStep(std::string const& source, 
            std::string const& target, bool overwrite)
        {
            jsdl__JobDescription_USCOREType* jd = get_jsdl_job_description();
            jsdl__DataStaging_USCOREType* stage = registry_.create(
                soap_new_jsdl__DataStaging_USCOREType,
                soap_delete_jsdl__DataStaging_USCOREType);

            saga::url u(source);
            stage->jsdl__FileName = u.get_path();

            stage->jsdl__CreationFlag = overwrite ? 
                jsdl__CreationFlagEnumeration__overwrite : 
                jsdl__CreationFlagEnumeration__append;

            stage->jsdl__Source = registry_.create(
                soap_new_jsdl__SourceTarget_USCOREType,
                soap_delete_jsdl__SourceTarget_USCOREType);
            stage->jsdl__Source->jsdl__URI = 
                registry_.create<std::string>(target);

            jd->jsdl__DataStaging.push_back(stage);
        }

        void add_StageOutStep(std::string const& source, 
            std::string const& target, bool overwrite)
        {
            jsdl__JobDescription_USCOREType* jd = get_jsdl_job_description();
            jsdl__DataStaging_USCOREType* stage = registry_.create(
                soap_new_jsdl__DataStaging_USCOREType,
                soap_delete_jsdl__DataStaging_USCOREType);

            saga::url u (target);
            stage->jsdl__FileName = u.get_path();

            stage->jsdl__CreationFlag = overwrite ? 
                jsdl__CreationFlagEnumeration__overwrite : 
                jsdl__CreationFlagEnumeration__append;

            stage->jsdl__Target = registry_.create(
                soap_new_jsdl__SourceTarget_USCOREType,
                soap_delete_jsdl__SourceTarget_USCOREType);
            stage->jsdl__Target->jsdl__URI = 
                registry_.create<std::string>(source);

            jd->jsdl__DataStaging.push_back(stage);
        }

        // serialize the POSIXApplication description 
        void serialize()
        {
            get_jsdl_app()->__any.clear();
            get_jsdl_app()->__any.push_back(
                serialize_to_xml(registry_, posix_app_, "jsdlposix:POSIXApplication"));
        }

    protected:
        gridsam__JobDescriptionType* get_job_description(int i = 0) const
        {
            return request_->gridsam__JobDescription[i];
        }

        jsdl__JobDescription_USCOREType* get_jsdl_job_description() const
        {
            return get_job_description()->
                jsdl__JobDefinition->jsdl__JobDescription;
        }

        jsdl__Application_USCOREType* get_jsdl_app() const
        {
            return get_jsdl_job_description()->jsdl__Application;
        }

        jsdl__Resources_USCOREType* get_jsdl_resource_description() const
        {
            return get_jsdl_job_description()->jsdl__Resources;
        }

    private:
        soap_registry registry_;
        _gridsam__submitJob* request_;
        jsdlposix__POSIXApplication_USCOREType* posix_app_;
    };

    ///////////////////////////////////////////////////////////////////////////
    class SubmitJobResponse 
    {
    public:
        SubmitJobResponse(soap* soap) 
          : registry_(soap),
            response_(registry_.create(soap_new__gridsam__submitJobResponse, 
                                       soap_delete__gridsam__submitJobResponse))
        {
        }
        ~SubmitJobResponse() {}

        _gridsam__submitJobResponse* get() { return response_; }

        // return the job-id of the submitted job
        std::string get_job_id() const
        {
            return response_->gridsam__JobIdentifier[0]->ID;
        }

    private:
        soap_registry registry_;
        _gridsam__submitJobResponse* response_;
    };

///////////////////////////////////////////////////////////////////////////////
}   // namespace util

///////////////////////////////////////////////////////////////////////////////
class JobSubmission : public JobSubmissionSOAPBindingProxy
{
private:
    typedef JobSubmissionSOAPBindingProxy base_type;
    JobSubmission* this_() { return this; }
    
public:
    JobSubmission(saga::impl::v1_0::cpi* cpi, std::string const& endpoint, 
            std::vector<saga::context> const& ctxs, std::string const& exename) 
      : endpoint_(endpoint), request_(this_(), exename), response_(this_())
    {
        this->soap_endpoint = endpoint_.c_str();

        saga::impl::exception_list exceptions;
        std::vector<saga::context>::const_iterator end = ctxs.end();
        for (std::vector<saga::context>::const_iterator it = ctxs.begin();
             it != end; ++it)
        {
            std::string certs, usercert, userkey, userpass;
            certs = retrieve_attribute(*it, saga::attributes::context_certrepository);
            usercert = retrieve_attribute(*it, saga::attributes::context_usercert);
            userkey = retrieve_attribute(*it, saga::attributes::context_userkey);
            userpass = retrieve_attribute(*it, saga::attributes::context_userpass);
            
            try {
                util::connect_to_gridsam(cpi, this, certs, usercert, 
                    userkey, userpass);
            }
            catch (saga::adaptors::exception const& e) {
                exceptions.add(e);
            }
        }
        
        if (exceptions.get_error_count())
        {
            SAGA_ADAPTOR_THROW_PLAIN_LIST(cpi, exceptions)
        }
    }

    ~JobSubmission() 
    {}

    // initialize different parts of the request 
    void set_arguments(std::vector<std::string> const& args)
    { request_.set_arguments(args); }
    void set_environment(std::map<std::string, std::string> const& env)
    { request_.set_environment(env); }
    void set_working_directory(std::string const& wd)
    { request_.set_working_directory(wd); }
    void set_stdin_file(std::string const& name)
    { request_.set_stdin_file(name); }
    void set_stdout_file(std::string const& name)
    { request_.set_stdout_file(name); }
    void set_stderr_file(std::string const& name)
    { request_.set_stderr_file(name); }

    void set_ProcessCountLimit(int count)
    { request_.set_ProcessCountLimit(count); }
    void set_ThreadCountLimit(int count)
    { request_.set_ThreadCountLimit(count); }
    void set_WallTimeLimit(int limit)
    { request_.set_WallTimeLimit(limit); }
    void set_TotalCPUCount(int count)
    { request_.set_TotalCPUCount(count); }
    void set_TotalCPUTime(int count)
    { request_.set_TotalCPUTime(count); }
    void set_CandidateHosts(std::vector<std::string> const& args)
    { request_.set_CandidateHosts(args); }
    void set_TotalPhysicalMemory(double count)
    { request_.set_TotalPhysicalMemory(count); }
    void set_CPUArchitecture(std::string const& cpuarch)
    { request_.set_CPUArchitecture(cpuarch); }
    void set_OperatingSystem(std::string const& ostype)
    { request_.set_OperatingSystem(ostype); }

    void set_job_name(std::string const& name)
    { request_.set_job_name(name); }
    void set_job_description(std::string const& desc)
    { request_.set_job_description(desc); }
    void set_job_annotation(std::vector<std::string> const& args)
    { request_.set_job_annotation(args); }
    void set_job_project(std::vector<std::string> const& args)
    { request_.set_job_project(args); }

    void add_StageInStep(std::string const& source, 
        std::string const& target, bool overwrite)
    { request_.add_StageInStep(source, target, overwrite); }
    void add_StageOutStep(std::string const& source, 
        std::string const& target, bool overwrite)
    { request_.add_StageOutStep(source, target, overwrite); }

    // do the submission
    int submitJob(std::string& job_id) 
    { 
        request_.serialize();
        int result = base_type::submitJob(request_.get(), response_.get());
        if (SOAP_OK == result)
            job_id = response_.get_job_id();
        return result;
    }

    std::string error()
    {
        char buffer[512] = { '\0' };
        soap_sprint_fault(buffer, sizeof(buffer));
        return buffer;
    }

private:
    std::string endpoint_;
    util::SubmitJobRequest request_;
    util::SubmitJobResponse response_;
};

#endif
