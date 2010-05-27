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


#ifndef WORKFLOW_HPP
#define WORKFLOW_HPP

#include <iostream>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "debug.hpp"
#include "jsdl.hpp"
#include "wfml.hpp"

namespace naregi_job { namespace wfml {

  using namespace naregi_job::jsdl;

  namespace activity_type {
    char const* const transfer = "transfer";
  }

  //////////////////////////////////////////////////////////////////////
  //
  class activity {
    int n;
    std::string name;

  protected:
    //
    void set_name(std::string name_) {
      std::ostringstream ost;
      ost << name_ << "#" << n;
      name = ost.str();
    }

  public:
    activity(int n) : n(n) {}
    DESTRUCTOR(activity);
    //
    std::string get_name() const { return name; }
  };

  //////////////////////////////////////////////////////////////////////
  //
  class jsdl_activity : public activity {
    mutable std::string nodename;
    description desc;

  public:
    jsdl_activity(int n) : activity(n) { set_name("program"); }
    DESTRUCTOR(jsdl_activity);
    //
    description& get_desc() { return desc; }

    //
    void set_desc(description d) {
      desc = d;
    }

    //
    std::string get_nodename(void) const;
  };

  //////////////////////////////////////////////////////////////////////
  //
  class mkdir_activity : public activity {
    std::string wall_time_limit;
    std::string target;

  public:
    mkdir_activity(int n) : activity(n),
			    wall_time_limit("300")
    {
      set_name("mkdir");
    }
    DESTRUCTOR(mkdir_activity);
    //
    std::string get_wall_time_limit() { return wall_time_limit; }
    std::string get_target() { return target; }
    //
    void set_wall_time_limit(std::string value) {
      wall_time_limit = value;
    }
    void set_target(std::string d) {
      target = d;
    }
  };

  //////////////////////////////////////////////////////////////////////
  //
  class transfer_activity : public activity {
    std::string wall_time_limit;
    transfer t;

  public:
    transfer_activity(int n) : activity(n),
			       wall_time_limit("300")
    {
      set_name("transfer");
    }
    DESTRUCTOR(transfer_activity);
    //
    std::string get_wall_time_limit() { return wall_time_limit; }
    transfer get_transfer() { return t; }
    std::string get_source() { return t.first; }
    std::string get_target() { return t.second; }
    //
    void set_wall_time_limit(std::string value) {
      wall_time_limit = value;
    }
    void set_transfer(std::string source, std::string target) {
      t = transfer(source, target);
    }
  };

  typedef_ptr(activity);
  typedef_ptr(jsdl_activity);
  typedef_ptr(mkdir_activity);
  typedef_ptr(transfer_activity);

  //////////////////////////////////////////////////////////////////////
  //
  class activity_factory {
    int count;

    // default value
    std::string transfer_wall_time_limit;

  public:
    activity_factory() : count(0),
			 transfer_wall_time_limit("")
    {}
    DESTRUCTOR(activity_factory);
    //
    void reset() { count = 0; }
    //
    void set_transfer_default(std::map<std::string, std::string>& m) {
      std::map<std::string, std::string>::const_iterator p;
      p = m.find(attribute::wall_time_limit);
      if (p != m.end()) {
	transfer_wall_time_limit = p->second;
      }
    }
    //
    jsdl_activity_ptr create_jsdl_activity() {
      return jsdl_activity_ptr(new jsdl_activity(++count));
    }
    //
    mkdir_activity_ptr create_mkdir_activity() {
      return mkdir_activity_ptr(new mkdir_activity(++count));
    }
    //
    transfer_activity_ptr create_transfer_activity() {
      transfer_activity_ptr ta(new transfer_activity(++count));
      if (!transfer_wall_time_limit.empty()) {
	ta->set_wall_time_limit(transfer_wall_time_limit);
      }
      return ta;
    }
  };
  typedef_ptr(activity_factory);

  //////////////////////////////////////////////////////////////////////
  //
  class control_link;
  typedef_ptr(control_link);

  class workflow {
    // <definitions name="name">
    // <exportedActivityInfo name="name">
    // <controlModel controlIn="name">
    std::string name;

    jsdl_activity_ptr job;
    mkdir_activity_ptr workdir;
    std::vector<transfer_activity_ptr> stage_in;
    std::vector<transfer_activity_ptr> stage_out;

  public:
    //
    workflow() : job((jsdl_activity*)0) {}
    //
    DESTRUCTOR(workflow);

    //
    void set_name(std::string name) { this->name = name; }
    std::string get_name() { return name; }
    //
    void set_jsdl_activity(jsdl_activity_ptr a) { job = a; }
    jsdl_activity_ptr get_jsdl_activity(void) { return job; }
    //
    void set_mkdir_activity(mkdir_activity_ptr a) { workdir = a; }
    mkdir_activity_ptr get_mkdir_activity(void) { return workdir; }
    //
    void add_stage_in_activity(transfer_activity_ptr a) {
      stage_in.push_back(a);
    }
    std::vector<transfer_activity_ptr>& get_stage_in_activity() {
      return stage_in;
    }
    //
    void add_stage_out_activity(transfer_activity_ptr a) {
      stage_out.push_back(a);
    }
    std::vector<transfer_activity_ptr>& get_stage_out_activity() {
      return stage_out;
    }

    //
    bool has_mkdir_activity() {
      return (workdir.get() && !workdir->get_target().empty());
    }
    bool has_stage_in_activity() { return (!stage_in.empty()); }
    bool has_stage_out_activity() { return (!stage_out.empty()); }

    //
    control_link_ptr create_control_link();
  };

  //////////////////////////////////////////////////////////////////////
  //
  class control_link {
    std::vector<activity_ptr> activities;
    control_link_ptr _next;

    //
    control_link_ptr next() {
      _next = control_link_ptr(new control_link());
      return _next;
    }

    friend control_link_ptr workflow::create_control_link();

  public:
    control_link() : _next((control_link*)0) {}
    //
    DESTRUCTOR(control_link);
    //
    void set_next(activity_ptr a) { activities.push_back(a); }
    //
    std::vector<activity_ptr> get_activities() { return activities; }
    //
    std::string get_type() { return activities[0]->get_name(); }
    //
    control_link_ptr get_next() { return _next; }

    void get_link();
  };

  //////////////////////////////////////////////////////////////////////
  //
  inline int count_link(control_link_ptr root) {
    int i = 0;

    for (control_link_ptr current = root;
	 current != 0; current = current->get_next()) {

      if (current->get_next() != 0) {
	i++;
      } else {
	i--;
      }
    }
    return i;
  }

  //////////////////////////////////////////////////////////////////////
  //
  class staging_path_builder {
    std::string localhost;
    std::string remotenode;
    const std::string home;
    std::string workdir;

  protected:
    //
    std::string create_uri(const std::string& nodename,
			   const std::string& path);
    //
    std::string resolve_path(const std::string& path,
			     const std::string& base = ".");
    //
    std::string get_cwd();

  public:
    staging_path_builder(std::string localhost) :
      localhost(localhost), remotenode(""), home("~"), workdir(home) {}
    //
    DESTRUCTOR(staging_path_builder);
    //
    void set_nodename(std::string nodename) { remotenode = nodename; }
    //
    void set_workdir(std::string path) {
      if (!path.empty()) {
	workdir = resolve_path(path, home);
      }
    }
    //
    std::string get_workdir() {
      return create_uri(remotenode, workdir);
    }
    //
    std::string get_stagein_source(transfer t) {
      // TODO t.first != URL
      return create_uri(localhost, resolve_path(t.first));
    }
    //
    std::string get_stagein_target(transfer t) {
      // TODO t.second != URL
      return create_uri(remotenode, resolve_path(t.second, workdir));
    }
    //
    std::string get_stageout_source(transfer t) {
      // TODO t.first != URL
      return create_uri(remotenode, resolve_path(t.first, workdir));
    }
    //
    std::string get_stageout_target(transfer t) {
      // TODO t.second != URL
      return create_uri(localhost, resolve_path(t.second));
    }

    std::string get_localhost() {
      return localhost;
    }
    std::string get_remotenode() {
      return remotenode;
    }
  };

  //////////////////////////////////////////////////////////////////////
  //
  class workflow_builder {

    activity_factory_ptr factory;

  public:
    //
    workflow_builder(activity_factory_ptr act_fc) :
      factory(act_fc) {}
    //
    DESTRUCTOR(workflow_builder);
    //
    workflow* get_workflow(std::string name, std::string localhost,
			   description* desc);
  };
}}

#endif  // WORKFLOW_HPP
