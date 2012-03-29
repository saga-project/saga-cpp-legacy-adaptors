/*
 * Copyright (C) 2008-2012 High Energy Accelerator Research Organization (KEK)
 * Copyright (C) 2008-2012 National Institute of Informatics in Japan.
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

#ifndef __RNS_API_LOGICALFILE_HPP__
#define __RNS_API_LOGICALFILE_HPP__

#include "rns_api.hpp"


namespace rns_replica_adaptor { namespace api {

  class rns_logicalfile {
  private:


  public:
    //
	  rns_logicalfile()
    {
    }

    //
    ~rns_logicalfile()
    {
    }

	saga::off_t get_size(const std::string& check_url);

	void remove(const std::string& rns_url, int flags);

	void add_location(const std::string& rns_url, const std::string& loc_url);
	bool add_epr(const std::string& rns_url, const std::string& loc_url);
	bool add_url(const std::string& rns_url, const std::string& loc_url);

	// Attribute
	bool attribute_exists(const std::string& rns_url, const std::string& key);
	void set_attribute(const std::string& rns_url, const std::string& key, const std::string& val);
	std::string get_attribute(const std::string& rns_url, const std::string& key);
	std::vector<std::string> list_attributes(const std::string& rns_url);
	std::vector<std::string> list_locations(const std::string& rns_url);


//    void test();

  };
}}

#endif  // __RNS_API_LOGICALFILE_HPP__
