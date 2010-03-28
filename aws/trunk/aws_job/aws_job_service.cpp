//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

// stl includes
#include <vector>
#include <fstream>

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga engine includes
#include <saga/impl/config.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/attribute.hpp>

// saga package includes
#include <saga/saga/packages/job/adaptors/job.hpp>
#include <saga/saga/packages/job/adaptors/job_self.hpp>

// adaptor includes
#include "aws_job_service.hpp"
#include "aws_helper.hpp"


////////////////////////////////////////////////////////////////////////
namespace aws_job
{
  //////////////////////////////////////////////////////////////////////
  //
  // constructor
  //
  job_service_cpi_impl::job_service_cpi_impl (proxy                * p, 
                                              cpi_info const       & info,
                                              saga::ini::ini const & glob_ini, 
                                              saga::ini::ini const & adap_ini,
                                              TR1::shared_ptr <saga::adaptor> adaptor)
    : base_cpi (p, info, adaptor, cpi::Noflags)
  {
    adaptor_data  adata (this);
    instance_data idata (this);

    std::string scheme = idata->rm_.get_scheme ();
    std::string type;
    bool        ok = false;

    std::vector <std::string> types = saga::adaptors::utils::split (adata->ini_["defaults"]["cloud_names"], ' ');

    std::vector <saga::context> contexts = p->get_session ().list_contexts ();

    for ( unsigned int i = 0; i < types.size () && ! ok; i++ )
    {
      if ( scheme == types[i] ||
           scheme == "any"    ||
           scheme == "" )
      {
        // check if we have a context for that type
        std::vector <saga::context> :: iterator it;

        for ( it = contexts.begin (); ! ok && it != contexts.end () ; it++ )
        {
          if ( (*it).get_attribute (saga::attributes::context_type) == types[i] )
          {
            ok   = true;
            type = types[i];
            ini_ = adata->ini_[types[i]];
            ctx_ = *it;
          }
        }
      }
    }

    if ( ! ok )
    {
      // FIXME
      SAGA_ADAPTOR_THROW_NO_CONTEXT ((std::string ("Adaptor only supports 'aws' and 'any' URL schemes, not ")
                                      + scheme).c_str (),
                                     saga::BadParameter);
    }

    user_      = ctx_.get_attribute (saga::attributes::context_userid);
    userkey_   = ctx_.get_attribute (saga::attributes::context_userkey);

    env_["JAVA_HOME"]       = ini_["java_home"];
    env_["EC2_HOME"]        = ini_["ec2_home"];
    env_["EC2_GSG_KEY"]     = userkey_;
    env_["EC2_PRIVATE_KEY"] = ini_["ec2_key"];
    env_["EC2_CERT"]        = ini_["ec2_cert"];
    env_["EC2_URL"]         = ini_["ec2_url"];

    aws_job::process proc (env_);

    SAGA_LOG_INFO (" ========== logging aws job service rm");
    SAGA_LOG_INFO (idata->rm_.get_string ().c_str ());

    // is a VM given which we should contact, or do we need to start a new one?
    bool instance_started = false;
    if ( idata->rm_.get_host () == "" )
    {
      // no host - create a new VM instance to submit jobs to.

      proc.set_cmd (ini_["ec2_scripts"] + "/ec2-run-instances");

      proc.add_args ("-k", ini_["ec2_keypair"]);
      proc.add_arg  (ini_["ec2_image_id"]);

      std::vector <std::string> out = proc.run_sync ();

      if ( out.size() < 1 )
      {
        SAGA_ADAPTOR_THROW ("could not start new VM instance", saga::NoSuccess);
      }

      std::vector <std::string> out_words = saga::adaptors::utils::split (out[out.size () - 1]);

      if ( out_words.size() < 6 )
      {
        SAGA_ADAPTOR_THROW ("could not start new VM instance", saga::NoSuccess);
      }


      vm_ip_ = "";
      vm_id_ = out_words[1];

      std::string state = out_words[5];



      proc.set_cmd (ini_["ec2_scripts"] + "/ec2-describe-instances");

      proc.clear_args ();
      proc.add_arg (vm_id_);

      while ( state == "pending" )
      {
        ::sleep (1);

        out = proc.run_sync ();

        if ( out.size() < 1 )
        {
          SAGA_ADAPTOR_THROW ("could not check vm state", saga::NoSuccess);
        }

        out_words = saga::adaptors::utils::split (out[out.size () - 1]);

        if ( out_words.size() < 6 )
        {
          SAGA_ADAPTOR_THROW ("could not check VM state", saga::NoSuccess);
        }

        state = out_words[5];
        // std::cout << state << std::endl;
      }


      vm_ip_ = out_words[3];

      if ( state != "running" )
      {
        SAGA_ADAPTOR_THROW ("Cannot run vm instance", saga::NoSuccess);
      }

      std::cout << "id: " << vm_id_ << std::endl;
      std::cout << "ip: " << vm_ip_ << std::endl;

      idata->rm_.set_host (      vm_ip_);
      idata->rm_.set_path ("/" + vm_id_);

      std::cout << "rm: " << idata->rm_ << " === " << std::endl;

      // need some time for ssh to fire up
      int count = 0;
      while ( true )
      {
        proc.set_cmd ("/usr/bin/ssh");

        proc.clear_args ();

        proc.add_args ("-o", "StrictHostKeyChecking=no");
        proc.add_args ("-i", ini_["ec2_proxy"]);
        proc.add_arg (user_ + "@" + vm_ip_);
        proc.add_arg ("/bin/true");

        (void) proc.run_sync (false);

        if ( proc.done () )
        {
          break;
        }
        else
        {
          SAGA_LOG_ALWAYS ("trying ssh failed");
          ::sleep (1);
        }

        if ( count++ > 100 )
        {
          SAGA_ADAPTOR_THROW ("image could not provide ssh access", saga::NoSuccess);
        }
      }

      SAGA_LOG_ALWAYS ("trying ssh ok");

      instance_started = true;

    } 
    else
    {
      // host != "" - we have a instance id or an host name.  Check: if it starts with 
      // "i-" and contains no dots, it is an instance id.  Otherwise, its a hostname
      std::string host  = idata->rm_.get_host (); 

      if ( host[0] == 'i' &&
           host[1] == '-' &&
           host.find (".") == std::string::npos )
      {
        // host is an instance id, and we need to get the hostname

        SAGA_LOG_ALWAYS ("found id");

        std::string vm_id_ = host;


        proc.set_cmd (ini_["ec2_scripts"] + "/ec2-describe-instances");
        proc.clear_args ();
        proc.add_arg (vm_id_);

        std::vector <std::string> out = proc.run_sync ();

        if ( proc.fail () || out.size() < 1 )
        {
          SAGA_ADAPTOR_THROW ("contact does not point to a running VM instance", saga::BadParameter);
        }

        std::vector <std::string> out_words = saga::adaptors::utils::split (out[out.size () - 1]);

        if ( out_words.size() < 6 )
        {
          SAGA_ADAPTOR_THROW ("contact does not point to a running VM instance", saga::BadParameter);
        }

        vm_ip_ = out_words[3];

        std::string state  = out_words[5];

        std::cout << "id: " << vm_id_ << std::endl;
        std::cout << "ip: " << vm_ip_ << std::endl;
        std::cout << "st: " << state  << std::endl;


        if ( state != "running" )
        {
          SAGA_ADAPTOR_THROW ("contact does not point to a running VM instance", saga::BadParameter);
        }

        idata->rm_.set_host (      vm_ip_);
        idata->rm_.set_path ("/" + vm_id_);

        SAGA_LOG_ALWAYS (idata->rm_.get_string ().c_str ());
      }
      else
      {
        // host is indeed a host, and we need to get the instance id
        SAGA_LOG_ALWAYS ("found host");

        std::string vm_ip_ = host;


        proc.set_cmd (ini_["ec2_scripts"] + "/ec2-describe-instances");

        std::vector <std::string> out = proc.run_sync ();

        if ( proc.fail () || out.size() < 1 )
        {
          SAGA_ADAPTOR_THROW ("contact does not point to a running VM instance", saga::BadParameter);
        }

        // find the line which contains the hostname
        out = saga::adaptors::utils::grep (host, out);

        if ( out.size () != 1 )
        {
          SAGA_ADAPTOR_THROW ("contact does not point to a running VM instance", saga::NoSuccess);
        }

        SAGA_LOG_ALWAYS (out[0].c_str ());

        std::vector <std::string> out_words = saga::adaptors::utils::split (out[0]);

        if ( out_words.size() < 6 )
        {
          SAGA_ADAPTOR_THROW ("contact does not point to a running VM instance", saga::BadParameter);
        }

        vm_id_ = out_words[1];

        std::string state  = out_words[5];

        std::cout << "id: " << vm_id_ << std::endl;
        std::cout << "ip: " << vm_ip_ << std::endl;
        std::cout << "st: " << state  << std::endl;


        if ( state != "running" )
        {
          SAGA_ADAPTOR_THROW ("contact does not point to a running VM instance", saga::NoSuccess);
        }

        idata->rm_.set_path ("/" + vm_id_);
      }
    }


    // we do have a job service instance, either new started or old and running.
    // we used a valid private key for that, so that is ok.  What we miss is the
    // public key of the pair.  So, if our context does not have such a public
    // key, we grab it from the ~/.ssh/authorized_keys file on the remote host,
    // store it locally, and 'fix' (aka complete) the context.

    if ( !     ctx_.attribute_exists (saga::attributes::context_usercert) ||
         "" == ctx_.get_attribute    (saga::attributes::context_usercert) )
    {
      SAGA_LOG_ALWAYS ("retrieving public key");

      proc.set_cmd ("/usr/bin/ssh");

      proc.clear_args ();
      proc.clear_out  ();
      proc.add_args ("-o", "StrictHostKeyChecking=no");
      proc.add_args ("-i", userkey_);
      proc.add_arg  (user_ + "@" + vm_ip_);

      // we run a grep for the 
      proc.add_args ("grep", ini_["ec2_keypair"]);

      // file to search
      proc.add_arg (".ssh/authorized_keys");

      (void) proc.run_sync (true);

      if ( proc.fail () )
      {
        SAGA_LOG_ALWAYS ("could not retrieve public ssh key");
        throw;
      }

      std::string out = proc.get_out_s ();
      std::string pub = ctx_.get_attribute (saga::attributes::context_userkey) + ".pub";

      std::fstream pub_fs;

      pub_fs.open (pub.c_str (), std::fstream::out);
      pub_fs << out;
      pub_fs.close ();

      std::cout << "setting usercert " << pub << "\n";
      ctx_.set_attribute (saga::attributes::context_usercert, pub);
    }

    usercert_  = ctx_.get_attribute (saga::attributes::context_usercert);


    // we are sure ssh is running - we should be able to create a ssh job
    // service for the instance now
    ssh_url_ = std::string ("ssh://") + vm_ip_;

    ssh_context_.set_attribute (saga::attributes::context_type,     "ssh");
    ssh_context_.set_attribute (saga::attributes::context_userid,   user_);
    ssh_context_.set_attribute (saga::attributes::context_userkey,  userkey_);
    ssh_context_.set_attribute (saga::attributes::context_usercert, usercert_);

    ssh_session_.add_context (ssh_context_);

    // create ssh job service which from now on handles job submission etc
    js_  = TR1::shared_ptr <saga::job::service> (new saga::job::service (ssh_session_, ssh_url_));

    // seems to work, ssh is set up - now prepare the image for job submission,
    // if needed
    if ( instance_started && ini_["ec2_image_prep"] != "" )
    {
      SAGA_LOG_ALWAYS ("preparing image");

      // stage the prep script to the VM
      proc.set_cmd ("/usr/bin/scp");

      proc.clear_args ();
      proc.add_args ("-o", "StrictHostKeyChecking=no");
      proc.add_args ("-i", ini_["ec2_proxy"]);
      proc.add_arg  (ini_["ec2_image_prep"]);
      proc.add_arg  (user_ + "@" + vm_ip_ + ":/tmp/saga-ec2-image-prep");

      (void) proc.run_sync (false);

      if ( proc.fail () )
      {
        SAGA_ADAPTOR_THROW ("image prep-staging failed", saga::NoSuccess);
      }


      // make executable
      proc.set_cmd ("/usr/bin/ssh");

      proc.clear_args ();
      proc.add_args ("-o", "StrictHostKeyChecking=no");
      proc.add_args ("-i", ini_["ec2_proxy"]);
      proc.add_arg  (user_ + "@" + vm_ip_);
      proc.add_arg  ("/bin/chmod");
      proc.add_arg  ("0755");
      proc.add_arg  ("/tmp/saga-ec2-image-prep");

      (void) proc.run_sync (false);

      if ( proc.fail () )
      {
        SAGA_ADAPTOR_THROW ("image prep-chmod failed", saga::NoSuccess);
      }

      // run the prep script
      // when the prep script returns successfully, we take that as a signal that 
      // SAGA applications can run on the remote host.
      proc.set_cmd ("/usr/bin/ssh");

      proc.clear_args ();
      proc.clear_out  ();
      proc.clear_err  ();
      proc.add_args ("-o", "StrictHostKeyChecking=no");
      proc.add_args ("-i", ini_["ec2_proxy"]);
      proc.add_arg  (user_ + "@" + vm_ip_);
      proc.add_arg  ("/tmp/saga-ec2-image-prep");

      (void) proc.run_sync (false);

      if ( proc.done () )
      {
        SAGA_LOG_ALWAYS ("image prep succeeded");
      }
      else
      {
        SAGA_ADAPTOR_THROW ("image preparation failed repeatedly", saga::NoSuccess);
      }
    }
  }

  //////////////////////////////////////////////////////////////////////
  //
  // destructor
  //
  job_service_cpi_impl::~job_service_cpi_impl (void) 
  {
    // TODO: if no jobs are running on the VM instance, kill it.  Otherwise warn
    // the user that the VM keeps running even after the last job dies...
    // An explicit close() is missing - VMs need to be terminated out-of-bound.
    //
    // FIXME: for now, we simply shut down the instance, if ec2_keepalive is not
    // set to false, to avoid overly large bills for my credit card :o)

    if ( ini_["ec2_keepalive"] == "yes"  ||
         ini_["ec2_keepalive"] == "true" )
    {
      SAGA_LOG_ALWAYS ("ec2_keepalive: true"); 
    }
    else
    {
      SAGA_LOG_ALWAYS ("ec2_keepalive: !true"); 

      std::string msg ("Shut down VM instance ");
      msg += vm_id_;
      SAGA_LOG_ALWAYS (msg.c_str ()); 

      aws_job::process proc (ini_["ec2_scripts"] + "/ec2-terminate-instances", env_);
      proc.add_arg (vm_id_);

      (void) proc.run_sync (false);

      if ( proc.fail () )
      {
        // well, we can't do much in case of errors, other then warn the user
        // that the VM may continue to run
        SAGA_LOG_CRITICAL ("Could not shut down VM instance ");
      }
    }

    // the ssh job service is automatically destroyed here.
  }

  //////////////////////////////////////////////////////////////////////
  // SAGA API functions
  void job_service_cpi_impl::sync_create_job (saga::job::job         & ret, 
                                              saga::job::description   jd)
  {
    ret = js_->create_job (jd);
  }

  void job_service_cpi_impl::sync_run_job (saga::job::job     & ret, 
                                           std::string          cmd, 
                                           std::string          host, 
                                           saga::job::ostream & in, 
                                           saga::job::istream & out, 
                                           saga::job::istream & err)
  {
    ret = js_->run_job (cmd, host, in, out, err);
  }

  void job_service_cpi_impl::sync_list (std::vector <std::string> & ret)
  {
    ret = js_->list ();
  }

  void job_service_cpi_impl::sync_get_job (saga::job::job & ret, 
                                           std::string      jobid)
  {
    ret = js_->get_job (jobid);
  }

  void job_service_cpi_impl::sync_get_self (saga::job::self & ret)
  {
    // will never be implemented by this adaptor
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_service_cpi_impl::dump_context (saga::context c)
  {
    std::cout << " --------------------------------------\n";

    std::vector <std::string> attribs = c.list_attributes ();

    for ( unsigned int i = 0; i < attribs.size (); i++ )
    {
      std::string key = attribs[i];
      std::string val;

      if ( c.attribute_is_vector (key) )
      {
        std::vector <std::string> vals = c.get_vector_attribute (key);
        for ( unsigned int j = 0; j < vals.size (); j++ )
        {
          val += vals[j] + " ";
        }
      }
      else
      {
        val = c.get_attribute (key);
      }

      std::cout << key << " \t: " << val << std::endl;
    }

    std::cout << " --------------------------------------\n";
  }

} // namespace aws_job
////////////////////////////////////////////////////////////////////////

