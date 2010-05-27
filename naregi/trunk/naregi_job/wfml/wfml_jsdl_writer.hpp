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


#ifndef WFML_JSDL_WRITER_HPP
#define WFML_JSDL_WRITER_HPP

#include <boost/scoped_ptr.hpp>

#include "wfml.hpp"
#include "jsdl.hpp"
#include "jsdl_formatter.hpp"
#include "writer.hpp"

namespace naregi_job { namespace wfml { namespace jsdl {

  using namespace naregi_job::jsdl;

  //////////////////////////////////////////////////////////////////////
  // jsdl writer
  class job_identity_writer :
    public job_identity_formatter,
    private xml_writer
  {
    void output_jobname(std::ostream& s, jsdl_job_identity* j);

  public:
    job_identity_writer(xml_ptr xml) : xml_writer(xml) {}
    DESTRUCTOR(job_identity_writer);
  };
  typedef_ptr(job_identity_writer);

  //////////////////////////////////////////////////////////////////////
  // jsdl writer
  template<class T> class extention_writer : private xml_writer {

  public:
    extention_writer(xml_ptr xml) : xml_writer(xml) {}
    //
    VDESTRUCTOR(extention_writer);
    //
    virtual xml_tag* get_tag(void) = 0;
    //
    virtual void format(std::ostream& s, T j) = 0;
  };

  typedef extention_writer<jsdl_application_ptr> application_extention;
  typedef_ptr(application_extention);

  //////////////////////////////////////////////////////////////////////
  // jsdl writer
  class application_writer :
    private xml_writer
  {
    application_extention_ptr ex;

  public:
    //
    application_writer(xml_ptr xml) : xml_writer(xml) {}
    //
    DESTRUCTOR(application_writer);
    //
    void set_extention(application_extention_ptr ptr) { ex = ptr; }
    //
    void format(std::ostream& s, jsdl_application_ptr j) {
      xml_tag_ptr t(ex->get_tag());
      s << xml->start_tag(*t) << '\n';
      ex->format(s, j);
      s << xml->end_tag(*t) << '\n';
    }
  };
  typedef_ptr(application_writer);

  //////////////////////////////////////////////////////////////////////
  // jsdl writer
  class resources_writer :
    public resources_formatter,
    private xml_writer
  {
    void output_candidate_hosts(std::ostream& s, jsdl_resources* j);
    void output_operating_system(std::ostream& s, jsdl_resources* j);
    void output_individual_cpu_count(std::ostream& s, jsdl_resources* j);

  public:
    resources_writer(xml_ptr xml) : xml_writer(xml) {}
    DESTRUCTOR(resources_writer);
  };
  typedef_ptr(resources_writer);

  //////////////////////////////////////////////////////////////////////
  // jsdl writer
  class posix_application_writer :
    public posix_application_formatter,
    private xml_writer
  {
    void output_executable(std::ostream& s, posix_application* j);
    void output_arguments(std::ostream& s, posix_application* j);
    void output_output(std::ostream& s, posix_application* j);
    void output_error(std::ostream& s, posix_application* j);
    void output_working_directory(std::ostream& s, posix_application* j);
    void output_environments(std::ostream& s, posix_application* j);
    void output_wall_time_limit(std::ostream& s, posix_application* j);
  public:
    posix_application_writer(xml_ptr xml) : xml_writer(xml) {}
    DESTRUCTOR(posix_application_writer);
  };
  typedef_ptr(posix_application_writer);

  //////////////////////////////////////////////////////////////////////
  // jsdl writer
  class posix_application_writer_extention :
    public application_extention
  {
    posix_application_writer_ptr impl;

  protected:
    //
    xml_tag* get_tag(void) {
      xml_tag* t = new xml_tag(jsdl_posix::application);
      t->add_attr("xmlns", jsdl_posix::jsdl_posix);
      return t;
    }

  public:
    posix_application_writer_extention(xml_ptr xml) :
      application_extention(xml),
      impl(new posix_application_writer(xml)) {}
    //
    DESTRUCTOR(posix_application_writer_extention);
    //
    void format(std::ostream& s, jsdl_application_ptr j) {
      application_impl_ptr p = j->get_impl();
      posix_application* jj = dynamic_cast<posix_application*>(p.get());
      impl->format(s, jj);
    }
  };
  typedef_ptr(posix_application_writer_extention);

  //////////////////////////////////////////////////////////////////////
  // jobdefinition formatter
  class jobdefinition_formatter {
    virtual void output_identification(std::ostream& s) = 0;
    virtual void output_application(std::ostream& s) = 0;
    virtual void output_resources(std::ostream& s) = 0;

  protected:
    //
    void format(std::ostream& s) {
      output_identification(s);
      output_application(s);
      output_resources(s);
    }

  public:
    //
    VDESTRUCTOR(jobdefinition_formatter);
  };

  class jobdefinition_writer :
    public jobdefinition_formatter,
    private xml_writer,
    private writer
  {
    description* desc;

    //
    void output_identification(std::ostream& s);
    void output_application(std::ostream& s);
    void output_resources(std::ostream& s);

    job_identity_writer_ptr ident;
    application_writer_ptr application;
    resources_writer_ptr resources;

    //
    xml_tag* get_tag(void) {
      xml_tag* t = new xml_tag(elements::job_definition);
      t->add_attr("xmlns", elements::jsdl);
      t->add_attr("xmlns:naregi", elements::naregi);
      return t;
    }

  public:
    //
    jobdefinition_writer(xml_ptr f) :
      xml_writer(f),
      ident(new job_identity_writer(f)),
      application(new application_writer(f)),
      resources(new resources_writer(f))
    {
      posix_application_writer_extention_ptr
	ex(new posix_application_writer_extention(f));
      application->set_extention(ex);
    }

    //
    DESTRUCTOR(jobdefinition_writer);

    //
    void set_desc(description* d) { desc = d; }

    //
    std::ostream& put(std::ostream& s);
  };
}}}

#endif  // WFML_JSDL_WRITER_HPP
