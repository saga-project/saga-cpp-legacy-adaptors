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


#ifndef XML_HPP
#define XML_HPP

#include <iostream>
#include <string>
#include <vector>

#include "debug.hpp"

namespace naregi_job { namespace wfml {

  //////////////////////////////////////////////////////////////////////
  //
  class xml_tag {
    std::string name;
    std::vector<std::string> attrs;

  public:
    //
    xml_tag(const std::string name) : name(name) {}
    DESTRUCTOR2(xml_tag, name);

    //
    void add_attr(const std::string& name, const std::string value) {
      // TODO value escape ?
      std::string attr = name + "=\"" + value + "\"";
      attrs.push_back(attr);
    }
    //
    std::string& get_name() { return name; }
    //
    std::vector<std::string>& get_attrs() { return attrs; }
  };

  //////////////////////////////////////////////////////////////////////
  //
  class xml_formatter {
  private:
    std::string name;
    std::string space;

    //protected:
    void indent_(void) {
      space += "  ";
    }

    //
    std::string& indent(void) {
      return space;
    }

    //
    void _indent(void) {
      space.erase(0, 2);
    }

    //
    std::string create_tag_with_attr(std::string name,
				     std::vector<std::string>& attrs);

  public:
    //
    xml_formatter() {}

    //
    DESTRUCTOR(xml_formatter);

    //
    std::string declaration(void) const {
      return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
    }

    // start_tag
    std::string start_tag(std::string name);
    std::string start_tag(std::string name,
			  std::vector<std::string> attrs);
    std::string start_tag(xml_tag& tag) {
      return start_tag(tag.get_name(), tag.get_attrs());
    }

    // simple_tag
    std::string simple_tag(std::string name);
    std::string simple_tag(std::string name, std::string value);
    std::string simple_tag(std::string name,
			   std::vector<std::string> attrs);
    std::string simple_tag(xml_tag& tag) {
      return simple_tag(tag.get_name(), tag.get_attrs());
    }
    std::string simple_tag(std::string name,
			   std::vector<std::string> attrs,
			   std::string value);
    std::string simple_tag(xml_tag& tag, std::string value) {
      return simple_tag(tag.get_name(), tag.get_attrs(), value);
    }

    // end_tag
    std::string end_tag(std::string name);
    std::string end_tag(xml_tag& tag) {
      return end_tag(tag.get_name());
    }
  };

}}

#endif  // XML_HPP
