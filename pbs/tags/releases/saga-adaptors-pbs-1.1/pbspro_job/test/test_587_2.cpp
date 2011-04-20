#include <saga/saga.hpp>

using namespace std;

int main(int argc, char** argv)
{

try 
{
    std::cout << "This is somewhat part of ticket: "
              << "http://faust.cct.lsu.edu/trac/saga/ticket/587" 
              << std::endl;

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

    jd.set_attribute(
      saga::job::attributes::description_queue, "workq");

    std::vector<std::string> j_p;
    j_p.push_back("MY_SECRET_JOB_PROJECT");
    jd.set_vector_attribute(
      saga::job::attributes::description_job_project, j_p);


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
