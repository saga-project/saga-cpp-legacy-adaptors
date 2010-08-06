//  Copyright (c) 2007-2008 Ole Weidner (oweidner@cct.lsu.edu)
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <saga/saga.hpp>
#include <saga/saga/adaptors/adaptor.hpp>

#include <boost/spirit/core/non_terminal/impl/static.hpp>

#include "../config/saga-globus-config.hpp"

#ifdef SAGA_HAVE_GLOBUS_GRAM
# include <globus_gram_client.h>
#endif

#ifdef SAGA_HAVE_GLOBUS_GRIDFTP
# include <globus_ftp_client.h>
#endif

#ifdef SAGA_HAVE_GLOBUS_RLS
# include <globus_rls_client.h>
#endif

#ifdef SAGA_HAVE_GLOBUS_GSI
# include <globus_gss_assist.h>
#endif

#ifdef SAGA_HAVE_GLOBUS_GASS
# include <globus_gass_copy.h>
#endif

#include <globus_common.h>
#include <globus_common_include.h>
#include "globus_global_loader.hpp"

namespace globus_module_loader
{
  // the singleton class
  class globus_init_singleton
  {
    public:
      // this c'tor is only called once, and is thread safe
      globus_init_singleton (void)
      {
        globus_module_activate (GLOBUS_COMMON_MODULE);
          
        #ifdef SAGA_HAVE_GLOBUS_GRAM
          SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_DEBUG) { std::cout << "globus_loader: activate GRAM" << std::endl; }
          globus_module_activate (GLOBUS_GRAM_CLIENT_MODULE);
        #endif
        
        #ifdef SAGA_HAVE_GLOBUS_GRIDFTP
          SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_DEBUG) { std::cout << "globus_loader: activate FTP" << std::endl; }
          globus_module_activate (GLOBUS_XIO_MODULE);
          globus_module_activate (GLOBUS_FTP_CLIENT_MODULE);
        #endif
        
        #ifdef SAGA_HAVE_GLOBUS_RLS
          SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_DEBUG) { std::cout << "globus_loader: activate RLS" << std::endl; }
          globus_module_activate (GLOBUS_RLS_CLIENT_MODULE);
        #endif
        
        #ifdef SAGA_HAVE_GLOBUS_GSI
          SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_DEBUG) { std::cout << "globus_loader: activate GSI" << std::endl; }
          globus_module_activate (GLOBUS_GSI_GSS_ASSIST_MODULE);
        #endif
        
        #ifdef SAGA_HAVE_GLOBUS_GASS
          SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_DEBUG) { std::cout << "globus_loader: activate GASS" << std::endl; }
          globus_module_activate (GLOBUS_GASS_COPY_MODULE);
        #endif
      }
  };

  struct globus_init_singleton_tag {};

  // avoid to be optimized away
  globus_init_singleton & get_singleton (void)
  {
    boost::spirit::static_ <globus_init_singleton, globus_init_singleton_tag> globus_init_singleton_;
    return globus_init_singleton_.get ();
  }

  void globus_init (void)
  {
    get_singleton ();  // ignore return value
  }
}

