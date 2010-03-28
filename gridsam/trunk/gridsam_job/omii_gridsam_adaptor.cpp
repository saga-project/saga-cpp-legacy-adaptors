//  Copyright (c) 2005-2009 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <map>
#include <vector>
#include <algorithm>

#include "stdsoap2.h"   // needs to be included first

#include <boost/assign/std/vector.hpp>
#include <boost/function_output_iterator.hpp>

#include <saga/saga/adaptors/config.hpp>
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/adaptor.hpp>

#include "omii_gridsam_adaptor.hpp"
#include "omii_gridsam_job_service.hpp"
#include "omii_gridsam_job.hpp"
#include "omii_gridsam_context.hpp"
#include "common_helpers.hpp"

SAGA_ADAPTOR_REGISTER (omii_gridsam_adaptor);

#ifndef WITH_NONAMESPACES
SOAP_NMAC struct Namespace namespaces[] = {};
#endif

///////////////////////////////////////////////////////////////////////////////
// global class used to initialize the needed environment
#if !defined(BOOST_WINDOWS)

#include <signal.h>

namespace init 
{
    // handle broken pipe signal
    class init_adaptor_sigpipe
    {
    public:
        init_adaptor_sigpipe()
        {
            handle_sigpipe = signal(SIGPIPE, sigpipe_handle); 
        }
        ~init_adaptor_sigpipe()
        {
            signal(SIGPIPE, handle_sigpipe); 
        }
        
        static void sigpipe_handle(int) {}
        
    private:
        typedef void (*SIGNAL_HANDLER)(int);
        SIGNAL_HANDLER handle_sigpipe;
    };

    init_adaptor_sigpipe init_sigpipe;
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Initialize OpenSSL
extern "C"
{
    int CRYPTO_thread_setup(void);
    void CRYPTO_thread_cleanup(void);
}

class init_adaptor_crypto
{
public:
    init_adaptor_crypto() 
    {
        // initialize gSOAP OpenSSL support
        soap_ssl_init(); 
        
        // initialize thread support in the OpenSSL crypto library
        CRYPTO_thread_setup();
    }
    ~init_adaptor_crypto()
    {
        CRYPTO_thread_cleanup();
    }
};

init_adaptor_crypto init_crypto;

///////////////////////////////////////////////////////////////////////////////
/// register function for the SAGA engine
saga::impl::adaptor_selector::adaptor_info_list_type
    omii_gridsam_adaptor::adaptor_register(saga::impl::session *s)
{
    // list of implemented cpi's
    saga::impl::adaptor_selector::adaptor_info_list_type list;

    // create preferences
    preference_type prefs; // (std::string ("security"), std::string ("none"));

    // create file adaptor infos (each adaptor instance gets its own uuid)
    // and add cpi_infos to list
    omii_gridsam_job_service::register_cpi(list, prefs, adaptor_uuid_);
    omii_gridsam_job::register_cpi(list, prefs, adaptor_uuid_);
    omii_gridsam_context::register_cpi(list, prefs, adaptor_uuid_);

    // and return list
    return (list);
}

bool omii_gridsam_adaptor::init(saga::impl::session *s, 
    saga::ini::ini const& glob_ini, saga::ini::ini const& adap_ini) 
{
    // create a default security context, if needed
    if (s->is_default_session())
    {
        // use the OMII installation directory as the default cert store
        std::string omii_root(get_omii_path(adap_ini));
        std::string omii_certs(omii_root + "/pem_format_certs/ca_certs");
        std::string omii_usercert(omii_root + "/pem_format_certs/usercert.pem");
        std::string omii_userkey(omii_root + "/pem_format_certs/userunencryptedkey.pem");

        typedef std::pair<std::string, std::string> entry_type;
        using namespace boost::assign;
        std::vector<entry_type> entries;

        entries += 
            entry_type(saga::attributes::context_type, "omii_gridsam"),
            entry_type(saga::attributes::context_usercert, omii_usercert),
            entry_type(saga::attributes::context_userkey, omii_userkey),
            entry_type(saga::attributes::context_certrepository, omii_certs)
        ;
        
        s->add_proto_context(entries);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
bool omii_gridsam_adaptor::register_job(std::string jobid, saga::job::description jd)
{
    std::pair<known_jobs_type::iterator, bool> p =
        known_jobs_.insert(known_jobs_type::value_type(jobid, jd));
    return p.second;
}

bool omii_gridsam_adaptor::unregister_job(std::string jobid)
{
    known_jobs_type::iterator it = known_jobs_.find(jobid);
    if (it == known_jobs_.end())
        return false;
        
    known_jobs_.erase(it);
    return true;
}

saga::job::description 
omii_gridsam_adaptor::get_job(omii_gridsam_job const* job, std::string jobid) const
{
    known_jobs_type::const_iterator it = known_jobs_.find(jobid);
    if (it == known_jobs_.end()) {
        SAGA_ADAPTOR_THROW_VERBATIM(job, "Nothing known about job: " + jobid, 
            saga::BadParameter);
    }
    return (*it).second;
}

///////////////////////////////////////////////////////////////////////////////
namespace {

    struct string_appender
    {
        typedef omii_gridsam_adaptor::known_jobs_type::value_type value_type;
        
        string_appender(std::vector<std::string>& v) : v_(v) {}
        void operator()(value_type v) { v_.push_back(v.first); }
        std::vector<std::string>& v_;
    };
}

std::vector<std::string> omii_gridsam_adaptor::list_jobs() const
{
    std::vector <std::string> jobids;
    std::copy(known_jobs_.begin(), known_jobs_.end(),
          boost::make_function_output_iterator(string_appender(jobids)));
    return jobids;
}

bool omii_gridsam_adaptor::knows_job(std::string jobid) const
{
    return known_jobs_.find(jobid) != known_jobs_.end();
}
