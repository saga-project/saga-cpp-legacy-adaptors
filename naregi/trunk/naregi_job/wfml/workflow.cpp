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


#if 0
#include <iostream>
#endif

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "workflow.hpp"

namespace naregi_job { namespace wfml {

  using namespace naregi_job::jsdl;

  //////////////////////////////////////////////////////////////////////
  //
  std::string staging_path_builder::create_uri(const std::string& nodename,
					       const std::string& path)
  {
    std::string uri = "default://" + nodename;
    if (path.compare(0, 1, "/") == 0) {
      uri += path;
    } else {
      uri += "/" + path;
    }
    return uri;
  }

  //
  std::string staging_path_builder::resolve_path(const std::string& path,
						 const std::string& base)
  {
    // path : absolute
    if (path.compare(0, 1, "/") == 0) {
      return path;
    }

    // path : related
    if (base.compare(".") == 0) {
      return get_cwd() + "/" + path;
    }

    return base + "/" + path;
  }

  //
  std::string staging_path_builder::get_cwd()
  {
    boost::filesystem::path cwd = boost::filesystem::current_path();
    return cwd.string();
  }

  //////////////////////////////////////////////////////////////////////
  //
  std::string jsdl_activity::get_nodename(void) const
  {
    //std::cout << "get_nodename()" << std::endl;
    if (nodename.empty()) {
      std::string a = get_name();

      // FIXME? % encode => URL
      std::string::size_type i = a.find("#");
      if (i != std::string::npos) {
	a.replace(i, 1, "%23");
      }

      nodename = "/FixMe(" + a + ")eMxiF";
    }
    return nodename;
  }

  //////////////////////////////////////////////////////////////////////
  //
  control_link_ptr workflow::create_control_link()
  {
    control_link_ptr root = control_link_ptr(new control_link());
    control_link_ptr current = root;

    if (has_mkdir_activity()) {
      mkdir_activity_ptr ma = get_mkdir_activity();
//std::cout << ma->get_name() << std::endl;
      current->set_next(ma);
      current = current->next();
    }

    if (has_stage_in_activity()) {
      BOOST_FOREACH(transfer_activity_ptr ta, get_stage_in_activity()) {
//std::cout << ta->get_name() << std::endl;
	current->set_next(ta);
      }
      current = current->next();
    }

    jsdl_activity_ptr ja = get_jsdl_activity();
//std::cout << ja->get_name() << std::endl;
    current->set_next(ja);
    current = current->next();

    if (has_stage_out_activity()) {
      BOOST_FOREACH(transfer_activity_ptr ta, get_stage_out_activity()) {
//std::cout << ta->get_name() << std::endl;
	current->set_next(ta);
      }
      current = current->next();
    }
    return root;
  }

  //////////////////////////////////////////////////////////////////////
  //
  workflow* workflow_builder::get_workflow(std::string name,
                                           std::string localhost,
                                           description* desc)
  {
    staging_path_builder sp_bldr(localhost);

    factory->reset();

    workflow* wf = new workflow();
    wf->set_name(name);

    jsdl_activity_ptr ja = factory->create_jsdl_activity();
    ja->set_desc(*desc);
    wf->set_jsdl_activity(ja);

    sp_bldr.set_nodename(ja->get_nodename());

    posix_application_ptr pa =
      get_posix_application_ptr(desc->get_application());

    if (!pa->get_working_directory().empty()) {
      sp_bldr.set_workdir(pa->get_working_directory());
      mkdir_activity_ptr ma = factory->create_mkdir_activity();
      ma->set_target(sp_bldr.get_workdir());
      wf->set_mkdir_activity(ma);
    }

    jsdl_data_staging_ptr ds = desc->get_data_staging();
    if (ds == 0) {
      return wf;
    }

    BOOST_FOREACH(transfer t, ds->get_stage_in()) {
      transfer_activity_ptr ta = factory->create_transfer_activity();
      ta->set_transfer(sp_bldr.get_stagein_source(t),
		       sp_bldr.get_stagein_target(t));
      wf->add_stage_in_activity(ta);
    }

    BOOST_FOREACH(transfer t, ds->get_stage_out()) {
      transfer_activity_ptr ta = factory->create_transfer_activity();
      ta->set_transfer(sp_bldr.get_stageout_source(t),
		       sp_bldr.get_stageout_target(t));
      wf->add_stage_out_activity(ta);
    }

    return wf;
  }
}}
