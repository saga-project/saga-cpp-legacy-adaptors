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


#include <boost/foreach.hpp>

#include "wfml.hpp"
#include "wfml_writer.hpp"

namespace naregi_job { namespace wfml {
  //////////////////////////////////////////////////////////////////////
  //
  void mkdir_element_writer::set_attrs(xml_tag* t)
  {
    t->add_attr(attribute::wall_time_limit, wall_time_limit);
  }

  //
  //  <mkdir>
  //    <target />
  //  </mkdir>
  //
  void mkdir_element_writer::format(std::ostream& s)
  {
    xml_tag_ptr t(new xml_tag(element::target));
    t->add_attr(attribute::name, target);
    s << xml->simple_tag(*t) << '\n';
  }

  //
  void mkdir_element_writer::set_activity(mkdir_activity_ptr ma)
  {
    wall_time_limit = ma->get_wall_time_limit();
    target = ma->get_target();
  }

  //////////////////////////////////////////////////////////////////////
  //
  void transfer_element_writer::set_attrs(xml_tag* t) {
    t->add_attr(attribute::wall_time_limit, wall_time_limit);
  }

  //
  //  <transfer>
  //    <source />
  //    <target />
  //  </transfer>
  //
  void transfer_element_writer::format(std::ostream& s)
  {
    xml_tag_ptr t1(new xml_tag(element::source));
    t1->add_attr(attribute::name, source);
    s << xml->simple_tag(*t1) << '\n';

    xml_tag_ptr t2(new xml_tag(element::target));
    t2->add_attr(attribute::name, target);
    s << xml->simple_tag(*t2) << '\n';
  }

  //
  void transfer_element_writer::set_activity(transfer_activity_ptr ta)
  {
    wall_time_limit = ta->get_wall_time_limit();
    source = ta->get_source();
    target = ta->get_target();
  }

  //////////////////////////////////////////////////////////////////////
  //
  //  <exportedActivity>
  //    <exportedActivityInfo />
  //    <controlModel>
  //      <controlLink />
  //      ...
  //    </controlModel>
  //  </exportedActivity>
  //
  void exported_activity_writer::format(std::ostream& s)
  {
    static const std::string label = "WFTGEN";
    xml_tag_ptr
      t1(new xml_tag(element::exported_activity_info));
    t1->add_attr(attribute::name, wf->get_name());

    xml_tag_ptr t2(new xml_tag(element::control_model));
    t2->add_attr(attribute::control_in, wf->get_name());

    s << xml->simple_tag(*t1) << '\n';
    s << xml->start_tag(*t2) << '\n';

    control_link_ptr root = wf->create_control_link();

    int n = count_link(root);

    if (n > 0) {
      control_link_ptr current = root;
      for (int l = 0; l < n; l++) {
	control_link_ptr next = current->get_next();
	BOOST_FOREACH(activity_ptr sa, current->get_activities()) {
	  BOOST_FOREACH(activity_ptr ta, next->get_activities()) {
	    xml_tag_ptr t(new xml_tag(element::control_link));
	    t->add_attr(attribute::label, label);
	    t->add_attr(attribute::source, sa->get_name());
	    t->add_attr(attribute::target, ta->get_name());
	    s << xml->simple_tag(*t) << '\n';
	  }
	}
	current = next;
      }
    } else {
      xml_tag_ptr t(new xml_tag(element::control_link));
      t->add_attr(attribute::label, label);
      t->add_attr(attribute::source, root->get_type());
      s << xml->simple_tag(*t) << '\n';
    }

    s << xml->end_tag(*t2) << '\n';
  }

  //////////////////////////////////////////////////////////////////////
  //
  std::ostream& model_writer::put(std::ostream& s)
  {
    if (has_data()) {
      s << xml->start_tag(name) << '\n';
      format(s);
      s << xml->end_tag(name) << '\n';
    } else {
      s << xml->simple_tag(name) << '\n';
    }

    return s;
  }

  //////////////////////////////////////////////////////////////////////
  //
  // <activity>
  //   <jsdl>...</jsdl>
  // </activity>
  // <activity>
  //   <mkdir>...</mkdir>
  // </activity>
  // <activity>
  //   <transfer>...</transfer>
  // </activity>
  // <activity>
  //   <transfer>...</transfer>
  // </activity>
  //
  void activity_model_writer::format(std::ostream& s)
  {
    jsdl_activity_ptr ja = wf->get_jsdl_activity();
    jsdlwr->set_description(&ja->get_desc());
    actwr->set_writer(ja->get_name(), jsdlwr);
    s << *actwr;

    if (wf->has_mkdir_activity()) {
      mkdir_activity_ptr ma = wf->get_mkdir_activity();
      mkdirwr->set_activity(ma);
      actwr->set_writer(ma->get_name(), mkdirwr);
      s << *actwr;
    }

    if (wf->has_stage_in_activity()) {
      BOOST_FOREACH(transfer_activity_ptr ta, wf->get_stage_in_activity()) {
	transwr->set_activity(ta);
	actwr->set_writer(ta->get_name(), transwr);
	s << *actwr;
      }
    }

    if (wf->has_stage_out_activity()) {
      BOOST_FOREACH(transfer_activity_ptr ta, wf->get_stage_out_activity()) {
	transwr->set_activity(ta);
	actwr->set_writer(ta->get_name(), transwr);
	s << *actwr;
      }
    }

  }

  //////////////////////////////////////////////////////////////////////
  //
  void export_model_writer::format(std::ostream& s)
  {
    s << *exactwr;
  }

  //////////////////////////////////////////////////////////////////////
  //
  xml_tag* workflow_writer::get_tag(void)
  {
    xml_tag* t = new xml_tag(element::definitions);
    t->add_attr("xmlns", xmlns);
    t->add_attr(attribute::name, wf->get_name());
    return t;
  }

  //////////////////////////////////////////////////////////////////////
  //
  std::ostream& workflow_writer::put(std::ostream& s)
  {
    xml_tag_ptr t(get_tag());

    amwr->set_workflow(wf);

    export_model_writer_ptr exwr = cmwr->get_export_model_writer();
    // TODO workflow -> activity
    exwr->set_workflow(wf);

    s << xml->declaration() << '\n';
    s << xml->start_tag(*t) << '\n';

    s << *amwr << *cmwr;

    s << xml->end_tag(*t) << '\n';

    return s;
  }
}}
