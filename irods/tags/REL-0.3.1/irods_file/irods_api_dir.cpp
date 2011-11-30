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

#include "irods_api_dir.hpp"

namespace irods_file_adaptor { namespace api
{
	void irods_dir::copy(const std::string& irods_url_src,
						  const std::string& irods_url_tar, int flags)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::copy");
		SAGA_LOG_CRITICAL("-------------- copy file -----------------\n");

		char* src_c_url = const_cast<char*>(irods_url_src.c_str());
		char* tar_c_url = const_cast<char*>(irods_url_tar.c_str());

		int argc = 4;
		char *argv[4] = {"icp", "-r", src_c_url, tar_c_url};
		icmd.icp(argc, argv);
	}


	void irods_dir::move(const std::string& irods_url_src,
						  const std::string& irods_url_tar, int flags)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::move");
		SAGA_LOG_CRITICAL("-------------- move file -----------------\n");

		char* src_c_url = const_cast<char*>(irods_url_src.c_str());
		char* tar_c_url = const_cast<char*>(irods_url_tar.c_str());

		int argc = 3;
		char *argv[3] = {"imv", src_c_url, tar_c_url};
		icmd.imv(argc, argv);
	}


	void irods_dir::remove(const std::string& irods_url, int flags)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::remove");
		SAGA_LOG_CRITICAL("-------------- remove file -----------------\n");

		char* c_url = const_cast<char*>(irods_url.c_str());

		int argc = 3;
		char *argv[3] = {"irm", "-rf", c_url};
		icmd.irm(argc, argv);
	}

	void irods_dir::change_dir(const std::string& irods_url)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::change_dir");
		SAGA_LOG_CRITICAL("-------------- change directory -----------------\n");

		char* c_url = const_cast<char*>(irods_url.c_str());

		int argc = 2;
		char *argv[2] = {"icd", c_url};
		icmd.icd(argc, argv);
	}

	std::vector <std::string> irods_dir::list(const std::string& irods_url, std::string pattern, int flags)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::list");
		SAGA_LOG_CRITICAL("-------------- list entries -----------------\n");
		SAGA_LOG_CRITICAL("-------------- list entries (data) -----------------\n");

		std::string sql_str_data = "\"SELECT DATA_NAME  WHERE COLL_NAME = ";
		sql_str_data += "'" + irods_url + "'";
		sql_str_data += "\"";

		char* c_sql_data = const_cast<char*>(sql_str_data.c_str());
//		std::cout << "c_sql_data:" << c_sql_data << std::endl;

		int argc = 2;
		char *argv[2] = {"iquest", c_sql_data};
		std::vector <std::string> results_data;
//		std::vector <irdEnt_t*> irds_data;
		std::vector <irdEnt_t> irds_data;
		irds_data = icmd.iquest(argc, argv);

		for ( unsigned int i = 0; i < irds_data.size (); i++ )
		{
//			results_data.push_back(irds_data[i]->dataName);
			results_data.push_back(irds_data[i].dataName);
		}


		SAGA_LOG_CRITICAL("-------------- list entries (coll) -----------------\n");

		std::string sql_str_coll = "\"SELECT COLL_NAME  WHERE COLL_NAME like ";
		sql_str_coll += "'" + irods_url + "/%'";
		sql_str_coll += " AND COLL_NAME not like ";
		sql_str_coll += "'" + irods_url + "/%/%'";
		sql_str_coll += "\"";

		char* c_sql_coll = const_cast<char*>(sql_str_coll.c_str());
//		std::cout << "c_sql_coll:" << c_sql_coll << std::endl;

		int argc2 = 2;
		char *argv2[2] = {"iquest", c_sql_coll};
		std::vector <std::string> results_coll;
//		std::vector <irdEnt_t*> irds_coll;
		std::vector <irdEnt_t> irds_coll;
		irds_coll = icmd.iquest(argc2, argv2);

		for ( unsigned int i = 0; i < irds_coll.size (); i++ )
		{
//			boost::filesystem::path i_path(irds_coll[i]->collName);
			boost::filesystem::path i_path(irds_coll[i].collName);
			results_coll.push_back(i_path.leaf());
		}

		std::vector <std::string> results;
		results = results_data;
		results.insert(results.end(), results_coll.begin(), results_coll.end());

		return results;

	}

	std::vector <std::string> irods_dir::find(const std::string& irods_url,	std::string entry_pattern, int flags)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::find");

		SAGA_LOG_CRITICAL("-------------- find entries -----------------\n");

		// Replace wildcard chars
		std::string entry_pattern_buf0 = boost::regex_replace(entry_pattern, boost::regex("[%_]"), "\\\\$0");
		std::string entry_pattern_buf1 = boost::regex_replace(entry_pattern_buf0, boost::regex("[*]"), "%");
		std::string entry_pattern_sql = boost::regex_replace(entry_pattern_buf1, boost::regex("[?]"), "_");

		SAGA_LOG_CRITICAL("-------------- find entries (data) in the current dir -----------------\n");

		std::string sql_str_data_cwd = "\"SELECT DATA_NAME, COLL_NAME WHERE COLL_NAME = ";
		sql_str_data_cwd += "'" + irods_url + "'";
		sql_str_data_cwd += " AND DATA_NAME like ";
		sql_str_data_cwd += "'" + entry_pattern_sql + "'";
		sql_str_data_cwd += "\"";

		char* c_sql_data_cwd = const_cast<char*>(sql_str_data_cwd.c_str());
//		std::cout << "c_sql_data_cwd:" << c_sql_data_cwd << std::endl;

		int argc_data_cwd = 2;
		char *argv_data_cwd[2] = {"iquest", c_sql_data_cwd};
		std::vector <std::string> results_data_cwd;
		std::vector <irdEnt_t> irds_data_cwd;
		irds_data_cwd = icmd.iquest(argc_data_cwd, argv_data_cwd);

		for ( unsigned int i = 0; i < irds_data_cwd.size (); i++ )
		{
			results_data_cwd.push_back(irds_data_cwd[i].collName + "/" + irds_data_cwd[i].dataName);
		}

		SAGA_LOG_CRITICAL("-------------- find entries (data) in sub dir -----------------\n");

		std::string sql_str_data = "\"SELECT DATA_NAME, COLL_NAME WHERE COLL_NAME like ";
		sql_str_data += "'" + irods_url + "/%'";
		sql_str_data += " AND DATA_NAME like ";
		sql_str_data += "'" + entry_pattern_sql + "'";
		sql_str_data += "\"";

		char* c_sql_data = const_cast<char*>(sql_str_data.c_str());
//		std::cout << "c_sql_data:" << c_sql_data << std::endl;

		int argc_data = 2;
		char *argv_data[2] = {"iquest", c_sql_data};
		std::vector <std::string> results_data;
//		std::vector <irdEnt_t*> irds_data;
		std::vector <irdEnt_t> irds_data;
		irds_data = icmd.iquest(argc_data, argv_data);

		for ( unsigned int i = 0; i < irds_data.size (); i++ )
		{
//			results_data.push_back(irds_data[i]->collName + "/" + irds_data[i]->dataName);
			results_data.push_back(irds_data[i].collName + "/" + irds_data[i].dataName);
		}

		SAGA_LOG_CRITICAL("-------------- find directories (coll) -----------------\n");

		std::string sql_str_coll = "\"SELECT COLL_NAME WHERE COLL_NAME like ";
		sql_str_coll += "'" + irods_url + "/";
		sql_str_coll += "%" + entry_pattern_sql + "%'";
		sql_str_coll += "\"";

		char* c_sql_coll = const_cast<char*>(sql_str_coll.c_str());
//		std::cout << "c_sql_coll:" << c_sql_coll << std::endl;

		int argc_coll = 2;
		char *argv_coll[2] = {"iquest", c_sql_coll};
		std::vector <std::string> results_coll;
//		std::vector <irdEnt_t*> irds_coll;
		std::vector <irdEnt_t> irds_coll;
		irds_coll = icmd.iquest(argc_coll, argv_coll);

		for ( unsigned int i = 0; i < irds_coll.size (); i++ )
		{
//			boost::filesystem::path i_path(irds_coll[i]->collName);
//			if(i_path.leaf() == entry) results_coll.push_back(irds_coll[i]->collName + "/");
//			boost::filesystem::path i_path(irds_coll[i].collName);
//			if(i_path.leaf() == entry_pattern) results_coll.push_back(irds_coll[i].collName + "/");

//			results_coll.push_back(irds_coll[i].collName + "/");
			results_coll.push_back(irds_coll[i].collName);
		}

		std::vector <std::string> results;
		results = results_data_cwd;
		results.insert(results.end(), results_data.begin(), results_data.end());
		results.insert(results.end(), results_coll.begin(), results_coll.end());

		return results;

	}

	bool irods_dir::exists(const std::string& check_url)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::exists");

		SAGA_LOG_CRITICAL("-------------- check exists -----------------\n");
		SAGA_LOG_CRITICAL("-------------- check exists (data) -----------------\n");

		bool check = false;

		boost::filesystem::path check_url_org(check_url);
		std::string check_name = check_url_org.leaf();
		std::string check_url_bpath = check_url_org.branch_path().string();

		std::string sql_str_data = "\"SELECT DATA_NAME, COLL_NAME WHERE COLL_NAME = ";
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

		if (irds_data.size() > 0){
			check = true;
		}

		SAGA_LOG_CRITICAL("-------------- check exists (coll) -----------------\n");

		std::string sql_str_coll = "\"SELECT COLL_NAME WHERE COLL_NAME = ";
		sql_str_coll += "'" + check_url + "'";
		sql_str_coll += "\"";

		char* c_sql_coll = const_cast<char*>(sql_str_coll.c_str());
//		std::cout << "c_sql_coll:" << c_sql_coll << std::endl;

		int argc2 = 2;
		char *argv2[2] = {"iquest", c_sql_coll};
//		std::vector <irdEnt_t*> irds_coll;
		std::vector <irdEnt_t> irds_coll;
		irds_coll = icmd.iquest(argc2, argv2);

		if (irds_coll.size() > 0){
			check = true;
		}

		return check;
	}

	bool irods_dir::is_dir(const std::string& check_url)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::is_dir");

		SAGA_LOG_CRITICAL("-------------- check is_dir -----------------\n");

		bool check = false;

		std::string sql_str_coll = "\"SELECT COLL_NAME WHERE COLL_NAME = ";
		sql_str_coll += "'" + check_url + "'";
		sql_str_coll += "\"";

		char* c_sql_coll = const_cast<char*>(sql_str_coll.c_str());
//		std::cout << "c_sql_coll:" << c_sql_coll << std::endl;

		int argc = 2;
		char *argv[2] = {"iquest", c_sql_coll};
//		std::vector <irdEnt_t*> irds_coll;
		std::vector <irdEnt_t> irds_coll;
		irds_coll = icmd.iquest(argc, argv);

		if (irds_coll.size() > 0){
			check = true;
		}
		else {
			check = false;
		}

		return check;
	}

	bool irods_dir::is_entry(const std::string& check_url)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::is_entry");

		SAGA_LOG_CRITICAL("-------------- check is_entry -----------------\n");

		bool check = false;

		boost::filesystem::path check_url_org(check_url);
		std::string check_name = check_url_org.leaf();
		std::string check_url_bpath = check_url_org.branch_path().string();

		std::string sql_str_data = "\"SELECT DATA_NAME, COLL_NAME WHERE COLL_NAME = ";
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

		if (irds_data.size() > 0){
			check = true;
		}
		else {
			check = false;
		}

		return check;
	}

	bool irods_dir::is_link(const std::string& check_url)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::is_link");

		SAGA_LOG_CRITICAL("-------------- check is_link -----------------\n");

		bool check = false;	// current iRODS v2.2 does not support link.

		return check;
	}

	std::size_t irods_dir::get_num_entries(const std::string& irods_url)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::get_num_entries");

		SAGA_LOG_CRITICAL("-------------- get_num_entries -----------------\n");

		std::string pattern; int flags; //dummy

		std::vector <std::string> entry_list;
		entry_list = this->list(irods_url, pattern, flags);

		return entry_list.size();
	}

	std::string irods_dir::get_entry(const std::string& irods_url, std::size_t entry)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::get_num_entries");

		SAGA_LOG_CRITICAL("-------------- get entry -----------------\n");

		std::string pattern; int flags; //dummy

		std::vector <std::string> entry_list;
		entry_list = this->list(irods_url, pattern, flags);

		return entry_list[entry];
	}

	void irods_dir::make_dir(const std::string& irods_url, int flags)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::make_dir");

		SAGA_LOG_CRITICAL("-------------- make directory -----------------\n");

		char* c_url = const_cast<char*>(irods_url.c_str());

		int argc;
		if(flags == saga::name_space::CreateParents){
			argc = 3;
			char *argv[3] = {"imkdir", "-p", c_url};
			icmd.imkdir(argc, argv);
		}
		else {
			argc = 2;
			char *argv2[2] = {"imkdir", c_url};
			icmd.imkdir(argc, argv2);
		}

	}

	void irods_dir::open(const std::string& irods_url, int flags)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::open");

		SAGA_LOG_CRITICAL("-------------- create entry (open) -----------------\n");

		// Create an empty file as dummy
		char emp_f[256] = "/tmp/sia-empty-dummy-file";
//		char emp_f[256] = "irods2.4.tgz_0";
		FILE *fp;
		fp = fopen(emp_f, "w");
		fclose(fp);

//		system("ls /tmp");
//		sleep(1);

		char* c_url = const_cast<char*>(irods_url.c_str());
		std::cout << "c_url:" << c_url << std::endl;
		std::cout << "emp_f:" << emp_f << std::endl;

		int argc = 3;
		char *argv[3] = {"iput", emp_f, c_url};
		icmd.iput(argc, argv);

	}


	void irods_dir::open_dir(const std::string& irods_url, int flags)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::open_dir");

		SAGA_LOG_CRITICAL("-------------- create directory (open_dir) -----------------\n");

		char* c_url = const_cast<char*>(irods_url.c_str());

		int argc = 3;
		char *argv[3] = {"imkdir", "-p", c_url};
		icmd.imkdir(argc, argv);

	}

	saga::off_t irods_dir::get_size(const std::string& check_url)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::get_size");

		SAGA_LOG_CRITICAL("-------------- dir, get_size -----------------\n");

		saga::off_t data_size = 0;

		std::string sql_str_coll = "\"SELECT sum(DATA_SIZE) WHERE COLL_NAME = ";
		sql_str_coll += "'" + check_url + "'";
		sql_str_coll += "\"";

		char* c_sql_coll = const_cast<char*>(sql_str_coll.c_str());
		std::cout << "c_sql_data:" << c_sql_coll << std::endl;

		int argc = 2;
		char *argv[2] = {"iquest", c_sql_coll};
//		std::vector <irdEnt_t*> irds_coll;
		std::vector <irdEnt_t> irds_coll;
		irds_coll = icmd.iquest(argc, argv);

		if (irds_coll.size() == 1){
//			data_size = irds_coll[0]->dataSize;
			data_size = irds_coll[0].dataSize;
		}
		else {
			data_size = 0;
		}

		return data_size;

	}

	void irods_dir::meta(const std::string& meta_url,
			const std::string& meta_cmd, const std::string& meta_opt,
			const std::string& meta_key, const std::string& meta_val)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_file::meta");

		SAGA_LOG_CRITICAL("-------------- col metadata -----------------\n");

	    // remove the '/' at the end of the pathname
		std::string col_path = meta_url;
		if (col_path.rfind('/') == col_path.size()-1){
			col_path.erase(col_path.rfind('/'));
		}

		char* c_meta_url = const_cast<char*>(col_path.c_str());
		char* c_meta_cmd = const_cast<char*>(meta_cmd.c_str());
		char* c_meta_opt = const_cast<char*>(meta_opt.c_str());
		char* c_meta_key = const_cast<char*>(meta_key.c_str());
		char* c_meta_val = const_cast<char*>(meta_val.c_str());
//		std::cout << "c_meta_url=" << c_meta_url << std::endl;
//		std::cout << "c_meta_cmd=" << c_meta_cmd << std::endl;
//		std::cout << "c_meta_opt=" << c_meta_opt << std::endl;
//		std::cout << "c_meta_key=" << c_meta_key << std::endl;
//		std::cout << "c_meta_val=" << c_meta_val << std::endl;

		int argc = 6;
//		char *argv[6] = {"imeta", "ls", "-d", "hoge.txt", test_attr1, value1};
		char *argv[6] = {"imeta", c_meta_cmd, c_meta_opt, c_meta_url, c_meta_key, c_meta_val};
		icmd.imeta(argc, argv);
	}

	std::vector<std::string> irods_dir::meta_list_attr(const std::string& meta_url)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_file::meta");

		SAGA_LOG_CRITICAL("-------------- col metadata list_attr -----------------\n");

		boost::filesystem::path meta_url_org(meta_url);
		std::string meta_name = meta_url_org.leaf();
		std::string meta_url_bpath = meta_url_org.branch_path().string();

		std::string sql_str_data = "\"SELECT META_COLL_ATTR_NAME WHERE COLL_NAME = ";
		sql_str_data += "'" + meta_url_bpath + "'";
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
			result_buf = irds_meta[i].metaCollAttr;
			keys.push_back(result_buf);
		}


		return keys;

//	    // remove the '/' at the end of the pathname
//		std::string col_path = meta_url;
//		if (col_path.rfind('/') == col_path.size()-1){
//			col_path.erase(col_path.rfind('/'));
//		}
//
//		char* c_meta_url = const_cast<char*>(col_path.c_str());
////		std::cout << "c_meta_url=" << c_meta_url << std::endl;
//
//		std::vector<std::string> keys;
//		keys = icmd.imeta_list_attr(c_meta_url, COL_COLL_NAME);
//
//		return keys;
	}

	std::string irods_dir::meta_get_val(const std::string& meta_url, const std::string& attrName)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_file::meta");

		SAGA_LOG_CRITICAL("-------------- col metadata get_val -----------------\n");

		boost::filesystem::path meta_url_org(meta_url);
		std::string meta_name = meta_url_org.leaf();
		std::string meta_url_bpath = meta_url_org.branch_path().string();

		std::string sql_str_data = "\"SELECT META_COLL_ATTR_NAME, META_COLL_ATTR_VALUE WHERE COLL_NAME = ";
		sql_str_data += "'" + meta_url_bpath + "'";
		sql_str_data += " AND META_COLL_ATTR_NAME = ";
		sql_str_data += "'" + attrName + "'";
		sql_str_data += "\"";

		char* c_sql_data = const_cast<char*>(sql_str_data.c_str());
		std::cout << "c_sql_data:" << c_sql_data << std::endl;

		int argc = 2;
		char *argv[2] = {"iquest", c_sql_data};
		std::vector <std::string> results;
		std::vector <irdEnt_t> irds_meta;
		irds_meta = icmd.iquest(argc, argv);

		std::string val;
		val = irds_meta[0].metaCollVal;
		std::cout << "val:" << val << std::endl;

		return val;

//	    // remove the '/' at the end of the pathname
//		std::string col_path = meta_url;
//		if (col_path.rfind('/') == col_path.size()-1){
//			col_path.erase(col_path.rfind('/'));
//		}
//
//		char* c_meta_url = const_cast<char*>(col_path.c_str());
//		char* c_attrName = const_cast<char*>(attrName.c_str());
////		std::cout << "c_meta_url=" << c_meta_url << std::endl;
////		std::cout << "c_attrName=" << c_attrName << std::endl;
//
//		std::string val;
//		val = icmd.imeta_get_val(c_meta_url, c_attrName, COL_COLL_NAME);
//
//		return val;
	}

	bool irods_dir::meta_attr_exists(const std::string& meta_url, const std::string& attrName)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::exists");

		SAGA_LOG_CRITICAL("-------------- coll metadata attr_exists -----------------\n");

		bool check = false;

		boost::filesystem::path meta_url_org(meta_url);
		std::string meta_name = meta_url_org.leaf();
		std::string meta_url_bpath = meta_url_org.branch_path().string();

		std::string sql_str_data = "\"SELECT META_COLL_ATTR_NAME, META_COLL_ATTR_VALUE WHERE COLL_NAME = ";
		sql_str_data += "'" + meta_url_bpath + "'";
		sql_str_data += " AND META_COLL_ATTR_NAME = ";
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

	std::vector <std::string>  irods_dir::find_attr(const std::string& meta_url, const std::string& attr_pattern)
	{
		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::find_attr");

		SAGA_LOG_CRITICAL("-------------- coll metadata find_attr -----------------\n");

		// Replace wildcard chars
		std::string attr_pattern_buf0 = boost::regex_replace(attr_pattern, boost::regex("[%_]"), "\\\\$0");
		std::string attr_pattern_buf1 = boost::regex_replace(attr_pattern_buf0, boost::regex("[*]"), "%");
		std::string attr_pattern_sql = boost::regex_replace(attr_pattern_buf1, boost::regex("[?]"), "_");

		std::string sql_str_data = "\"SELECT META_COLL_ATTR_NAME, META_COLL_ATTR_VALUE WHERE COLL_NAME = ";
		sql_str_data += "'" + meta_url + "'";
		sql_str_data += " AND META_COLL_ATTR_NAME like ";
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
			results.push_back(irds_meta[i].metaCollAttr);
		}

		return results;
	}

}
}
