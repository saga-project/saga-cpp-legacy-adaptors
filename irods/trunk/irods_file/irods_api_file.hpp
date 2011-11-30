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
#ifndef __iRODS_API_FILE_HPP__
#define __iRODS_API_FILE_HPP__
#include <iostream>
#include <vector>

#include <boost/regex.hpp>

#include "irods_api.hpp"

#define BUFSIZE	30

/* The possibilities for the third argument to `fseek'.
   These values should not be changed.  */
#define SEEK_SET	0	/* Seek from beginning of file.  */
#define SEEK_CUR	1	/* Seek from current position.  */
#define SEEK_END	2	/* Seek from end of file.  */

namespace irods_file_adaptor { namespace api {

  class irods_file {
  private:

	  icommands icmd;

  public:
    //
    irods_file()
    {
    }

    //
    ~irods_file()
    {
    }

	void copy(const std::string& irods_url_src,
						  const std::string& irods_url_tar, int flags);

	void move(const std::string& irods_url_src,
						  const std::string& irods_url_tar, int flags);

	void remove(const std::string& irods_url_src, int flags);
	void meta(const std::string& meta_url,
			const std::string& meta_cmd, const std::string& meta_opt,
			const std::string& meta_key, const std::string& meta_val);
	std::vector<std::string> meta_list_attr(const std::string& meta_url);
	std::string meta_get_val(const std::string& meta_url, const std::string& attrName);
	std::vector<std::string> meta_list_locations(const std::string& meta_url);
	bool meta_attr_exists(const std::string& meta_url, const std::string& attrName);

	saga::off_t get_size(const std::string& check_url);

	std::size_t read (const std::string&  read_url, char *buf, std::size_t size, off_t offset, int seek_mode);
	std::size_t write(const std::string& write_url, char *buf, std::size_t size, off_t offset, int seek_mode);
	saga::off_t seek(const std::string& seek_url, off_t offset, int seek_mode);

	std::vector <std::string>  find_attr(const std::string& meta_url, const std::string& attr_pattern);

  };
}}

#endif  // __iRODS_API_FILE_HPP__
