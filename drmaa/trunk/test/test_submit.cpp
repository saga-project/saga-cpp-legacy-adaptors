#include <saga/saga.hpp>
#include <vector>

namespace sja = saga::job::attributes;

int run_test (std::string       name)
{
  int err = 0;

  try
  {

    saga::session s;

    saga::job::service     js (s, "drmaa://localhost");
    saga::job::description jd;

    jd.set_attribute        (sja::description_executable, "/bin/sleep");

    std::vector<std::string> args;
    args.push_back("10");
    jd.set_vector_attribute (sja::description_arguments, args);

    jd.set_attribute (sja::description_output, "output");

    // not supported, yet
    //std::vector <std::string> transfers;
    //transfers.push_back ("file://localhost/tmp/output < output");
    //jd.set_vector_attribute (sja::description_file_transfer, transfers);


    saga::job::job j = js.create_job (jd);

    j.run ();

    std::cout << name << ": Submitted" << std::endl;
    
    // j.wait (-1.0);

    saga::job::state state = j.get_state ();

    while ( state == saga::job::New )
    {
      std::cout << name << ": New" << std::endl;
      ::sleep (1);
      state = j.get_state ();
    }

    while ( state == saga::job::Running )
    {
      std::cout << name << ": Running" << std::endl;
      ::sleep (1);
      state = j.get_state ();
    }

    if ( state == saga::job::Done )
    {
      std::cout << name << ": Done" << std::endl;
    }
    else
    {
      std::cout << name << ": Failed?" << std::endl;
      err++;
    }
  }
  catch ( const saga::exception & e )
  {
    std::cout << name << ": Exception: " << e.what () << std::endl;
    err++;
  }
  catch ( const char * m )
  {
    std::cout << name << ": exception: " << m << std::endl;
    err++;
  }

  std::cout <<  " ----- " << err << " ---------------------------------------------------------- " << std::endl;

  return err;
}

int main (int argc, char** argv)
{
  run_test ("Simple Test1");

  return 0;
}

