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

#include <cstdlib>
#include <fstream>

#include <boost/foreach.hpp>
#include <boost/process.hpp>
#include <boost/regex.hpp>

#include "naregi_cli.hpp"
#include "naregi_helper.hpp"
#include "wfml/wfml_writer.hpp"


namespace naregi_job { namespace cli {
  error_msg em;

  //
  output_parser::output_parser() {}

  //
  void output_parser::reset(std::string pattern)
  {
    reg = boost::regex(pattern);
  }

  //////////////////////////////////////////////////////////////////////
  //
  bool output_parser::parse_line(std::string line, std::string& data)
  {
    bool found = false;
    boost::smatch m;
    if (boost::regex_search(line, m, reg)) {
      if (m[1].matched) {
	found = true;
	data = m[1];
      }
    }
    return found;
  }

  //////////////////////////////////////////////////////////////////////
  //
  bool output_parser::parse_line(std::string line)
  {
    bool found = false;
    boost::smatch m;
    if (boost::regex_match(line, reg)) {
      found = true;
    }
    return found;
  }


  //////////////////////////////////////////////////////////////////////
  //  error_msg::error_msg()
  error_msg::error_msg()
  {
    reg_auth = boost::regex(std::string("Please sign-on again"));
    reg_id = boost::regex(std::string("Invalid job-id."));
    reg_no_file = boost::regex(std::string("NAREGI-WFML file"));
    reg_unknown = boost::regex(std::string("result code"));
  }

  //////////////////////////////////////////////////////////////////////
  //  saga::error error_msg::check(std::string msg)
  saga::error error_msg::check(std::string msg)
  {
    boost::smatch m;

    if (boost::regex_search(msg, m, reg_auth)) {
      return saga::AuthenticationFailed;
    } else if (boost::regex_search(msg, m, reg_id)) {
      return saga::BadParameter;
    } else if (boost::regex_search(msg, m, reg_no_file)) {
      return saga::NoSuccess;
    } else if (boost::regex_search(msg, m, reg_unknown)) {
      return saga::NoSuccess;
    } else {
      return saga::NoSuccess;
    }
  }

  //////////////////////////////////////////////////////////////////////
  //  execute
  //
  namespace bp = ::boost::process;

  bp::child execute(std::string command,
		    std::vector<std::string>& options,
		    std::string arg)
  {

    // FIXME third : path
    //bp::command_line cl(command, , path);
    bp::command_line cl(command);

    //  if server_certificate_verify == false
    cl.argument("-N");

    //
    BOOST_FOREACH(std::string& option, options) {
      cl.argument(option);
    }

    if (!arg.empty()) {
      cl.argument(arg);
    }

#if 0
    std::cout << "*** command line class arguments ***" << std::endl;
    BOOST_FOREACH(std::string a, cl.get_arguments()) {
      std::cout << a << std::endl;
    }
#endif

    bp::launcher l;
    l.set_stdout_behavior(bp::redirect_stream);
    l.set_stderr_behavior(bp::redirect_stream);

    bp::child c = l.start(cl);

    return c;
  }

  void error_handling(bp::pistream& err, std::ostringstream& os);

  //////////////////////////////////////////////////////////////////////
  //  naregi-simplejob-submit
  //
  //  ----------------------------------------
  //
  //  Submitting is done...
  //  JobID=CID_1319
  //
  //  ----------------------------------------
  //
  bool naregi_simplejob_submit(saga::job::description & jd,
			       std::string& id,
			       std::ostringstream& os)
  {
    using namespace saga::job::attributes;

    // executable
    std::string executable(jd.get_attribute(description_executable));

	std::vector<std::string> options;

    // candidate hosts
    if (jd.attribute_exists(description_candidate_hosts)) {
      std::string host(jd.get_vector_attribute(description_candidate_hosts).front());
      options.push_back("--host=" + host);
      std::cout << "description_candidate_hosts : " << host << std::endl;
    }

    // stdout
    std::string std_out;
	if(jd.attribute_exists(description_output)){
	  std_out = jd.get_attribute(description_output);
	  options.push_back("--output=" + std_out);
	  std::cout << "description_output : " << std_out << std::endl;
	}
	else {
	  std_out = "saga-app.o";
	  options.push_back("--output=" + std_out);
	  std::cout << "description_output (Default) : " << std_out << std::endl;
	}

	// stderr
    std::string std_err;
	if(jd.attribute_exists(description_error)){
	  std_err = jd.get_attribute(description_error);
	  options.push_back("--error=" + std_err);
	  std::cout << "description_error : " << std_err << std::endl;
	}
	else {
	  std_err = "saga-app.e";
	  options.push_back("--error=" + std_err);
	  std::cout << "description_error (Default) : " << std_err << std::endl;
	}

	// working directory
    std::string wrk_dir;
	if(jd.attribute_exists(description_working_directory)){
	  wrk_dir = jd.get_attribute(description_working_directory);
	  options.push_back("--workdir=" + wrk_dir);
	  std::cout << "description_working_directory : " << wrk_dir << std::endl;
	}
	else {
	  wrk_dir = ".";
	  options.push_back("--workdir=" + wrk_dir);
	  std::cout << "description_working_directory (Default) : " << wrk_dir << std::endl;
	}

    // arguments
    if (jd.attribute_exists(description_arguments)) {
      BOOST_FOREACH(std::string arg,
		    jd.get_vector_attribute(description_arguments)) {
    		  options.push_back("--argument=" + arg);
      }
    }


#if 0
    std::cout << "*** command line options ***" << std::endl;
    BOOST_FOREACH(std::string& arg, options) {
      std::cout << arg << std::endl;
    }
#endif

    // TODO --debug?
    // TODO --printwf?

    bp::child c = execute("naregi-simplejob-submit", options, executable);

    std::istream& stdout = c.get_stdout();

    output_parser parser;
    parser.reset("^JobID=(CID_\\d+)");

    std::string cid;
    std::string line;

    // TODO id = ""?
    while (std::getline(stdout, line)) {
      if (parser.parse_line(line, cid)) {
	id = cid;
	break;
      }
    }

    bp::status s = c.wait();
    if (s.exited() && s.exit_status() == EXIT_SUCCESS) {
      // ?
      return cid.empty() ? false : true;
    } else {
      error_handling(c.get_stderr(), os);
      return false;
    }
  }

  //////////////////////////////////////////////////////////////////////
  // naregi-job-list
  //
  //  ----------------------------------------
  //  CID_1309   program     Done       2008/12/18 19:52:47 JST  2008/12/18 19:53:58 JST
  //  ID_50991   uname       Exception  2008/10/08 13:36:37 JST  2008/10/08 13:36:46 JST
  //  ----------------------------------------
  //
  bool naregi_job_list(std::vector<std::string>& ids, std::ostringstream& os)
  {
    std::vector<std::string> options;
    options.push_back("--noheader");

    bp::child c = execute("naregi-job-list", options);

    bp::pistream& out = c.get_stdout();
    std::istream& stdout = out;

    output_parser parser;
    parser.reset("^((C)?ID_\\d+)\\s+");

    std::string line;
    std::string id;

    while (std::getline(stdout, line)) {
      if (parser.parse_line(line, id)) {
	ids.push_back(id);
      }
    }
#if 0
    BOOST_FOREACH(std::string& jobid, ids) {
      std::cout << jobid << std::endl;
    }
#endif
    bp::status s = c.wait();
    if (s.exited() && s.exit_status() == EXIT_SUCCESS) {
      return true;
    } else {
      error_handling(c.get_stderr(), os);
      return false;
    }
  }

  //////////////////////////////////////////////////////////////////////
  //  naregi-job-submit
  //
  //  ----------------------------------------
  //
  //  Submitting is done...
  //  JobID=CID_1319
  //
  //  ----------------------------------------
  //
  bool naregi_job_submit(naregi_job::wfml::workflow* wf, std::string& id,
                         std::ostringstream& os)
  {
    naregi_job::wfml::workflow_writer writer;
    writer.set_workflow(wf);

    naregi_job::helper::tempfile wfml("wfml_XXXXXX");
    wfml.get_stream() << writer;
    wfml.close();

    std::vector<std::string> options; // dummy

    bp::child c = execute("naregi-job-submit", options, wfml.get_name());

    std::istream& stdout = c.get_stdout();

    output_parser parser;
    parser.reset("^JobID=(CID_\\d+)");

    std::string cid;
    std::string line;

    // TODO id = ""?
    while (std::getline(stdout, line)) {
      if (parser.parse_line(line, cid)) {
	id = cid;
	break;
      }
    }

    bp::status s = c.wait();
    if (s.exited() && s.exit_status() == EXIT_SUCCESS) {
      // ?
      return cid.empty() ? false : true;
    } else {
      error_handling(c.get_stderr(), os);
      return false;
    }

  }

  //////////////////////////////////////////////////////////////////////
  // naregi-job-status
  //
  //  ----------------------------------------
  //
  //  uname(Done)
  //      uname#1(Done:nrgclstr037.soum.co.jp)
  //  ----------------------------------------
  //
  bool naregi_job_status(std::string id, std::string& status,
                         std::ostringstream& os)
  {
    // TODO if (id.empty()) assert?

    std::vector<std::string> options; // dummy

    bp::child c = execute("naregi-job-status", options, id);

    std::istream& stdout= c.get_stdout();

    output_parser parser;
    parser.reset("^\\S+\\(([A-Za-z]+)\\)");
    //             \S+ : exportedActivityInfo name
    // \(([A-Za-z]+)\) : NAREGI Job Status

    std::string line;
    std::string stat;

    while (std::getline(stdout, line)) {
      if (parser.parse_line(line, stat)) {
	status = stat;
	break;
      }
    }
    // TODO ? error ?

    bp::status s = c.wait();
    if (s.exited() && s.exit_status() == EXIT_SUCCESS) {
      return true;
    } else {
      error_handling(c.get_stderr(), os);
      return false;
    }
  }

  //////////////////////////////////////////////////////////////////////
  // naregi-job-cancel
  //
  //  ----------------------------------------
  //
  //  Canceling is done...
  //
  //  ----------------------------------------
  //
  //  faild.
  //  ----------------------------------------
  //  An exception occurs in canceling a job.
  //  Invalid job-id. [CID_1234] is not found.
  //  ----------------------------------------
  //
  //  TODO timeout
  //
  bool naregi_job_cancel(std::string id, std::ostringstream& os)
  {
    // TODO if (id.empty()) assert?

    std::vector<std::string> options; // dummy

    bp::child c = execute("naregi-job-cancel", options, id);

    std::istream& stdout= c.get_stdout();

    output_parser parser;
    parser.reset("^Canceling is done...");

    std::string line;
    std::string stat = "";

    while (std::getline(stdout, line)) {
      if (parser.parse_line(line)) {
        stat = "Canceled";
        break;
      }
    }
    // TODO ? error ?

    bp::status s = c.wait();
    if (s.exited() && s.exit_status() == EXIT_SUCCESS) {
      return (stat == "Canceled") ?  true : false;
    } else {
      error_handling(c.get_stderr(), os);
      return false;
    }
  }

  //////////////////////////////////////////////////////////////////////
  //  naregi-std-print command
  //
  bool naregi_std_print(std::string id,
						  std::ostringstream& std_out,
						  std::ostringstream& os)
  {
	std::vector<std::string> options; // dummy

	bp::child c = execute("naregi-std-print", options, id);

	std::istream& is = c.get_stdout();

	std::string line;
	while (std::getline(is, line)) {
		std_out << line << std::endl;
	}

	bp::status s = c.wait();
	if (s.exited() && s.exit_status() == EXIT_SUCCESS) {
	  return true;
	} else {
	  error_handling(c.get_stderr(), os);
	  return false;
	}

  }


  //////////////////////////////////////////////////////////////////////
  //  error_handling
  void error_handling(bp::pistream& err, std::ostringstream& os)
  {
    std::string line;

    while (err.good()) {
      std::getline(err, line);
      if (!line.empty()) {
	if (os.tellp() != std::ostream::pos_type(0)) {
	  os <<  '\n';
	}
	os << line;
      }
    }
  }

}}
