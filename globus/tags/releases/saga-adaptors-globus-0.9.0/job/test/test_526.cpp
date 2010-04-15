#include <saga/saga.hpp>

using namespace std;

int main(int argc, char** argv)
{

try 
{
    std::cout << "If #526 is not fixed, the code below will produce at least an exception. " << std::endl;
    std::cout << "Please check: http://faust.cct.lsu.edu/trac/saga/ticket/526 for details." << std::endl;

    saga::url js_url("gram://qb1.loni.org/");

    saga::job::service js(js_url);

    saga::job::description jd;
    jd.set_attribute(saga::job::attributes::description_executable, "/bin/hostname");
    jd.set_attribute(saga::job::attributes::description_output, "myjob.out");

    vector<string> ft;
    ft.push_back("myjob.out < myjob.out");
    jd.set_vector_attribute(saga::job::attributes::description_file_transfer, ft);

    saga::job::job j = js.create_job(jd);
    j.run();
    j.wait();

    std::cout << "\nPASSED!" << std::endl;
}
catch(saga::exception const & e)
{
    std::cerr << "\nFAILED: " << e.what() << std::endl;
}

}
