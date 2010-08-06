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

#include <iostream>
#include <sstream>

#include <boost/foreach.hpp>

#include <saga/impl/exception.hpp>

#include "directives_impl.hpp"
#include "pbspro_cli_staging.hpp"

namespace pbspro_job { namespace cli {

  //////////////////////////////////////////////////////////////////////
  //
  class directives_impl : public directives {
    const std::string prefix;
    std::vector<std::string> _list;
    std::vector<std::string> _pre_exe;

  public:
    directives_impl() : prefix("#PBS") {}

    void set_job_name(std::string job_name);
    void set_host(std::string& hostname);
    void set_output(std::string& path);
    void set_error(std::string& path);
    void set_environment(std::vector<std::string>& env);
    void set_working_directory(std::string path);
    void set_stagein(std::string file_list);
    void set_stageout(std::string file_list);
    void set_walltime(std::string& seconds);
    void set_job_contact(std::string& address);

    void put(std::ostream& s);
  };

  //////////////////////////////////////////////////////////////////////
  //
  void directives_impl::set_job_name(std::string job_name)
  {
    std::ostringstream os;
    os << "-N " << job_name;
    _list.push_back(os.str());
  }

  //////////////////////////////////////////////////////////////////////
  //
  void directives_impl::set_host(std::string& hostname)
  {
    std::ostringstream os;
//    os << "-l host=" << hostname;

    if(!hostname.empty()){
    	os << "-q @" << hostname;
        _list.push_back(os.str());
    }

  }

  //////////////////////////////////////////////////////////////////////
  //
  void directives_impl::set_output(std::string& path)
  {
	std::ostringstream os;

	if(!path.empty()){
		os << "-o " << path;
		_list.push_back(os.str());
	}
  }

  //////////////////////////////////////////////////////////////////////
  //
  void directives_impl::set_error(std::string& path)
  {
	std::ostringstream os;

	if(!path.empty()){
		os << "-e " << path;
		_list.push_back(os.str());
	}
  }

  //////////////////////////////////////////////////////////////////////
  //
  void directives_impl::set_environment(std::vector<std::string>& envs)
  {
    std::ostringstream os;
    os << "-v ";
    int n = envs.size();
    BOOST_FOREACH(std::string env, envs) {
      os << env;
      if (--n > 0) {
	os << ",";
      }
    }
    _list.push_back(os.str());
  }

  //////////////////////////////////////////////////////////////////////
  //
  void directives_impl::set_working_directory(std::string path)
  {
    std::ostringstream os;

    // Create working directory if it does not exist
    os.str("");
    os << "if [ -d \"" << path << "\" ]; then \n";
    os << "   echo \"Working Directory already exists\" \n";
    os << "else \n";
    os << "   echo \"Create Working Directory : " << path << "\" \n";
    os << "   /bin/mkdir -p " << path << "\n";
    os << "fi";

    _pre_exe.push_back(os.str());

    // Change directory
    os.str("");
    os << "cd " << path;
    _pre_exe.push_back(os.str());


  }

  //////////////////////////////////////////////////////////////////////
  //
  void directives_impl::set_stagein(std::string file_list)
  {
    std::ostringstream os;
    os << "-W stagein=" << file_list;
    _list.push_back(os.str());
  }

  void directives_impl::set_stageout(std::string file_list)
  {
    std::ostringstream os;
    os << "-W stageout=" << file_list;
    _list.push_back(os.str());
  }

  //////////////////////////////////////////////////////////////////////
  //
  void directives_impl::set_walltime(std::string& seconds)
  {
    std::ostringstream os;
    os << "-l walltime=" << seconds;
    _list.push_back(os.str());
  }

  //////////////////////////////////////////////////////////////////////
  //
  void directives_impl::set_job_contact(std::string& address)
  {
    std::ostringstream os;
    os << "-M " << address;
    _list.push_back(os.str());
  }

  //////////////////////////////////////////////////////////////////////
  //
  void directives_impl::put(std::ostream& s)
  {

    BOOST_FOREACH(std::string str, _list) {
    	s << prefix << " " << str << "\n";
    }

    s << "\n";

    BOOST_FOREACH(std::string str, _pre_exe) {
      s << str << "\n";
    }

  }

  //////////////////////////////////////////////////////////////////////
  //
  directives_builder_impl::directives_builder_impl(directives_checker_ptr c,
						   file_transfer_parser_ptr p)
    : checker(c), parser(p)
  {
  }

  //////////////////////////////////////////////////////////////////////
  //
  directives_builder_impl::directives_builder_impl(directives_checker_ptr c)
    : checker(c)
  {
  }

  //////////////////////////////////////////////////////////////////////
  //
  directives_ptr directives_builder_impl::create()
  {
    directives_ptr d(new directives_impl());
    d->set_job_name("saga-app");
    return d;
  }

  //////////////////////////////////////////////////////////////////////
  //
  directives_ptr directives_builder_impl::build(saga::job::description& jd,
						std::string localhost)
  {
    directives_ptr d = create();
    w = d;
    set_directives(jd, localhost);
    return d;
  }

  //////////////////////////////////////////////////////////////////////
  //
  void directives_builder_impl::set_directives(saga::job::description& jd,
					       std::string localhost)
  {
    staging_path_builder stgp_builder(localhost);

    directives_ptr p = w.lock();
    std::string wkdir;

    // WorkingDirectory
    if (jd.attribute_exists(sja::description_working_directory)) {
      std::string path = jd.get_attribute(sja::description_working_directory);

      checker->check_working_directory(path);
      stgp_builder.set_workdir(path);

      wkdir = stgp_builder.get_workdir();
      p->set_working_directory(wkdir);

    }
    else {
    	wkdir = std::getenv("HOME");
    }

    // Output File
    std::string path_output;
    if (jd.attribute_exists(sja::description_output)) {
      path_output = jd.get_attribute(sja::description_output);

      if (!path_output.empty()){
    	  // Absolute path ?
    	  if (path_output.substr(0,1) != "/"){
    	  	path_output = wkdir + "/" + path_output;
    	  }
    	  p->set_output(path_output);
      }
      else{
    	  path_output = wkdir;
    	  p->set_output(path_output);
      }

//		// Absolute path ?
//		if (path_output.substr(0,1) != "/"){
//			path_output = wkdir + "/" + path_output;
//		}
//		p->set_output(path_output);
    }
    else{
  	  path_output = wkdir;
  	  p->set_output(path_output);
    }

    // Error File
    std::string path_error;
    if (jd.attribute_exists(sja::description_error)) {
      path_error = jd.get_attribute(sja::description_error);

      if (!path_error.empty()){
    	  // Absolute path ?
    	  if (path_error.substr(0,1) != "/"){
    		  path_error = wkdir + "/" + path_error;
    	  }
    	  p->set_error(path_error);
      }
      else{
    	  path_error = wkdir;
    	  p->set_error(path_error);
      }

//		// Absolute path ?
//		if (path_error.substr(0,1) != "/"){
//			path_error = wkdir + "/" + path_error;
//		}
//
//		p->set_error(path_error);

    }
    else{
  	  path_error = wkdir;
  	  p->set_error(path_error);
    }

    // Environment
    if (jd.attribute_exists(sja::description_environment)) {

      std::vector<std::string> envs =
	jd.get_vector_attribute(sja::description_environment);
      if (envs.size() > 0) {
	BOOST_FOREACH(std::string env, envs) {
	  checker->check_environment(env);
	}
	p->set_environment(envs);
      }
    }

    // WallTimeLimit
    if (jd.attribute_exists(sja::description_wall_time_limit)) {
      std::string sec = jd.get_attribute(sja::description_wall_time_limit);
      // TODO check ?
      checker->check_walltime(sec);
      p->set_walltime(sec);
    }

    // FileTransfer
    if (jd.attribute_exists(sja::description_file_transfer)) {
      std::vector<std::string> ftlist =
	jd.get_vector_attribute(sja::description_file_transfer);

      if (!ftlist.empty() && !parser.get()) {
	// TODO
	throw;
      }

      BOOST_FOREACH(std::string spec, ftlist) {
	file_transfer_ptr ft = parser->parse(spec);
	// throw BadParameter, IncorrectURL, NotImplemented

	checker->check_file_transfer(ft);

	transfer_ptr t = ft->get_transfer();
	std::ostringstream os;
	switch (ft->get_type()) {
	case file_transfer::in:
	  os << stgp_builder.get_stagein_target(*t)
	     << "@"
	     << stgp_builder.get_stagein_source(*t);
	  p->set_stagein(os.str());
	  break;
	case file_transfer::out:
	  os << stgp_builder.get_stageout_source(*t)
	     << "@"
	     << stgp_builder.get_stageout_target(*t);
	  p->set_stageout(os.str());
	  break;
	}
      }
    }

#if 0
    // OperatingSystemName
    if (jd.attribute_exists(sja::description_operating_system_type)) {
      p->set_osnames(jd.get_vector_attribute(sja::description_operating_system_type));
    }

#endif

    // CandidateHosts
    if (jd.attribute_exists(sja::description_candidate_hosts)) {
    	std::vector<std::string> hosts =
    		jd.get_vector_attribute(sja::description_candidate_hosts);

      // FIXME
      checker->check_host(hosts[0]);
      p->set_host(hosts[0]);
    }

    // Job_Contact
    if (jd.attribute_exists(sja::description_job_contact)) {
      std::string uri = jd.get_attribute(sja::description_job_contact);
      saga::url mailto(uri);
      if (!checker->check_job_contact(mailto)) {
	SAGA_OSSTREAM strm;
	strm << "Parse failed: "
	     << "(JobContact entry: '" << uri << "').";
	SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm),
				      saga::BadParameter);
	throw;
      }
      std::string mailaddr(mailto.get_path());
      p->set_job_contact(mailaddr);
    }
  }

  //////////////////////////////////////////////////////////////////////
  //
  directives_checker_impl::~directives_checker_impl() {
    std::cout << "delete directives_checker_impl" << std::endl;
  }

  //
  bool directives_checker_impl::check_host(std::string& hostname) const {
    std::cout << "check hostname:[" << hostname << "]"<< std::endl;
    return true;
  }

  //
  bool directives_checker_impl::check_working_directory(std::string& path) const {
    std::cout << "check workdir:[" << path << "]" << std::endl;
    return true;
  }

  //
  bool directives_checker_impl::check_environment(std::string& env) const {
    std::cout << "check env:[" << env << "]" << std::endl;
    return true;
  }

  //
  bool directives_checker_impl::check_file_transfer(file_transfer_ptr ft) const {
    std::cout << "check file_transfer:[" << ft->get_source() << "]" << std::endl;
    return true;
  }

  //
  bool directives_checker_impl::check_walltime(std::string& seconds) const {
    std::cout << "check walltime:[" << seconds << "]" << std::endl;
    return true;
  }

  //
  bool directives_checker_impl::check_job_contact(saga::url& mail_uri) const {
    std::cout << "check job_contact:[" << mail_uri << "]"<< std::endl;
    return true;
  }
}}
