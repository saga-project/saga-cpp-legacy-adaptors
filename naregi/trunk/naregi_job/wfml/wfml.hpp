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


#ifndef WFML_HPP
#define WFML_HPP

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "writer.hpp"
#include "xml.hpp"

namespace naregi_job { namespace wfml {

  namespace element {
    char const* const definitions = "definitions";

    char const* const activity_model = "activityModel";
    char const* const activity = "activity";
    char const* const jsdl = "jsdl";
    char const* const mkdir = "mkdir";
    char const* const transfer = "transfer";
    char const* const source = "source";
    char const* const target = "target";

    char const* const composition_model = "compositionModel";
    char const* const import_model = "importModel";
    char const* const export_model = "exportModel";
    char const* const exported_activity = "exportedActivity";
    char const* const exported_activity_info = "exportedActivityInfo";
    char const* const control_model = "controlModel";
    char const* const control_link = "controlLink";
  }

  namespace attribute {
    //
    char const* const name = "name";
    // mkdir, transfer
    char const* const wall_time_limit = "WallTimeLimit";
    // controlModel
    char const* const control_in = "controlIn";
    // controlLink
    char const* const label = "label";
    char const* const source = "source";
    char const* const target = "target";
  }

  typedef boost::shared_ptr<xml_formatter> xml_ptr;
  typedef boost::scoped_ptr<xml_tag> xml_tag_ptr;

  //////////////////////////////////////////////////////////////////////
  // xml_writer
  class xml_writer {

  protected:
    xml_ptr xml;

  public:
    //
    xml_writer() {
      xml_ptr a(new xml_formatter());
      xml = a;
    }
    //
    xml_writer(xml_ptr a) : xml(a) {};
  };

}}

#endif  // WFML_HPP
