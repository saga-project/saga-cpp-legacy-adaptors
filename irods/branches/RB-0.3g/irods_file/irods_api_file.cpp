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
#include <sstream>

#include <saga/saga/exception.hpp>

#include "irods_api_file.hpp"

namespace irods_file_adaptor { namespace api
{
	void irods_file::copy(const std::string& irods_url_src,
						  const std::string& irods_url_tar, int flags)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_file::copy");

		SAGA_LOG_CRITICAL("-------------- copy file -----------------\n");

		char* src_c_url = const_cast<char*>(irods_url_src.c_str());
		char* tar_c_url = const_cast<char*>(irods_url_tar.c_str());

		int argc = 3;
		char *argv[3] = {"icp", src_c_url, tar_c_url};
		icmd.icp(argc, argv);
	}


	void irods_file::move(const std::string& irods_url_src,
						  const std::string& irods_url_tar, int flags)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_file::move");

		SAGA_LOG_CRITICAL("-------------- move file -----------------\n");

		char* src_c_url = const_cast<char*>(irods_url_src.c_str());
		char* tar_c_url = const_cast<char*>(irods_url_tar.c_str());

		int argc = 3;
		char *argv[3] = {"imv", src_c_url, tar_c_url};
		icmd.imv(argc, argv);
	}


	void irods_file::remove(const std::string& irods_url, int flags)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_file::remove");

		SAGA_LOG_CRITICAL("-------------- remove file -----------------\n");

		char* c_url = const_cast<char*>(irods_url.c_str());

		int argc = 2;
		char *argv[2] = {"irm", c_url};
		icmd.irm(argc, argv);
	}

	saga::off_t irods_file::get_size(const std::string& check_url)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_file::get_size");

		SAGA_LOG_CRITICAL("-------------- get_size -----------------\n");

		saga::off_t data_size = 0;

		boost::filesystem::path check_url_org(check_url);
		std::string check_name = check_url_org.leaf();
		std::string check_url_bpath = check_url_org.branch_path().string();

		std::string sql_str_data = "\"SELECT DATA_NAME, DATA_SIZE WHERE COLL_NAME = ";
		sql_str_data += "'" + check_url_bpath + "'";
		sql_str_data += " AND DATA_NAME = ";
		sql_str_data += "'" + check_name + "'";
		sql_str_data += "\"";

		char* c_sql_data = const_cast<char*>(sql_str_data.c_str());
//		std::cout << "c_sql_data:" << c_sql_data << std::endl;

		int argc = 2;
		char *argv[2] = {"iquest", c_sql_data};
//		std::vector <irdEnt_t*> irds_data;
		std::vector <irdEnt_t> irds_data;
		irds_data = icmd.iquest(argc, argv);

		if (irds_data.size() == 1){
//			data_size = irds_data[0]->dataSize;
			data_size = irds_data[0].dataSize;
		}
		else {
			data_size = 0;
		}

		return data_size;

	}


	std::size_t irods_file::read(const std::string& read_url, char *buf, std::size_t size, off_t offset, int seek_mode)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_file::read");

		SAGA_LOG_CRITICAL("--------------read file -----------------\n");

		char* c_read_data = const_cast<char*>(read_url.c_str());

//		std::cout << "c_read_data=" << c_read_data << std::endl;
//		std::cout << "size=" << size << std::endl;
//		std::cout << "offset=" << offset << std::endl;
//		std::cout << "seek_mode=" << seek_mode << std::endl;

		icmd.iread(c_read_data, buf, size, offset, seek_mode);

		return size;
//		return BUFSIZE;
	}

	std::size_t irods_file::write(const std::string& write_url, char *buf, std::size_t size, off_t offset, int seek_mode)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_file::write");

		SAGA_LOG_CRITICAL("--------------write file -----------------\n");

		char* c_write_data = const_cast<char*>(write_url.c_str());

//		std::cout << "c_write_data=" << c_write_data << std::endl;
//		std::cout << "size=" << size << std::endl;
//		std::cout << "offset=" << offset << std::endl;
//		std::cout << "seek_mode=" << seek_mode << std::endl;

		icmd.iwrite(c_write_data, buf, size, offset, seek_mode);

		return size;
//		return BUFSIZE;
	}

//	saga::off_t irods_file::seek(const std::string& seek_url, off_t offset, int seek_mode)
//	{
//		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_file::seek");
//
//		SAGA_LOG_CRITICAL("--------------seek file -----------------\n");
//
//		saga::off_t seek_ptr = 0;
//		rcComm_t *conn;
//
//		char* c_seek_data = const_cast<char*>(seek_url.c_str());
//
//		std::cout << "c_seek_data=" << c_seek_data << std::endl;
//		std::cout << "offset=" << offset << std::endl;
//		std::cout << "seek_mode=" << seek_mode << std::endl;
//
//		conn = icmd.iconnect();
//		icmd.iseek(conn, c_seek_data, offset, seek_mode);
//
//		return seek_ptr;
//	}

	void irods_file::meta(const std::string& meta_url,
			const std::string& meta_cmd, const std::string& meta_opt,
			const std::string& meta_key, const std::string& meta_val)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_file::meta");

		SAGA_LOG_CRITICAL("-------------- metadata -----------------\n");

		char* c_meta_url = const_cast<char*>(meta_url.c_str());
		char* c_meta_cmd = const_cast<char*>(meta_cmd.c_str());
		char* c_meta_opt = const_cast<char*>(meta_opt.c_str());
		char* c_meta_key = const_cast<char*>(meta_key.c_str());
		char* c_meta_val = const_cast<char*>(meta_val.c_str());
//		std::cout << "c_meta_url=" << c_meta_url << std::endl;
//		std::cout << "c_meta_cmd=" << c_meta_cmd << std::endl;
//		std::cout << "c_meta_opt=" << c_meta_opt << std::endl;
//		std::cout << "c_meta_key=" << c_meta_key << std::endl;
//		std::cout << "c_meta_val=" << c_meta_val << std::endl;

//		SAGA_LOG_CRITICAL("c_meta_url=%s \n", c_meta_url);
//		SAGA_LOG_CRITICAL("c_meta_cmd=%s \n", c_meta_cmd);
//		SAGA_LOG_CRITICAL("c_meta_opt=%s \n", c_meta_opt);
//		SAGA_LOG_CRITICAL("c_meta_key=%s \n", c_meta_key);
//		SAGA_LOG_CRITICAL("c_meta_val=%s \n", c_meta_val);

		int argc = 6;
//		char *argv[6] = {"imeta", "ls", "-d", "hoge.txt", test_attr1, value1};
		char *argv[6] = {"imeta", c_meta_cmd, c_meta_opt, c_meta_url, c_meta_key, c_meta_val};
		icmd.imeta(argc, argv);
	}

	std::vector<std::string> irods_file::meta_list_attr(const std::string& meta_url)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_file::meta");

		SAGA_LOG_CRITICAL("-------------- file metadata list_attr -----------------\n");

		boost::filesystem::path meta_url_org(meta_url);
		std::string meta_name = meta_url_org.leaf();
		std::string meta_url_bpath = meta_url_org.branch_path().string();

		std::string sql_str_data = "\"SELECT META_DATA_ATTR_NAME WHERE COLL_NAME = ";
		sql_str_data += "'" + meta_url_bpath + "'";
		sql_str_data += " AND DATA_NAME = ";
		sql_str_data += "'" + meta_name + "'";
		sql_str_data += "\"";

		char* c_sql_data = const_cast<char*>(sql_str_data.c_str());
//		std::cout << "c_sql_data:" << c_sql_data << std::endl;

		int argc = 2;
		char *argv[2] = {"iquest", c_sql_data};
		std::vector <std::string> keys;
		std::vector <irdEnt_t> irds_meta;
		irds_meta = icmd.iquest(argc, argv);

		for ( unsigned int i = 0; i < irds_meta.size (); i++ )
		{
			std::string result_buf;
			result_buf = irds_meta[i].metaDataAttr;
			keys.push_back(result_buf);
		}


		return keys;


//		char* c_meta_url = const_cast<char*>(meta_url.c_str());
////		std::cout << "c_meta_url=" << c_meta_url << std::endl;
//
//		std::vector<std::string> keys;
//		keys = icmd.imeta_list_attr(c_meta_url, COL_DATA_NAME);
//
//		return keys;
	}

	std::string irods_file::meta_get_val(const std::string& meta_url, const std::string& attrName)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_file::meta");

		SAGA_LOG_CRITICAL("-------------- file metadata get_val -----------------\n");

		boost::filesystem::path meta_url_org(meta_url);
		std::string meta_name = meta_url_org.leaf();
		std::string meta_url_bpath = meta_url_org.branch_path().string();

		std::string sql_str_data = "\"SELECT META_DATA_ATTR_NAME, META_DATA_ATTR_VALUE WHERE COLL_NAME = ";
		sql_str_data += "'" + meta_url_bpath + "'";
		sql_str_data += " AND DATA_NAME = ";
		sql_str_data += "'" + meta_name + "'";
		sql_str_data += " AND META_DATA_ATTR_NAME = ";
		sql_str_data += "'" + attrName + "'";
		sql_str_data += "\"";

		char* c_sql_data = const_cast<char*>(sql_str_data.c_str());
//		std::cout << "c_sql_data:" << c_sql_data << std::endl;

		int argc = 2;
		char *argv[2] = {"iquest", c_sql_data};
		std::vector <std::string> results;
		std::vector <irdEnt_t> irds_meta;
		irds_meta = icmd.iquest(argc, argv);

		std::string val;
		val = irds_meta[0].metaDataVal;
//		std::cout << "val:" << val << std::endl;

		return val;
	}

	std::vector<std::string> irods_file::meta_list_locations(const std::string& meta_url)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_file::meta");

		SAGA_LOG_CRITICAL("-------------- metadata list_locations -----------------\n");


		boost::filesystem::path meta_url_org(meta_url);
		std::string meta_name = meta_url_org.leaf();
		std::string meta_url_bpath = meta_url_org.branch_path().string();

		std::string sql_str_data = "\"SELECT RESC_LOC, DATA_PATH WHERE COLL_NAME = ";
		sql_str_data += "'" + meta_url_bpath + "'";
		sql_str_data += " AND DATA_NAME = ";
		sql_str_data += "'" + meta_name + "'";
		sql_str_data += "\"";

		char* c_sql_data = const_cast<char*>(sql_str_data.c_str());
//		std::cout << "c_sql_data:" << c_sql_data << std::endl;

		int argc = 2;
		char *argv[2] = {"iquest", c_sql_data};
		std::vector <std::string> results;
		std::vector <irdEnt_t> irds_data;
		irds_data = icmd.iquest(argc, argv);

		for ( unsigned int i = 0; i < irds_data.size (); i++ )
		{
			std::string result_buf;
			result_buf = irds_data[i].rescLoc + irds_data[i].dataPath;
			results.push_back(result_buf);
		}

		return results;

	}

	bool irods_file::meta_attr_exists(const std::string& meta_url, const std::string& attrName)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::exists");

		SAGA_LOG_CRITICAL("-------------- file metadata attr_exists -----------------\n");

		bool check = false;

		boost::filesystem::path meta_url_org(meta_url);
		std::string meta_name = meta_url_org.leaf();
		std::string meta_url_bpath = meta_url_org.branch_path().string();

		std::string sql_str_data = "\"SELECT META_DATA_ATTR_NAME, META_DATA_ATTR_VALUE WHERE COLL_NAME = ";
		sql_str_data += "'" + meta_url_bpath + "'";
		sql_str_data += " AND DATA_NAME = ";
		sql_str_data += "'" + meta_name + "'";
		sql_str_data += " AND META_DATA_ATTR_NAME = ";
		sql_str_data += "'" + attrName + "'";
		sql_str_data += "\"";

		char* c_sql_data = const_cast<char*>(sql_str_data.c_str());
//		std::cout << "c_sql_data:" << c_sql_data << std::endl;

		int argc = 2;
		char *argv[2] = {"iquest", c_sql_data};
		std::vector <std::string> results;
		std::vector <irdEnt_t> irds_meta;
		irds_meta = icmd.iquest(argc, argv);

		if (irds_meta.size() > 0){
			check = true;
		}

		return check;
	}

	std::vector <std::string>  irods_file::find_attr(const std::string& meta_url, const std::string& attr_pattern)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::find_attr");

		SAGA_LOG_CRITICAL("-------------- file metadata find_attr -----------------\n");

		// Replace wildcard chars
		std::string attr_pattern_buf0 = boost::regex_replace(attr_pattern, boost::regex("[%_]"), "\\\\$0");
		std::string attr_pattern_buf1 = boost::regex_replace(attr_pattern_buf0, boost::regex("[*]"), "%");
		std::string attr_pattern_sql = boost::regex_replace(attr_pattern_buf1, boost::regex("[?]"), "_");

		boost::filesystem::path meta_url_org(meta_url);
		std::string meta_name = meta_url_org.leaf();
		std::string meta_url_bpath = meta_url_org.branch_path().string();

		std::string sql_str_data = "\"SELECT META_DATA_ATTR_NAME, META_DATA_ATTR_VALUE WHERE COLL_NAME = ";
		sql_str_data += "'" + meta_url_bpath + "'";
		sql_str_data += " AND DATA_NAME = ";
		sql_str_data += "'" + meta_name + "'";
		sql_str_data += " AND META_DATA_ATTR_NAME like ";
		sql_str_data += "'" + attr_pattern_sql + "'";
		sql_str_data += "\"";

		char* c_sql_data = const_cast<char*>(sql_str_data.c_str());
//		std::cout << "c_sql_data:" << c_sql_data << std::endl;

		int argc = 2;
		char *argv[2] = {"iquest", c_sql_data};
		std::vector <std::string> results;
		std::vector <irdEnt_t> irds_meta;
		irds_meta = icmd.iquest(argc, argv);

		for ( unsigned int i = 0; i < irds_meta.size (); i++ )
		{
			results.push_back(irds_meta[i].metaDataAttr);
		}

		return results;
	}
}
}
