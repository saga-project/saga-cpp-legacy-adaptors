/*
 * Copyright (C) 2008-2009 High Energy Accelerator Research Organization (KEK)
 * Copyright (C) 2008-2009 National Institute of Informatics in Japan.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DIRECTIVES_IMPL_HPP
#define DIRECTIVES_IMPL_HPP

#include <boost/scoped_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "directives.hpp"

namespace pbspro_job { namespace cli {

  class directives_checker_impl : public directives_checker {
  public:
    //
    ~directives_checker_impl();
    //
    bool check_host(std::string& hostname) const;
    //
    bool check_working_directory(std::string& path) const;
    //
    bool check_environment(std::string& env) const;
    //
    bool check_file_transfer(file_transfer_ptr ft) const;
    //
    bool check_walltime(std::string& seconds) const;
    //
    bool check_job_contact(saga::url& mail_uri) const;
    
    // added: 07/Feb/11 by Ole Weidner
    bool check_queue(std::string& queue) const; // same as set_host ?? 
    
    // added: 07/Feb/11 by Ole Weidner
    bool check_nodes_and_ppn(std::string& number_of_nodes,  // number of nodes PBS -nodes=X:ppn=Y
                             std::string& processors_per_node ) const;
    
  };

  //////////////////////////////////////////////////////////////////////
  //
  class directives_builder_impl : public directives_builder {
    directives_checker_ptr checker;
    file_transfer_parser_ptr parser;
    boost::weak_ptr<directives> w;
    //
    directives_ptr create();
    //
    void set_directives(saga::job::description& jd, std::string localhost);

  public:
    //
    directives_builder_impl(directives_checker_ptr checker,
			    file_transfer_parser_ptr parser);
    //
    directives_builder_impl(directives_checker_ptr checker);
    //
    directives_ptr build(saga::job::description& jd, std::string localhost);
  };

}}

#endif  // DIRECTIVES_IMPL_HPP
