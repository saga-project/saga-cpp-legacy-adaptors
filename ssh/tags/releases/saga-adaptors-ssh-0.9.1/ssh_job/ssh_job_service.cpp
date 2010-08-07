//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

// system includes
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>


// stl includes
#include <vector>
#include <fstream>

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga engine includes
#include <saga/impl/config.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/utils.hpp>
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/attribute.hpp>

// saga package includes
#include <saga/saga/packages/job/adaptors/job.hpp>
#include <saga/saga/packages/job/adaptors/job_self.hpp>

// adaptor includes
#include "ssh_job_service.hpp"


////////////////////////////////////////////////////////////////////////
namespace ssh_job
{
  // constructor
  job_service_cpi_impl::job_service_cpi_impl (proxy                * p, 
                                              cpi_info const       & info,
                                              saga::ini::ini const & glob_ini, 
                                              saga::ini::ini const & adap_ini,
                                              TR1::shared_ptr <saga::adaptor> adaptor)
    : base_cpi (p, info, adaptor, cpi::Noflags)
    , s_ (p->get_session ()) // empty session
      // Create a local job adaptor to spawn off ssh commands at will.  
      // If that throws, we simply pass on the exception.
  {
    adaptor_data  adata (this);
    instance_data idata (this);

    // create local job service which handles all job submissions.  This
    // may throw.
    js_ = saga::job::service ("fork://localhost");


    ini_ = adap_ini.get_section ("preferences").get_entries ();

    check_ini_ ();


    rm_   = idata->rm_;
    host_ = rm_.get_host ();
    port_ = rm_.get_port ();
    path_ = rm_.get_path ();


    if ( ! path_.empty () &&
           path_ != "/"   )
    {
      SAGA_ADAPTOR_THROW ("Cannot handle path in ssh URLs", 
                          saga::BadParameter);
    }

    // ssh declines localhost - that is taken care of by local adaptor...
    if ( host_.empty () )
    {
      // we can succeed w/o hostname for now, but may fail later on actually
      // running the job.  We will then assume either localhost, or one of the
      // candidate_hosts - but those may be invalid of course.
    }

    if ( rm_.get_scheme () != "ssh" &&
         rm_.get_scheme () != "any" &&
         rm_.get_scheme () != "" )
    {
      SAGA_ADAPTOR_THROW (std::string ("Adaptor only supports these schemas: 'ssh://', 'any://', none, but not ")
                          + rm_.get_scheme ().c_str (),
                          saga::adaptors::AdaptorDeclined);
    }

    if ( port_ > 0 )
    {
      std::stringstream ss;
      ss << port_;
      std::string port_s = ss.str ();

      ssh_opt_.push_back ("-p");
      ssh_opt_.push_back (port_s);
      scp_opt_.push_back ("-p");
      scp_opt_.push_back (port_s);
    }


    // check if we have a context for ssh
    std::vector <saga::context> contexts = p->get_session ().list_contexts ();    
    std::vector <saga::context> ssh_contexts;

    for ( unsigned int i = 0; i < contexts.size (); i++ )
    {
      if ( contexts[i].attribute_exists (saga::attributes::context_type) &&
           contexts[i].get_attribute (saga::attributes::context_type) == "ssh" )
      {
        SAGA_LOG_DEBUG ("found ssh context");
        ssh_contexts.push_back (contexts[i]);
      }
    }
    
    if ( 0 == ssh_contexts.size () )
    {
      // FIXME: isn't a warning enough?  ssh may be configured ok out-of-bound.
      SAGA_LOG_WARN ("no ssh context found for session");
    }

    SAGA_LOG_DEBUG (rm_.get_string ().c_str ())


    // We copy over the ssh identity file of the context we used, so that jobs
    // running on that instance can use it to contact other instances using the
    // same context.  Note that we do _not_ copy the identity files of other
    // contexts, as we don't want to spread credentials beyond their respective
    // universe.
    //
    // FIXME: need to pass location to started SAGA jobs on that host, via some
    // environment variable
    //
    // FIXME: the saga ssh context should evaluate that variable, and try to
    // pick up that identity
    //
    // FIXME: we need to be able to create multiple default ssh contexts
    
    

    for ( unsigned int j = 0; j < ssh_contexts.size (); j++ )
    {
      // try that context
      ctx_ = ssh_contexts[j];

      // FIXME: check if attribs exist
      if ( !     ctx_.attribute_exists ("UserKey")  ||
           "" == ctx_.get_attribute    ("UserKey")  ||
           !     ctx_.attribute_exists ("UserCert") ||
           "" == ctx_.get_attribute    ("UserCert") )
      {
        // _need_ private and public key to be useful
        SAGA_LOG_DEBUG ("context  incomplete");
        break;
      }

      loc_ssh_key_priv_  = ctx_.get_attribute ("UserKey");
      loc_ssh_key_pub_   = ctx_.get_attribute ("UserCert");

      struct passwd * p = ::getpwuid (::getuid ());
      if ( p == NULL )
      {
        local_user_ = "root";
      }
      local_user_ = p->pw_name;
      

      if ( ctx_.attribute_exists ("UserKey") )
      {
        user_ = ctx_.get_attribute ("UserID");
      } 
      else
      {
        user_ = local_user_;
      }


      // the URL may actually have a userid fixed
      if ( "" != rm_.get_userinfo () )
      {
        user_ = rm_.get_userinfo ();
      }


      // determine additional vars used for the environement of started jobs
      // FIXME: the saga jobid leads to invalid command lines and file names
      // try
      // {
      //   saga::job::job self = js_.get_self ();
      //   parent_id_ = self.get_job_id ();
      // }
      // catch ( ... )
      {
        // did not get jobid - invent one
        std::stringstream ss;
        ss << "[saga_parent_id:" << ::getpid () << "]";
        parent_id_ = ss.str ();
      }

      rem_ssh_key_pub_  = std::string ("/tmp/saga_") + parent_id_ + "_ssh.pub";
      rem_ssh_key_priv_ = std::string ("/tmp/saga_") + parent_id_ + "_ssh";


      // prepare the remote host
      saga::adaptors::utils::process proc;

      // NOTE: ssh_spread_keys is disabled - see trac
      // // FIXME: spread keys does not work if no host is given, and
      // // candidate_hosts is used instead.  This code should thus be reused when
      // // running the job, but one needs to keep track of hosts we already copied
      // // to, for performance reasons.
      // if ( ini_["ssh_spread_keys"] == "yes"  ||
      //      ini_["ssh_spread_keys"] == "true" )
      // {

      //   // we don't spread keys to localhost, or if host is not known
      //   if ( ! host_.empty () && 
      //        ! saga::adaptors::utils::is_local_address (host_) )
      //   {
      //     env_.push_back (std::string ("SAGA_PARENT_JOBID") + "=" + parent_id_);
      //     env_.push_back (std::string ("SAGA_SSH_KEY")      + "=" + rem_ssh_key_priv_);
      //     env_.push_back (std::string ("SAGA_SSH_PUB")      + "=" + rem_ssh_key_pub_);
      //     env_.push_back (std::string ("SAGA_SSH_USER")     + "=" + local_user_);


      //     {
      //       // first copy private key
      //       SAGA_LOG_DEBUG (" copying private key");

      //       proc.set_cmd  (scp_bin_);
      //       proc.set_args (scp_opt_);

      //       // FIXME: ensure that context is complete
      //       proc.add_args ("-i", loc_ssh_key_priv_);

      //       // file to stage
      //       // FIXME: ensure that context is complete
      //       // FIXME: we silently assume that the .ssh dirctory exists
      //       // FIXME: the target below SHOULD not exist *aehem*
      //       proc.add_arg (loc_ssh_key_priv_);
      //       proc.add_arg (user_ + "@" + host_ + ":" + rem_ssh_key_priv_);

      //       (void) proc.run_sync ();

      //       if ( ! proc.done () )
      //       {
      //         SAGA_ADAPTOR_THROW ("Could not copy private key", saga::NoSuccess);
      //       }
      //     }

      //     {
      //       // set private key readable only by user
      //       SAGA_LOG_DEBUG (" chmod on private key");

      //       proc.set_cmd  (ssh_bin_);
      //       proc.set_args (ssh_opt_);

      //       // FIXME: ensure that context is complete
      //       proc.add_args ("-i", loc_ssh_key_priv_);

      //       proc.add_arg (user_ + "@" + host_);
      //       proc.add_args ("chmod", "0600" );
      //       proc.add_arg  (rem_ssh_key_priv_);

      //       (void) proc.run_sync ();

      //       if ( ! proc.done () )
      //       {
      //         SAGA_ADAPTOR_THROW ("Could not chmod on private key", saga::NoSuccess);
      //       }
      //     }

      //     {
      //       SAGA_LOG_DEBUG (" copying public key");

      //       // if ok, copy public key
      //       proc.set_cmd  (scp_bin_);
      //       proc.set_args (scp_opt_);

      //       // FIXME: ensure that context is complete
      //       proc.add_args ("-i", loc_ssh_key_priv_);

      //       // file to stage
      //       // FIXME: ensure that context is complete
      //       // FIXME: we silently assume that the .ssh dirctory exists
      //       // FIXME: the target below SHOULD not exist *aehem*
      //       proc.add_arg (loc_ssh_key_pub_);
      //       proc.add_arg (user_ + "@" + host_ + ":" + rem_ssh_key_pub_);

      //       (void) proc.run_sync ();

      //       if ( ! proc.done () )
      //       {
      //         SAGA_ADAPTOR_THROW ("Could not copy public key", saga::NoSuccess);
      //       }
      //     }

      //     {
      //       SAGA_LOG_DEBUG (" register public key");

      //       // append public key to local authorized_keys file, so that
      //       // application can call back home.  A key is exactly one line:
      //       // so we append the key, and then do a sort|uniq on the
      //       // authorized_keys file, to avoid duplicates.

      //       char* home_tmp = ::getenv ("HOME");

      //       if ( home_tmp == NULL )
      //       {
      //         SAGA_ADAPTOR_THROW ("Could not determine home directory", saga::NoSuccess);
      //       }
      //       
      //       std::string home = home_tmp;


      //       // FIXME: use better name
      //       std::fstream cmd;
      //       cmd.open ("/tmp/saga_tmp_cmd", std::fstream::out);

      //       cmd << " cat " << loc_ssh_key_pub_ 
      //           << " "     << home << "/.ssh/authorized_keys"
      //           << " | sort | uniq > /tmp/saga_keys_tmp" 
      //           << std::endl
      //           << "  mv /tmp/saga_keys_tmp " 
      //           << home << "/.ssh/authorized_keys" 
      //           << std::endl;

      //       cmd.close();

      //       ::chmod ("/tmp/saga_tmp_cmd", S_IRWXU);

      //       proc.set_cmd     ("/bin/sh");
      //       proc.clear_args  ();
      //       proc.add_args    ("-c", "/tmp/saga_tmp_cmd");

      //       (void) proc.run_sync ();

      //       if ( ! proc.done () )
      //       {
      //         SAGA_ADAPTOR_THROW ("Could not register public key", saga::NoSuccess);
      //       }

      //       // remove temporary script
      //       ::unlink ("/tmp/saga_tmp_cmd");

      //     }
      //   } // empty or local host_
      // }
      // else
      if ( ini_["ssh_test_remote"] == "yes"  ||
           ini_["ssh_test_remote"] == "true" )
      {
        // we don't test if host is not known
        if ( ! host_.empty () )
        {
          SAGA_LOG_DEBUG (" running ssh test");

          proc.set_cmd  (ssh_bin_);
          proc.set_args (ssh_opt_);

          // FIXME: ensure that context is complete
          proc.add_args ("-i", loc_ssh_key_priv_);
          proc.add_arg  (      user_ + "@" + host_);
          proc.add_arg  ("true");

          (void) proc.run_sync ();

          if ( ! proc.done () )
          {
            // SAGA_ADAPTOR_THROW ("Could not run a test ssh command", saga::NoSuccess);
            std::stringstream ss;
            ss << "Cannot execute jobs on remote host (" << proc.get_err_s () << ")";
            SAGA_ADAPTOR_THROW (ss.str (), saga::NoSuccess);
          }
        }
      }

      // we keep this context as valid for the host.
      // s_.add_context (ctx_);

      // we are done - no exception 'til now!
      return;
    }

    // no context was ok for scp or ssh, or preparation failed - flag error
    // FIXME: throw above when error occurs, with better error message
    SAGA_ADAPTOR_THROW ("Could not connect to remote host", saga::NoSuccess);
  }

  // destructor
  job_service_cpi_impl::~job_service_cpi_impl (void)
  {
  }

  //////////////////////////////////////////////////////////////////////
  // SAGA API functions
  void job_service_cpi_impl::sync_create_job (saga::job::job         & ret, 
                                              saga::job::description   jd)
  {
    SAGA_LOG_DEBUG ("SSH Create Job");
    SAGA_LOG_DEBUG (rm_.get_string ().c_str ());

    // the ssh job adaptor sets a couple of env variables:
    //   SAGA_PARENT_JOBID: saga job id of self, i.e. the spawning process
    //   SAGA_SSH_KEY:      location of private ssh key used to spawn process
    //   SAGA_SSH_PUB:      location of public  ssh key used to spawn process
    //   SAGA_SSH_USER:     user which spawned the process
    std::vector <std::string> new_env;
    if ( jd.attribute_exists (saga::job::attributes::description_environment) )
    {
      new_env = jd.get_vector_attribute (saga::job::attributes::description_environment);
    }

    for ( unsigned int i = 0; i < env_.size (); i++ )
    {
      new_env.push_back (env_[i]);
    }

    jd.set_vector_attribute (saga::job::attributes::description_environment, new_env);

    // create the job with the 'fixed' job description
    saga::job::job job = saga::adaptors::job (rm_, jd, s_);
    ret = job;
  }

  void job_service_cpi_impl::sync_run_job (saga::job::job     & ret, 
                                           std::string          cmd, 
                                           std::string          host, 
                                           saga::job::ostream & in, 
                                           saga::job::istream & out, 
                                           saga::job::istream & err)
  {
    // we rely on the package fallback
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_service_cpi_impl::sync_list (std::vector <std::string> & ret)
  {
    std::vector <std::string> tmp = js_.list ();

    adaptor_data adata (this);

    for ( unsigned int i = 0; i < tmp.size (); i++ )
    {
      // FIXME: we should grep for ssh jobs

      // translate jobids from '[fork]-...' to '[ssh]-...' 
      ret.push_back (adata->translate_jobid (tmp[i]));
    }
  }


  void job_service_cpi_impl::sync_get_job (saga::job::job & ret,
                                           std::string      jobid)
  {
    instance_data idata (this);

    // we do *not* translate the job id - that will be taken care of by the job
    // ctor (using the init_from_jobid path)
    ret = saga::adaptors::job (idata->rm_,
                               jobid,
                               proxy_->get_session ());
  }
  


  void job_service_cpi_impl::sync_get_self (saga::job::self & ret)
  {
    // the ssh adaptor can never implement job::self
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_service_cpi_impl::dump_context_ (saga::context c)
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

  void job_service_cpi_impl::check_ini_ (void)
  {
    // check if ini has required entries
    if ( ini_.find ("ssh_bin") == ini_.end () ||
         ini_["ssh_bin"]       == ""          )
    {
      SAGA_ADAPTOR_THROW_NO_CONTEXT ("need path to ssh_bin specified in the SAGA ini",
                                     saga::NoSuccess);
    }

    if ( ini_.find ("scp_bin") == ini_.end () ||
         ini_["scp_bin"]       == ""          )
    {
      SAGA_ADAPTOR_THROW_NO_CONTEXT ("need path to scp specified in the SAGA ini",
                                     saga::NoSuccess);
    }

    // set default opts (none)
    if ( ini_.find ("ssh_opt") == ini_.end () )
    {
      ini_["ssh_opt"] = "";
    }

    if ( ini_.find ("scp_opt") == ini_.end () )
    {
      // set default opts (none)
      ini_["scp_opt"] = "";
    }

    ssh_bin_   = ini_["ssh_bin"];
    scp_bin_   = ini_["scp_bin"];
    ssh_opt_   = saga::adaptors::utils::split (ini_["ssh_opt"], ' ');
    scp_opt_   = saga::adaptors::utils::split (ini_["scp_opt"], ' ');


    // NOTE: ssh_spread_keys is disabled - see trac
    // // set sensible default options
    // if ( ini_.find ("ssh_spread_keys") == ini_.end () )
    // {
    //   ini_["ssh_spread_keys"] = "false";
    // }

    if ( ini_.find ("ssh_test_remote") == ini_.end () )
    {
      ini_["ssh_test_remote"] = "true";
    }
  }

} // namespace ssh_job
////////////////////////////////////////////////////////////////////////

