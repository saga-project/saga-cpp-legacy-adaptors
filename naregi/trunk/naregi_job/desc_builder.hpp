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

#ifndef DESC_BUILDER_HPP
#define DESC_BUILDER_HPP

#include <boost/shared_ptr.hpp>

#include <saga/saga/packages/job/job.hpp>

#include "wfml/debug.hpp"
#include "wfml/jsdl.hpp"

namespace sja = saga::job::attributes;
namespace sjad = saga::job::attributes::detail;

namespace naregi_job {

  void create_description(saga::job::description& jd);

  //////////////////////////////////////////////////////////////////////
  //
  //  saga::job::description => naregi_job::jsdl::description
  //
  class description_builder_impl : public jsdl::description_builder {
    saga::job::description* jd;

    jsdl::file_transfer_parser_ptr parser;

    //
    jsdl::jsdl_job_identity* create_identity();
    //
    jsdl::jsdl_application* create_application();
    //
    jsdl::jsdl_resources* create_resources();
    //
    jsdl::jsdl_data_staging* create_data_staging();

  public:
    //
    description_builder_impl();
    DESTRUCTOR(description_builder_impl);
    //
    void set_saga_job_description(saga::job::description* jd_) { jd = jd_; }
  };

}

#endif  // DESC_BUILDER_HPP
