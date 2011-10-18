#include <saga/saga.hpp>

using namespace std;

int main(int argc, char** argv)
{

try 
{
    std::cout << "Tests if exception gets thrown in case NumberOfProcesses" 
              << " is used without ProcessesPerHost and vice versa. " 
              << std::endl;
              
    std::cout << "This is somewhat part of ticket: "
              << "http://faust.cct.lsu.edu/trac/saga/ticket/587" 
              << std::endl;

    saga::url js_url("pbspro://localhost/");

    saga::job::service js(js_url);

    saga::job::description jd_nop_no_pph;
    
    jd_nop_no_pph.set_attribute(
      saga::job::attributes::description_executable, "/bin/hostname");
      
    jd_nop_no_pph.set_attribute(
      saga::job::attributes::description_output, "myjob.out");

    jd_nop_no_pph.set_attribute(
      saga::job::attributes::description_number_of_processes, "8");

    saga::job::job j = js.create_job(jd_nop_no_pph);
    j.run();
    j.wait();

    std::cout << "\nFAILED!" << std::endl;
}
catch(saga::exception const & e)
{
    std::cerr << "\nPASSED(check output for PBS adaptor exception): " 
              << e.what() << std::endl;
}

try 
{
    saga::url js_url("pbspro://localhost/");

    saga::job::service js(js_url);

    saga::job::description jd_no_nop_pph;
    
    jd_no_nop_pph.set_attribute(
      saga::job::attributes::description_executable, "/bin/hostname");
      
    jd_no_nop_pph.set_attribute(
      saga::job::attributes::description_output, "myjob.out");

    jd_no_nop_pph.set_attribute(
      saga::job::attributes::description_processes_per_host , "8");

    saga::job::job j = js.create_job(jd_no_nop_pph);
    j.run();
    j.wait();

    std::cout << "\nFAILED!" << std::endl;
}
catch(saga::exception const & e)
{
    std::cerr << "\nPASSED (check output for PBS adaptor exception): " 
              << e.what() << std::endl;
}

try
{
    saga::url js_url("pbspro://localhost/");

    saga::job::service js(js_url);

    saga::job::description jd;

    jd.set_attribute(
      saga::job::attributes::description_executable, "/bin/hostname");

    jd.set_attribute(
      saga::job::attributes::description_output, "myjob.out");

    jd.set_attribute(
      saga::job::attributes::description_processes_per_host , "2");
    jd.set_attribute(
      saga::job::attributes::description_number_of_processes, "4");

    saga::job::job j = js.create_job(jd);
    j.run();
    j.wait();

    std::cout << "\nPASSED!" << std::endl;
}
catch(saga::exception const & e)
{
    std::cerr << "\nFAILED (check output for PBS adaptor exception): "
              << e.what() << std::endl;
}


}
