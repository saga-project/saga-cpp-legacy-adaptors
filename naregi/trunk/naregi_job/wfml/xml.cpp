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


#include <boost/foreach.hpp>

#include "xml.hpp"

namespace naregi_job { namespace wfml {
  //////////////////////////////////////////////////////////////////////
  //
  std::string xml_formatter::create_tag_with_attr(std::string name,
						  std::vector<std::string>& attrs)
  {
    std::string t = name;

    BOOST_FOREACH(std::string& attr, attrs) {
      t += " " + attr;
    }

    return t;
  }

  //////////////////////////////////////////////////////////////////////
  //
  std::string xml_formatter::start_tag(std::string name)
  {
    std::string t = indent() + "<" + name + ">";
    indent_();
    return t;
  }

  //
  std::string xml_formatter::start_tag(std::string name,
				       std::vector<std::string> attrs)
  {
    std::string t = indent() + "<" + create_tag_with_attr(name, attrs) + ">";
    indent_();
    return t;
  }

  //////////////////////////////////////////////////////////////////////
  //
  std::string xml_formatter::simple_tag(std::string name, std::string value)
  {
    return indent() + "<" + name + ">" + value + "</" + name + ">";
  }

  //
  std::string xml_formatter::simple_tag(std::string name,
					std::vector<std::string> attrs)
  {
    std::string t = indent() + "<" + create_tag_with_attr(name, attrs) + " />";
    return t;
  }

  //
  std::string xml_formatter::simple_tag(std::string name,
					std::vector<std::string> attrs,
					std::string value)
  {
    std::string t = indent() + "<" + create_tag_with_attr(name, attrs) + ">";
    t += value + "</" + name + ">";

    return t;
  }

  //
  std::string xml_formatter::simple_tag(std::string name)
  {
    return indent() + "<" + name + "/>";
  }

  //////////////////////////////////////////////////////////////////////
  //
  std::string xml_formatter::end_tag(std::string name)
  {
    _indent();
    return indent() + "</" + name + ">";
  }
}}
