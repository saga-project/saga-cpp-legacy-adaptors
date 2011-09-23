#include <saga/saga.hpp>

using namespace std;

int main(int argc, char** argv)
{

try 
{
    std::cout << "This tests expects the GRAM adaptor to throw a BadParameter exception. " << std::endl;
    std::cout << "This didn't happen in the past..." << std::endl;

    saga::url js_url("gram://qb1.loni.org/jobmanager-pbs");

    saga::job::service js(js_url);

    saga::job::description jd;
    jd.set_attribute(saga::job::attributes::description_executable, "/bin/hostname");
    jd.set_attribute(saga::job::attributes::description_queue, "nonexisting");

    //vector<string> ft;
    //ft.push_back("myjob.out < myjob.out");
    //jd.set_vector_attribute(saga::job::attributes::description_file_transfer, ft);

    saga::job::job j = js.create_job(jd);
    j.run();
    j.wait();

    std::cerr << "\nFAILED: Exception expected "  << std::endl;

}
catch(saga::exception const & e)
{
	std::cout << e.what() << std::endl;
	std::cout << "\nPASSED!" << std::endl;
}


}
