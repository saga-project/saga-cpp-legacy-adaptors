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

#include "pbspro_cli.hpp"
#include "pbspro_helper.hpp"

namespace pbspro_job { namespace cli {
  namespace bp = ::boost::process;

  void error_handling(bp::pistream& err, std::ostringstream& os);

#define RE_QSTAT "^" RE_PBS_JOBID "(\\s+(\\S+)){3}\\s+([A-Z]{1})\\s+(\\S+).*$"
#define RE_QSTATF1 "^Job Id: " RE_PBS_JOBID "$"
#define RE_QSTATF2 "^\\s+(\\S+)\\s+=\\s+(.+)$"
#define RE_QSTATF3 "^\t(.+)$"

  //////////////////////////////////////////////////////////////////////
  // qsub Constructor
  //
  qsub::qsub(std::string localhost, std::string bin_pth)
  {
	if (!bin_pth.empty()) {
		command = bin_pth + "/qsub";
		//std::cout << "bin_pth/qsub = " << command << std::endl;
	}
	else {
		bin_pth = getenv("PBS_HOME");
		if(!bin_pth.empty()){
			command = bin_pth + "/bin/qsub";
			//std::cout << "$PBS_HOME/qsub = " << command << std::endl;
		}
		else{
			command = "qsub";
		}
	}
	//std::cout << "qsub command = " << command << std::endl;

	jsbuilder = job_script_builder_ptr(new job_script_builder(localhost));

  }

  //
  bool qsub::execute(saga::job::description& jd,
		     std::string& id, std::ostringstream& os)
  {
    job_script_ptr script = jsbuilder->build(jd);

    //bp::command_line cl(command, command, path);
    bp::command_line cl(command);

    bp::launcher l;
    l.set_stdin_behavior(bp::redirect_stream);
    l.set_stdout_behavior(bp::redirect_stream);
    l.set_stderr_behavior(bp::redirect_stream);

#if 0
    std::ofstream f("test.cmd");
    f << *script << std::endl;
    f.close();
#endif

    bp::child c = l.start(cl);
    bp::postream& pos = c.get_stdin();
    pos << *script << std::endl;
    //std::cout << "test test test test test " << std::endl;
    std::cout << *script << std::endl;
    pos.close();

    bp::pistream& stdout = c.get_stdout();

    output_parser parser;
    parser.reset("^" RE_PBS_JOBID "$");

    std::string line;
    std::vector<std::string> matched;
    while (std::getline(stdout, line)) {
      matched.clear();
      if (parser.parse_line(line, matched)) {
	id = matched[0];
	// TODO matched[4] check ?
	break;
      }
    }
    stdout.close();

    bp::status s = c.wait();
    if (s.exited() && s.exit_status() == EXIT_SUCCESS) {
      // ?
      return id.empty() ? false : true;
    } else {
      error_handling(c.get_stderr(), os);
      return false;
    }

  }

  //////////////////////////////////////////////////////////////////////
  // qstat Constructor
  //
  qstat::qstat(std::string bin_pth)
  {
	if (!bin_pth.empty()) {
		command = bin_pth + "/qstat";
		//std::cout << "bin_pth/qstat = " << command << std::endl;
	}
	else {
		bin_pth = getenv("PBS_HOME");
		if(!bin_pth.empty()){
			command = bin_pth + "/bin/qstat";
			//std::cout << "$PBS_HOME/qstat = " << command << std::endl;
		}
		else{
			command = "qstat";
		}
	}
	//std::cout << "qstat command = " << command << std::endl;

  }

  //////////////////////////////////////////////////////////////////////
  //  get all PBS JobID (qstat job_id)
  //
  bool qstat::execute(std::vector<std::string>& idlist, std::ostringstream& os)
  {
    //bp::command_line cl(command, command, path);
    bp::command_line cl(command);

    bp::launcher l;
    l.set_stdout_behavior(bp::redirect_stream);
    l.set_stderr_behavior(bp::redirect_stream);

    bp::child c = l.start(cl);

    bp::pistream& stdout = c.get_stdout();

    if (!check_header(stdout)) {
      // TODO exception ?
      stdout.close();
      return false;
    }

    // no list
    if (stdout.eof()) {
      stdout.close();
      return true;
    }

    parser.reset(RE_QSTAT);

    std::string line;
    std::vector<std::string> matched;

    while (std::getline(stdout, line)) {
      matched.clear();
      if (parser.parse_line(line, matched)) {
	idlist.push_back(matched[0]);
      }
    }
    stdout.close();

    bp::status s = c.wait();
    if (s.exited() && s.exit_status() == EXIT_SUCCESS) {
      return true;
    } else {
      error_handling(c.get_stderr(), os);
      return false;
    }

  }

  //////////////////////////////////////////////////////////////////////
  //  get PBS state (qstat job_id)
  //
  bool qstat::get_state(std::string id, std::string& pbs_state,
			std::ostringstream& os)
  {
    //bp::command_line cl(command, command, path);
    bp::command_line cl(command);

    cl.argument(id);

    bp::launcher l;
    l.set_stdout_behavior(bp::redirect_stream);
    l.set_stderr_behavior(bp::redirect_stream);

    bp::child c = l.start(cl);

    bp::pistream& stdout = c.get_stdout();

    if (!check_header(stdout)) {
      // TODO exception ?
      stdout.close();
      return false;
    }

    // no list -- status deleted ?
    if (stdout.eof()) {
      pbs_state = "?"; // TODO
      stdout.close();
      return true;
    }

    parser.reset(RE_QSTAT);

    std::vector<std::string> matched;
    std::string line;

    // TODO while ?
    if (std::getline(stdout, line)) {
      stdout.close();
      if (parser.parse_line(line, matched)) {
    	pbs_state = matched[7];
      }
      else {
	// parse failed.
	// TODO exception ?
    	  return false;
      }
    }
    else {
      // read failed.
      stdout.close();
      // TODO exception ?
      return false;
    }

    bp::status s = c.wait();
    if (s.exited() && s.exit_status() == EXIT_SUCCESS) {
      return pbs_state.empty() ? false : true;
    } else {
      error_handling(c.get_stderr(), os);
      return false;
    }

  }

  //////////////////////////////////////////////////////////////////////
  // get PBS Job full status (qstat -f job_id)
  //
  jobstat_ptr qstat::get_full_status(std::string id,
				     std::ostringstream& os)
  {
    //bp::command_line cl(command, command, path);
    bp::command_line cl(command);

    cl.argument("-f");
//    cl.argument("@nrg04.cc.kek.jp");
    cl.argument(id);

    bp::launcher l;
    l.set_stdout_behavior(bp::redirect_stream);
    l.set_stderr_behavior(bp::redirect_stream);

    bp::child c = l.start(cl);

    bp::pistream& stdout = c.get_stdout();

//    if(!stdout){
//    	std::cout << "pistream is empty" << std::endl;
//    }

    jobstat_ptr fullstat = builder.create(stdout);
    stdout.close();

    bp::status s = c.wait();
    if (s.exited() && s.exit_status() == EXIT_SUCCESS) {
      return fullstat;
    } else {
      error_handling(c.get_stderr(), os);
      jobstat_ptr empty_data;
      return empty_data;
    }
  }

  //////////////////////////////////////////////////////////////////////
  // header check.
  // Job id                    Name             User            Time Use S Queue
  // ------------------------- ---------------- --------------- -------- - -----
  bool qstat::check_header(std::istream& stdout)
  {
    parser.reset("^Job id\\s+Name\\s+User\\s+Time Use\\s+S\\s+Queue$");

    std::string line;
    if (!std::getline(stdout, line)) {
      // no lines (no jobs)
      return true;
    }

    if (!parser.parse_line(line)) {
      // parse failed -- qstat ?
      return false;
    }

    // separator (skip)
    if (!std::getline(stdout, line)) {
      // header only ?
      return false;
    }

    return true;
  }

  //////////////////////////////////////////////////////////////////////
  // tracejob Constructor
  //
  ssh::ssh(void)
  {
	  command = "ssh";
  }

  //////////////////////////////////////////////////////////////////////
  //  get exit state by tracejob through ssh
  //   --- parse the results of "ssh server_host tracejob jobid"
  //
  std::string ssh::tracejob_get_exit_state(std::string id, std::string svr_name, std::ostringstream& os)
  {
	  bp::command_line cl(command);
	  cl.argument(svr_name);
	  cl.argument("tracejob");
	  cl.argument(id);

	  bp::launcher l;
	  l.set_stdout_behavior(bp::redirect_stream);
	  l.set_stderr_behavior(bp::redirect_stream);

	  bp::child c = l.start(cl);

	  bp::pistream& stdout = c.get_stdout();

	  // Find Exit_status
	  std::string exit_state;
	  boost::regex r("(Exit_status=)(\\d+)");
	  boost::smatch results;
	  std::string line;
	  while ( std::getline(stdout, line)) {
		  boost::regex_search(line, results, r);
		  if (!results.str().empty()){
			  exit_state = results.str(2);
		  }
	  }
	  stdout.close();

	  bp::status s = c.wait();
	  if (s.exited() && s.exit_status() == EXIT_SUCCESS) {
		  //std::cout << "exit_state = " << exit_state << std::endl;
		  return exit_state;
	  } else {
		  //std::cout << "exit_state is empty..." << exit_state << std::endl;
		  error_handling(c.get_stderr(), os);
		  std::string empty_str;
		  return empty_str;
	  }
  }


  //////////////////////////////////////////////////////////////////////
  //  jobstat_builder
  jobstat_builder::jobstat_builder()
  {
    parser1.reset(RE_QSTATF1);
    parser2.reset(RE_QSTATF2);
    parser3.reset(RE_QSTATF3);
  }

  //
  jobstat_ptr jobstat_builder::create(std::istream& f)
  {
    jobstat_ptr js;

    std::string line;
    std::vector<std::string> matched;

    // read "Job Id: <job id>" line
    if (std::getline(f, line)) {
      matched.clear();
      if (parser1.parse_line(line, matched)) {
	js = jobstat_ptr(new jobstat(matched[0]));
      }
    }

    if (!js.get()) {
      // invalid data ?
      return js;
    }

    // read "    <name> = <value>" line
    std::string key;
    while (std::getline(f, line)) {
      matched.clear();
      if (parser3.parse_line(line, matched)) {
	js->append_value(key, matched[0]);
      } else if (parser2.parse_line(line, matched)) {
	key = matched[0];
	js->set_entry(key, matched[1]);
      }
    }

    return js;
  }

  //////////////////////////////////////////////////////////////////////
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
  bool output_parser::parse_line(std::string line,
				 std::vector<std::string>& matched)
  {
    bool found = false;
    boost::smatch m;
    if (boost::regex_search(line, m, reg)) {
      found = true;
      for (unsigned int i = 1; i < m.size(); i++) {
	matched.push_back(m[i]);
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
