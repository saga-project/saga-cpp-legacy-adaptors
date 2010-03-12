//  Copyright (c) 2005-2009 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <map>
#include <vector>
#include <ctime>

#include <saga/saga/util.hpp>
#include <saga/saga/exception.hpp>
#include <saga/saga/url.hpp>
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/packages/job/adaptors/job.hpp>
#include <saga/saga/packages/job/adaptors/job_self.hpp>
#include <saga/saga/adaptors/attribute.hpp>

#include <saga/impl/config.hpp>

#include "omii_gridsam_job_service.hpp"
#include "common_helpers.hpp"

#ifndef  MAX_PATH
#define MAX_PATH _POSIX_PATH_MAX
#endif

///////////////////////////////////////////////////////////////////////////////
omii_gridsam_job_service::omii_gridsam_job_service (proxy* p, 
        cpi_info const& info, saga::ini::ini const& glob_ini, 
        saga::ini::ini const& adap_ini, TR1::shared_ptr<saga::adaptor> adaptor)
    : base_cpi (p, info, adaptor, cpi::Noflags)
{
    // check if we can handle this request
    instance_data data(this);
    std::string rm(data->rm_.get_url());

    saga::url rm_url(rm);
    std::string scheme(rm_url.get_scheme());
    if (scheme.empty() || 
        (scheme != "gridsam" && scheme != "any" && scheme != "https"))
    {
      SAGA_OSSTREAM strm;
      strm << "Could not initialize job service for [" << data->rm_ << "]. " 
           << "Only any://, gridsam:// and https:// schemes are supported.";

        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
            saga::adaptors::AdaptorDeclined);
    }
    if (rm_url.get_host().empty())
    {
      SAGA_OSSTREAM strm;
      strm << "Could not initialize job service for [" << data->rm_ << "]. " 
           << "No hostname given.";

        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
            saga::BadParameter);
    }

    // use the rm as provided
    endpoint_ = rm;
}

omii_gridsam_job_service::~omii_gridsam_job_service (void)
{
}

///////////////////////////////////////////////////////////////////////////////
void 
omii_gridsam_job_service::sync_create_job(saga::job::job& ret, saga::job::description jd)
{
    // make sure the executable path is given
    saga::attribute attr (jd);
    if (!attr.attribute_exists(saga::job::attributes::description_executable) ||
        attr.get_attribute(saga::job::attributes::description_executable).empty())
    {
        SAGA_ADAPTOR_THROW("Missing 'Executable' attribute on job description.",
            saga::BadParameter);
    }

    // get resource manager string
    saga::job::job job = 
        saga::adaptors::job(endpoint_, jd, proxy_->get_session());

    // set the created attribute
    saga::adaptors::attribute jobattr (job);
    std::time_t current = 0;
    std::time(&current);
    jobattr.set_attribute(saga::job::attributes::created, ctime(&current));

    ret = job;
}

///////////////////////////////////////////////////////////////////////////////
namespace {

    // The command line splitting below is taken from the Boost.ProgramOptions
    // library.

    // Copyright Vladimir Prus 2002-2004.
    // Distributed under the Boost Software License, Version 1.0.
    // (See accompanying file LICENSE_1_0.txt
    // or copy at http://www.boost.org/LICENSE_1_0.txt)

    // Take a command line string and splits in into tokens, according
    // to the following rules
    //
    //  - Arguments are delimited by white space, which is either a space or 
    //    a tab.
    //  - A string surrounded by double quotation marks is interpreted as a 
    //    single argument, regardless of white space contained within. A 
    //    quoted string can be embedded in an argument. 
    //  - A double quotation mark preceded by a backslash, \", is interpreted 
    //    as a literal double quotation mark (").
    //  - Backslashes are interpreted literally, unless they immediately precede 
    //    a double quotation mark.
    //
    // These rules are a slight modification of the windows command line 
    // processor rules, as described here:
    //    http://article.gmane.org/gmane.comp.lib.boost.user/3005
    //    http://msdn.microsoft.com/library/en-us/vccelng/htm/progs_12.asp
    std::vector<std::string> split_commandline (std::string const & input)
    {
        using namespace std;
        vector<string> result;

        string::const_iterator i = input.begin ();
        string::const_iterator e = input.end   ();

        for (/**/; i != e; ++i)
            if (!isspace((unsigned char)*i))
                break;
       
        if (i != e) {
            string current;
            bool inside_quoted  = false;
            int backslash_count = 0;

            for (/**/; i != e; ++i) {
                if (*i == '\\') {
                    // just count backslashes 
                    ++backslash_count;
                }
                else if (*i == '"') {
                    // '"' preceded by a backslash is a literal quote
                    // the backslash which quoted is removed
                    if (backslash_count > 0) {
                        current += '"';
                        --backslash_count;
                    }
                    else    // '"' not preceded by a backslash limits a quote
                      inside_quoted = ! inside_quoted;
                } 
                else {
                    // Not quote or backslash. All accumulated backslashes 
                    // should be added
                    if (0 != backslash_count) {
                        current.append (backslash_count, '\\');
                        backslash_count = 0;
                    }

                    if (isspace((unsigned char) *i) && !inside_quoted) {
                        // Space outside quoted section terminate the current argument
                        result.push_back (current);
                        current.resize   (0);
                
                        for (/**/; i != e && isspace((unsigned char) *i); ++i) 
                            /**/;

                        --i;
                    } 
                    else 
                        current += *i;
                }
            }

            // If we have trailing backslashes, add them
            if (0 != backslash_count)
                current.append (backslash_count, '\\');

            // If we have non-empty 'current' or we're still in quoted
            // section (even if 'current' is empty), add the last token.
            if (!current.empty() || inside_quoted)
                result.push_back (current);

            // FIXME: we SHOULD throw an exception here, non-matching quotes
            // are a BadParameter -- AM
        }
        return result;
    }

///////////////////////////////////////////////////////////////////////////////
}

void omii_gridsam_job_service::run_job_noio(saga::job::job& ret, 
                                            std::string host,
                                            std::string commandline)
{
    std::vector<std::string> cmdline = split_commandline (commandline);
    std::string executable = cmdline[0];

    saga::job::description jd;

    std::vector <std::string> chosts;
    chosts.push_back (host);
    jd.set_vector_attribute(saga::job::attributes::description_candidate_hosts, chosts);

    jd.set_attribute(saga::job::attributes::description_executable, executable);
    if (cmdline.size() > 1)
    {
        cmdline.erase(cmdline.begin());   // get rid of argv[0]
        jd.set_vector_attribute(saga::job::attributes::description_arguments, 
                                cmdline);
    }

    // set working directory
    namespace fs = boost::filesystem;
    fs::path exe(executable);     // not fs::native!
    std::string branch(exe.branch_path().string());
    if (branch.empty())
        branch = ".";
    jd.set_attribute(saga::job::attributes::description_working_directory,
                     branch);
    
    // create a new job instance
    saga::job::job job = saga::adaptors::job(endpoint_, jd, proxy_->get_session());

    // set the created attribute 
    saga::adaptors::attribute attr (job);
    std::time_t current = 0;
    std::time(&current);
    attr.set_attribute(saga::job::attributes::created, ctime(&current));
    
    job.run();    // start this job

    ret = job;
}

void omii_gridsam_job_service::sync_run_job(saga::job::job& ret, 
                                            std::string commandline,
                                            std::string host, 
                                            saga::job::ostream& in, 
                                            saga::job::istream& out, 
                                            saga::job::istream& err)
{
    std::vector<std::string> cmdline = split_commandline (commandline);
    std::string executable = cmdline[0];
    
    saga::job::description jd;

    std::vector <std::string> chosts;
    chosts.push_back (host);
    jd.set_vector_attribute(saga::job::attributes::description_candidate_hosts, chosts);

    jd.set_attribute(saga::job::attributes::description_executable, executable);
    if (cmdline.size() > 1)
    {
        cmdline.erase(cmdline.begin());   // get rid of argv[0]
        jd.set_vector_attribute(saga::job::attributes::description_arguments, 
                                cmdline);
    }
// the following is not (yet) supported by Gridsam
//     jd.set_attribute(saga::job::attributes::description_interactive, 
//         saga::attributes::common_true);

    // set working directory
    namespace fs = boost::filesystem;
    fs::path exe(executable);     // not fs::native!
    std::string branch(exe.branch_path().string());
    if (branch.empty())
        branch = ".";
    jd.set_attribute(saga::job::attributes::description_working_directory,
                     branch);
    
    // create a new job instance
    saga::job::job job = saga::adaptors::job(endpoint_, jd, proxy_->get_session());

    // set the created attribute 
    saga::adaptors::attribute attr (job);
    std::time_t current = 0;
    std::time(&current);
    attr.set_attribute(saga::job::attributes::created, ctime(&current));
    
    job.run();    // start this job

    in = job.get_stdin();
    out = job.get_stdout();
    err = job.get_stderr();

    ret = job;
}

///////////////////////////////////////////////////////////////////////////////
//  return the list of jobs created by this job_service
void omii_gridsam_job_service::sync_list(std::vector<std::string>& list_of_jobids)
{
    adaptor_data_type data(this);
    list_of_jobids = data->list_jobs();
}

void omii_gridsam_job_service::sync_get_job(saga::job::job & ret, std::string jobid)
{
    saga::job::job job = saga::adaptors::job(endpoint_, jobid, proxy_->get_session());

    // set initial attribute if this is a new job
    if (saga::job::New == job.get_state()) {
        saga::adaptors::attribute attr (job);
        std::time_t current = 0;
        std::time(&current);
        attr.set_attribute(saga::job::attributes::created, ctime(&current));
    }
    
    ret = job;
}

void omii_gridsam_job_service::sync_get_self(saga::job::self& ret)
{
    SAGA_ADAPTOR_THROW("job_service::get_self is not implemented for the OMII "
                       "gridsam adaptor.", saga::NotImplemented);
}

