
#include "aws_helper.hpp"

// for the throw macro...
#include <saga/saga/object.hpp>
#include <saga/impl/exception.hpp>

namespace aws_job
{
  process::process (void)
    : cmd_  (""), 
      done_ (false),
      fail_ (false)
  {
  }

  process::process (std::string cmd)
    : cmd_  (cmd), 
      done_ (false),
      fail_ (false)
  {
  }

  process::process (std::vector <std::string> args)
    : args_ (args),
      done_ (false),
      fail_ (false)
  {
  }

  process::process (std::map    <std::string, std::string> env)
    : env_  (env), 
      done_ (false),
      fail_ (false)
  {
  }

  process::process (std::string               cmd, 
                    std::vector <std::string> args)
    : cmd_  (cmd), 
      args_ (args),
      done_ (false),
      fail_ (false)
  {
  }

  process::process (std::string                            cmd, 
                    std::map    <std::string, std::string> env)
    : cmd_  (cmd), 
      env_  (env), 
      done_ (false),
      fail_ (false)
  {
  }

  process::process (std::string                            cmd, 
                    std::vector <std::string>              args, 
                    std::map    <std::string, std::string> env)
    : cmd_  (cmd), 
      args_ (args),
      env_  (env), 
      done_ (false),
      fail_ (false)
  {
  }

  void process::clear (void)
  {
    cmd_  = "";

    env_.clear ();
    args_.clear ();

    msg_   = "";
    fail_  = false;
    done_  = false;

    out_v_.clear ();
    err_v_.clear ();
  }

  void process::set_cmd (std::string cmd)
  {
    cmd_= cmd;
  }

  void process::clear_args (void)
  {
    args_.clear ();
  }

  void process::set_args (std::vector <std::string> args)
  {
    args_ = args;
  }

  void process::add_args (std::string arg_1, 
                          std::string arg_2)
  {
    args_.push_back (arg_1);
    args_.push_back (arg_2);
  }

  void process::add_arg (std::string arg)
  {
    args_.push_back (arg);
  }

  void process::clear_env (void)
  {
    env_.clear ();
  }

  void process::set_env (std::map <std::string, std::string> env)
  {
    env_ = env;
  }

  void process::add_env (std::string key, std::string val)
  {
    env_[key] = val;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  // run a process, and return stdout.  In case of errors, throw a NoSuccess with
  // stderr as error message
  //
  std::vector <std::string> process::run_sync (bool io)
  {
    try 
    {
      fail_ = false;
      done_ = false;

      clear_out ();
      clear_err ();

      
      saga::job::description jd;


      if ( cmd_ == "")
      {
        std::cout << " === no cmd\n";
        fail_ = true;
        msg_  = "no command to run";

        std::vector <std::string> out;
        return out;
      }

      jd.set_attribute (saga::job::attributes::description_executable, cmd_);


      std::vector <std::string> env;

      std::map <std::string, std::string> :: iterator it;

      for ( it = env_.begin (); it != env_.end (); it++ )
      {
        std::string var;

        var += (*it).first;
        var += "=";
        var += (*it).second;

        env.push_back   (var);
      }

      jd.set_vector_attribute (saga::job::attributes::description_environment,  env);
      jd.set_vector_attribute (saga::job::attributes::description_arguments,  args_);

      

      if ( io )
      {
        jd.set_attribute (saga::job::attributes::description_interactive,
                          saga::attributes::common_true);
      }
      else
      {
        jd.set_attribute (saga::job::attributes::description_interactive,
                          saga::attributes::common_false);
      }


      saga::job::service js ("fork://localhost");
      saga::job::job     j   = js.create_job (jd);

      saga::job::istream out;
      saga::job::istream err;

      if ( io )
      {
        out = j.get_stdout ();
        err = j.get_stderr ();
      }


      j.run ();


      if ( io )
      {
        std::string line;

        while ( ! out.fail () ) 
        {
          char c;

          out.read (&c, 1);

          if ( out.fail () ) 
            break; 

          if ( c == '\n' )
          {
            out_v_.push_back (line);
            line = "";
          }
          else
          {
            line += c;
          }
        }

        if ( line != "" ) 
        { 
          out_v_.push_back (line); 
          line = "";
        }


        while ( ! err.fail () ) 
        {
          char c;

          err.read (&c, 1);

          if ( err.fail () ) 
            break; 

          if ( c == '\n' )
          {
            err_v_.push_back (line);
            line = "";
          }
          else
          {
            line += c;
          }
        }

        if ( line != "" ) 
        {
          err_v_.push_back (line); 
          line = "";
        }
      }



      j.wait ();


      if ( j.get_state () != saga::job::Done )
      {
        fail_ = true;
        msg_  = "exit status != 0";

        if ( msg_ != "" )
        {
          SAGA_LOG_ALWAYS (msg_.c_str ());
        }

        for ( unsigned int i = 0; i < err_v_.size (); i++ )
        {
          SAGA_LOG_ALWAYS (err_v_[i].c_str ());
        }
      }
      else
      {
        done_ = true;
      }
    }
    catch ( const saga::exception & e )
    {
      SAGA_THROW_NO_OBJECT (e.what (), saga::NoSuccess);
    }

    return out_v_;
  }


  std::string process::get_out_s (void)
  {
    std::string out;

    for ( unsigned int i = 0; i < out_v_.size (); i++ )
    {
      out += out_v_[i] + "\n";
    }

    return out;
  }

  std::string process::get_err_s (void)
  {
    std::string err;

    for ( unsigned int i = 0; i < err_v_.size (); i++ )
    {
      err += err_v_[i] + "\n";
    }

    return err;
  }


  std::vector <std::string>  process::get_out_v (void)
  {
    return out_v_;
  }

  std::vector <std::string>  process::get_err_v (void)
  {
    return err_v_;
  }

  void process::clear_out (void)
  {
    out_v_.clear ();
  }

  void process::clear_err (void)
  {
    err_v_.clear ();
  }
  ///////////////////////////////////////////////////////////////////////////////

} // aws_job

