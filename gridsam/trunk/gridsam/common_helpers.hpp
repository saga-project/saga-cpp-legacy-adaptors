//  Copyright (c) 2005-2009 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(ADAPTORS_GRIDSAM_COMMON_HELPERS_HK20070910_1141AM)
#define ADAPTORS_GRIDSAM_COMMON_HELPERS_HK20070910_1141AM

#include <boost/version.hpp>
#if BOOST_VERSION >= 103800
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_actor.hpp>
#else
#include <boost/spirit/core.hpp>
#include <boost/spirit/actor/assign_actor.hpp>
#endif

#include <saga/saga/url.hpp>

#ifndef  MAX_PATH
# define MAX_PATH _POSIX_PATH_MAX
#endif

///////////////////////////////////////////////////////////////////////////////
// the jobid must have the format [gridsam://js_host/]-[job_host:jobid], where 
// jobid is the jobid assigned by the Gridsam job manager

inline bool
extract_jobid(std::string const& jobid, std::string& remote_id)
{
    using namespace boost::spirit;
    
    parse_info<> pi 
        =   parse(jobid.c_str(), 
                    str_p("[") >> *(anychar_p - ']') >> "]-[" 
                >> *(anychar_p - ':') >> ':' 
                >>  (*(anychar_p - ']'))[assign_a(remote_id)] 
                >> ']'
            );

    return pi.full;
}

///////////////////////////////////////////////////////////////////////////////
// wrapper for gethostname()
inline std::string get_hostname()
{
    char buffer[MAX_PATH] = { '\0' };
    gethostname(buffer, sizeof(buffer));
    return std::string(buffer);
}

// ensure to get a non-empty resource manager name
inline std::string ensure_resourcemanager(saga::url rm_url)
{
    std::string scheme(rm_url.get_scheme());
    if (scheme.empty() || scheme == "any" || scheme == "gridsam")
    {
        rm_url.set_scheme("https");
    }

    std::string host(rm_url.get_host());
    if (host.empty()) 
    {
        rm_url.set_host(get_hostname());
    }
    return rm_url.get_url();
}

// extract the remote host name
inline std::string ensure_hostname(std::string rm)
{
    saga::url rm_url(rm);
    
    std::string host(rm_url.get_host());
    if (host.empty()) 
        return get_hostname();

    return host;
}

///////////////////////////////////////////////////////////////////////////////
// split a key=value expression into the key and value parts
inline bool 
split_environment(std::string const& env, std::string& key, std::string & value)
{
    std::string::size_type pos = env.find_first_of("=");
    if (std::string::npos == pos)
        return false;
        
    key = env.substr(0, pos);
    value = env.substr(pos+1);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
inline std::string
get_resourcemanager(saga::ini::ini const& ini)
{
    if (ini.has_section("preferences")) {
        saga::ini::ini prefs = ini.get_section ("preferences");
        if (prefs.has_entry("connect"))
            return prefs.get_entry("connect");
    }
    return std::string();
}

///////////////////////////////////////////////////////////////////////////////
inline std::string
get_omii_path(saga::ini::ini const& ini)
{
    if (ini.has_section("preferences")) {
        saga::ini::ini prefs = ini.get_section ("preferences");
        if (prefs.has_entry("omii_client_root"))
            return prefs.get_entry("omii_client_root");
    }
    char* env = saga::safe_getenv("OMIICLIENT");
    return std::string(env ? env : "");
}

///////////////////////////////////////////////////////////////////////////
inline std::string 
retrieve_attribute(saga::context ctx, std::string const& key)
{
    if (!ctx.attribute_exists(key))
        return "";
    return ctx.get_attribute(key);
}

#endif
