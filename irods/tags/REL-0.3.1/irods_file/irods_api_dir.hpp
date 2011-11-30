/*
 * Copyright (C) 2008-2011 High Energy Accelerator Research Organization (KEK)
 * Copyright (C) 2008-2011 National Institute of Informatics in Japan.
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
#ifndef __iRODS_API_DIR_HPP__
#define __iRODS_API_DIR_HPP__
#include <iostream>
#include <vector>

#include <boost/regex.hpp>

#include "irods_api.hpp"
#include <saga/saga.hpp>
#include "irods_file_adaptor_helper.hpp"

//#define BUFSIZE	10

namespace irods_file_adaptor { namespace api {

  class irods_dir {
  private:

	  icommands icmd;

  public:
    //
	irods_dir()
    {
    }

    //
    ~irods_dir()
    {
    }

	void copy(const std::string& irods_url_src,
						  const std::string& irods_url_tar, int flags);

	void move(const std::string& irods_url_src,
						  const std::string& irods_url_tar, int flags);

	void remove(const std::string& irods_url, int flags);

	void change_dir(const std::string& irods_url);

	std::vector <std::string> list(const std::string& irods_url, std::string pattern, int flags);

	std::vector <std::string> find(const std::string& irods_url, std::string entry, int flags);

	bool exists(const std::string& check_url);
	bool is_dir(const std::string& check_url);
	bool is_entry(const std::string& check_url);
	bool is_link(const std::string& check_url);

	std::size_t get_num_entries(const std::string& irods_url);
	std::string get_entry(const std::string& irods_url, std::size_t entry);

	void make_dir(const std::string& irods_url, int flags);

	void open(const std::string& irods_url, int flags);
	void open_dir(const std::string& irods_url, int flags);

	saga::off_t get_size(const std::string& check_url);

	void meta(const std::string& meta_url,
			const std::string& meta_cmd, const std::string& meta_opt,
			const std::string& meta_key, const std::string& meta_val);
	std::vector<std::string> meta_list_attr(const std::string& meta_url);
	std::string meta_get_val(const std::string& meta_url, const std::string& attrName);
	bool meta_attr_exists(const std::string& meta_url, const std::string& attrName);

	std::vector <std::string>  find_attr(const std::string& meta_url, const std::string& attr_pattern);


  };
}}

#endif  // __iRODS_API_DIR_HPP__
