//  Copyright (c) 2007-2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


////////////////////////////////////////////////////////////////////////////
//
// This is an attempt to load the globus modules only *once* per application.
// the globus_init_ functions are thread-safe singleton and can be called from
// any other globus adaptors (however, they must be linked against this 
// context adaptor.
//

#ifndef SAGA_ADAPTOR_GLOBUS_LOADER_HPP
#define SAGA_ADAPTOR_GLOBUS_LOADER_HPP

namespace globus_module_loader
{
  SAGA_ADAPTOR_EXPORT void globus_init (void);                                                                
}

#endif // SAGA_ADAPTOR_GLOBUS_LOADER_HPP

