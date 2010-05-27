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


#ifndef WFML_WRITER_HPP
#define WFML_WRITER_HPP

#include <boost/shared_ptr.hpp>

#include "wfml.hpp"
#include "wfml_jsdl_writer.hpp"
#include "writer.hpp"
#include "workflow.hpp"

namespace naregi_job { namespace wfml {

  char const* const xmlns = "http://www.naregi.org/wfml/02";

  //typedef_ptr(workflow);
  typedef workflow* wf_ptr;

  //////////////////////////////////////////////////////////////////////
  //
  class element_writer : public xml_writer, public writer {
    //
    virtual xml_tag* get_tag(void) = 0;

  protected:
    //
    virtual void format(std::ostream& s) = 0;

  public:
    //
    element_writer(xml_ptr xml) : xml_writer(xml) {}
    VDESTRUCTOR(element_writer);

    //
    std::ostream& put(std::ostream& s)
    {
      xml_tag_ptr t(get_tag());

      s << xml->start_tag(*t) << '\n';
      format(s);
      s << xml->end_tag(*t) << '\n';
      return s;
    }
  };

  //////////////////////////////////////////////////////////////////////
  //
  class activity_child_writer : public element_writer {
    std::string name;

    //
    xml_tag* get_tag(void) {
      xml_tag* t = new xml_tag(name);
      set_attrs(t);
      return t;
    }

  protected:
    //
    virtual void set_attrs(xml_tag* t) {}

  public:
    activity_child_writer(xml_ptr xml, std::string name) :
      element_writer(xml), name(name) {}
    VDESTRUCTOR(activity_child_writer);
  };
  typedef_ptr(activity_child_writer);

#define ACTIVITY_CHILD_CONSTRUCTOR(klass, name)                  \
      klass (xml_ptr xml) : activity_child_writer(xml, name)

  //////////////////////////////////////////////////////////////////////
  //  <activity name="">...</activity>
  //
  class activity_writer : public element_writer {
    std::string name;
    activity_child_writer_ptr child_writer;

    //
    xml_tag* get_tag(void)
    {
      xml_tag* t = new xml_tag(element::activity);
      t->add_attr(attribute::name, name);
      return t;
    }

  protected:
    //
    void format(std::ostream& s) {
      child_writer->put(s);
    }

  public:
    //
    activity_writer(xml_ptr xml) : element_writer(xml) {}
    DESTRUCTOR(activity_writer);
    //
    void set_writer(std::string name_, activity_child_writer_ptr wr) {
      name = name_;
      child_writer = wr;
    }
  };

  //////////////////////////////////////////////////////////////////////
  //  <jsdl>...</jsdl>
  //
  class jsdl_element_writer : public activity_child_writer {
    jsdl::jobdefinition_writer jsdlwr;

  protected:
    //
    void format(std::ostream& s) { jsdlwr.put(s); }

  public:
    //
    ACTIVITY_CHILD_CONSTRUCTOR(jsdl_element_writer, element::jsdl),
      jsdlwr(jsdl::jobdefinition_writer(xml)) {}
    //
    DESTRUCTOR(jsdl_element_writer);
    //
    void set_description(description* d) { jsdlwr.set_desc(d); }
  };

  //////////////////////////////////////////////////////////////////////
  //  <mkdir>...</mkdir>
  //
  class mkdir_element_writer : public activity_child_writer {
    std::string wall_time_limit;
    std::string target;

  protected:
    //
    void set_attrs(xml_tag* t);
    //
    void format(std::ostream& s);

  public:
    //
    ACTIVITY_CHILD_CONSTRUCTOR(mkdir_element_writer, element::mkdir) {}
    DESTRUCTOR(mkdir_element_writer);
    //
    void set_activity(mkdir_activity_ptr activity);
  };

  //////////////////////////////////////////////////////////////////////
  //  <transfer>...</transfer>
  //
  class transfer_element_writer :public activity_child_writer {
    std::string wall_time_limit;
    std::string source;
    std::string target;

  protected:
    //
    void set_attrs(xml_tag* t);
    //
    void format(std::ostream& s);

  public:
    //
    ACTIVITY_CHILD_CONSTRUCTOR(transfer_element_writer,
				 element::transfer) {}
    DESTRUCTOR(transfer_element_writer);
    //
    void set_activity(transfer_activity_ptr activity);
  };

  //////////////////////////////////////////////////////////////////////
  //  <exportedActivity>...</exportedActivity>
  //
  class exported_activity_writer : public element_writer {
    //std::string flow_name;
    workflow* wf;

    //
    xml_tag* get_tag(void) {
      xml_tag* t = new xml_tag(element::exported_activity);
      return t;
    }

  protected:
    //
    void format(std::ostream& s);

  public:
    //
    exported_activity_writer(xml_ptr xml) : element_writer(xml) {}
    DESTRUCTOR(exported_activity_writer);
    //
    //void set_flow_name(std::string name) { flow_name = name; }
    void set_workflow(workflow *wf) { this->wf = wf; }
  };

  //////////////////////////////////////////////////////////////////////
  //
  class model_writer : public xml_writer, public writer {
    std::string name;

  protected:
    //
    virtual bool has_data() { return true; }
    //
    virtual void format(std::ostream& s) = 0;

  public:
    //
    model_writer(std::string name, xml_ptr xml) :
      xml_writer(xml), name(name) {}
    //
    VDESTRUCTOR(model_writer);
    //
    std::ostream& put(std::ostream& s);
  };

#define MODEL_CONSTRUCTOR(klass, name)                  \
      klass (xml_ptr xml) : model_writer(name, xml)

  //////////////////////////////////////////////////////////////////////
  //  <activityModel>...</activityModel>
  //
  class activity_model_writer : public model_writer {
    boost::shared_ptr<activity_writer> actwr;
    boost::shared_ptr<jsdl_element_writer> jsdlwr;
    boost::shared_ptr<mkdir_element_writer> mkdirwr;
    boost::shared_ptr<transfer_element_writer> transwr;

    wf_ptr wf;

  protected:
    //
    void format(std::ostream& s);

  public:
    //
    MODEL_CONSTRUCTOR(activity_model_writer, element::activity_model),
      actwr(new activity_writer(xml)),
      jsdlwr(new jsdl_element_writer(xml)),
      mkdirwr(new mkdir_element_writer(xml)),
      transwr(new transfer_element_writer(xml))
    {}
    //
    DESTRUCTOR(activity_model_writer);
    //
    void set_workflow(wf_ptr w) { wf = w; }
  };

  //////////////////////////////////////////////////////////////////////
  //  <importModel>...</importModel>
  //
  class import_model_writer : public model_writer {

  protected:
    //
    bool has_data() { return false; }

    //
    void format(std::ostream& s)
    {
      s << "import_model" << '\n';
    }

  public:
    //
    MODEL_CONSTRUCTOR(import_model_writer, element::import_model) {}
    //
    DESTRUCTOR(import_model_writer);
  };
  typedef_ptr(import_model_writer);

  //////////////////////////////////////////////////////////////////////
  //  <exmportModel>...</exmportModel>
  //
  class export_model_writer : public model_writer {
    // TODO activity
    boost::shared_ptr<exported_activity_writer> exactwr;

  protected:
    //
    void format(std::ostream& s);

  public:
    //
    MODEL_CONSTRUCTOR(export_model_writer, element::export_model),
      exactwr(new exported_activity_writer(xml))
    {}
    //
    DESTRUCTOR(export_model_writer);
    //
    void set_workflow(workflow* wf) {
      // TODO activity
      //exactwr->set_flow_name(wf->get_name());
      exactwr->set_workflow(wf);
    }
  };
  typedef_ptr(export_model_writer);

  //////////////////////////////////////////////////////////////////////
  //  <compositionModel>...</compositinoModel>
  //
  class composition_model_writer : public model_writer {
    import_model_writer_ptr imwr;
    export_model_writer_ptr exwr;

  protected:
    //
    void format(std::ostream& s) { s << *imwr << *exwr; }

  public:
    //
    MODEL_CONSTRUCTOR(composition_model_writer, element::composition_model),
      imwr(new import_model_writer(xml)),
      exwr(new export_model_writer(xml)) {}
    //
    DESTRUCTOR(composition_model_writer);
    //
    import_model_writer_ptr get_import_model_writer(void) { return imwr; }
    //
    export_model_writer_ptr get_export_model_writer(void) { return exwr; }
    // TODO set_activity?
  };

  //////////////////////////////////////////////////////////////////////
  //
  class workflow_writer : public xml_writer, public writer {

    boost::shared_ptr<activity_model_writer> amwr;
    boost::shared_ptr<composition_model_writer> cmwr;

    wf_ptr wf;
    xml_tag* get_tag(void);

  public:
    //
    workflow_writer() :
      amwr(new activity_model_writer(xml)),
      cmwr(new composition_model_writer(xml)) { }
    //
    DESTRUCTOR(workflow_writer);
    //
    void set_workflow(workflow* w) { wf = w; }
    //
    std::ostream& put(std::ostream& s);
  };
}}

#endif  // WFML_WRITER_HPP
