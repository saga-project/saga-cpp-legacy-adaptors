
#ifndef AWS_JOB_AWS_HELPER_HPP
#define AWS_JOB_AWS_HELPER_HPP

#include <saga/saga.hpp>

namespace aws_job
{
  class process 
  {
    private:
      std::string                            cmd_;
      std::vector <std::string>              args_;
      std::map    <std::string, std::string> env_;

      std::vector <std::string> out_v_;
      std::vector <std::string> err_v_;

      bool        done_;
      bool        fail_;

      std::string msg_;


    public:
      process (void);

      process (std::string                            cmd); 
      process (std::vector <std::string>              args);
      process (std::map    <std::string, std::string> env);

      process (std::string                            cmd, 
               std::vector <std::string>              args);
      process (std::string                            cmd, 
               std::map    <std::string, std::string> env);

      process (std::string                            cmd, 
               std::vector <std::string>              args, 
               std::map    <std::string, std::string> env);

      void clear      (void);
      void set_cmd    (std::string cmd);

      void clear_args (void);
      void set_args   (std::vector <std::string> args);
      void add_arg    (std::string arg);
      void add_args   (std::string arg_1, 
                       std::string arg_2);

      void clear_env  (void);
      void set_env    (std::map <std::string, std::string> env);
      void add_env    (std::string key, std::string val);

      std::vector <std::string>  run_sync  (bool io = true);

      void                       clear_out (void);
      void                       clear_err (void);

      std::string                get_out_s (void);
      std::string                get_err_s (void);

      std::vector <std::string>  get_out_v (void);
      std::vector <std::string>  get_err_v (void);

      bool        done (void) { return done_; }
      bool        fail (void) { return fail_; }
      std::string msg  (void) { return msg_;  }
  };
}

#endif // AWS_JOB_AWS_HELPER_HPP

