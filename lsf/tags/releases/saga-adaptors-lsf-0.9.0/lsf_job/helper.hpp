//  Copyright (c) 2008 João Abecasis
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef SAGA_ADAPTORS_LSF_JOB_HELPER_HPP_INCLUDED
#define SAGA_ADAPTORS_LSF_JOB_HELPER_HPP_INCLUDED

#include <saga/saga/url.hpp>
#include <saga/saga/packages/job/job_description.hpp>
#include <saga/saga/adaptors/file_transfer_spec.hpp>
#include <saga/impl/exception.hpp>


#include <boost/version.hpp>
#if BOOST_VERSION >= 103800
#include <boost/spirit/include/classic_debug.hpp>
#include <boost/spirit/include/classic_parser_names.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_utility.hpp>
#else
#include <boost/spirit/debug.hpp>
#include <boost/spirit/debug/parser_names.hpp>
#include <boost/spirit/core/non_terminal/grammar.hpp>
#include <boost/spirit/core/non_terminal/rule.hpp>
#include <boost/spirit/core/primitives/primitives.hpp>
#include <boost/spirit/core/composite/actions.hpp>
#include <boost/spirit/core/composite/directives.hpp>
#include <boost/spirit/core/composite/epsilon.hpp>
#include <boost/spirit/core/composite/kleene_star.hpp>
#include <boost/spirit/core/composite/sequence.hpp>
#include <boost/spirit/utility/chset.hpp>
#endif



#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/regex.hpp>

#define DBG_PRFX    "Platform LSF Adaptor: "

namespace saga { namespace adaptors { namespace lsf { namespace detail {

  inline bool is_lsf_or_any_scheme(std::string jobid)
  {
    bool is_lsf_or_any_scheme = false;
    
    size_t pos = jobid.find("any://");
    if(!(pos == std::string::npos))
      is_lsf_or_any_scheme = true;

    pos = jobid.find("lsf://");
    if(!(pos == std::string::npos))
      is_lsf_or_any_scheme = true;
        
    return is_lsf_or_any_scheme;
  }
  
  
  inline std::string saga_to_native_jobid(std::string jobid)
  {
    size_t pos = jobid.find("]-[");
    if((pos == std::string::npos))
    {
      SAGA_OSSTREAM strm;
      strm << "Malformed jobid: " << jobid << ". The format has to be"
      << "`[backend url]-[native id]`.";
      SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
    }
    std::string native_jobid = jobid.substr(pos+3);
    native_jobid.erase(native_jobid.length()-1);
  
    return native_jobid;
  }
  
///////////////////////////////////////////////////////////////////////////////
// splits command line arguments into a string vector 
//
inline std::vector <std::string> split_cmdline (std::string const & input)
{
  using namespace std;
  vector <string> result;
  
  string::const_iterator i = input.begin ();
  string::const_iterator e = input.end   ();
  
  for ( /**/ ; i != e; ++i )
    if ( ! isspace ((unsigned char)*i) )
      break;
  
  if ( i != e ) 
  {
    string current;
    bool   inside_quoted   = false;
    int    backslash_count = 0;
    
    for (/**/; i != e; ++i) 
    {
      if (*i == '\\') 
      {
        // just count backslashes 
        ++backslash_count;
      }
      else if ( *i == '"' ) 
      {
        // '"' preceded by a backslash is a literal quote
        // the backslash which quoted is removed
        if ( backslash_count > 0 ) 
        {
          current += '"';
          --backslash_count;
        }
        // '"' not preceded by a backslash limits a quote
        else 
        {
          inside_quoted = ! inside_quoted;
        }
      } 
      else 
      {
        // Not quote or backslash. All accumulated backslashes should be
        // added
        if ( backslash_count ) 
        {
          current.append (backslash_count, '\\');
          backslash_count = 0;
        }
        
        if ( isspace ((unsigned char) *i) && ! inside_quoted ) 
        {
          // Space outside quoted section terminate the current argument
          result.push_back (current);
          current.resize   (0);
          
          for ( /**/ ; i != e && isspace ((unsigned char) *i); ++i ) 
          /**/;
          
          --i;
        } 
        else 
        {                  
          current += *i;
        }
      }
    }
    
    // If we have trailing backslashes, add them
    if ( backslash_count )
    {
      current.append (backslash_count, '\\');
    }
    
    // If we have non-empty 'current' or we're still in quoted
    // section (even if 'current' is empty), add the last token.
    if ( ! current.empty () || inside_quoted )
    {
      result.push_back (current);        
    }
    // FIXME: we SHOULD thrown an exception here, non-matching quotes
    // are a BadParameter -- AM
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////
// the jobid must have the format [scheme://js_host/]-[job_host:jobid], where 
// jobid is the jobid assigned by the Gridsam job manager

inline bool
extract_jobid(std::string const& jobid, std::string& remote_id)
{
    using namespace boost::spirit;

    boost::spirit::parse_info<> pi
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
    char buffer[512] = { '\0' };
    gethostname(buffer, sizeof(buffer));
    return std::string(buffer);
}

}}}}

#endif
