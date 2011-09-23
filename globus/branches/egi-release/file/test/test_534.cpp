#include <saga/saga.hpp>

int main(int argc, char** argv)
{

try 
{
    std::cout << "If #534 is not fixed, the code below will produce at least an exception. " << std::endl;
    std::cout << "Please check: http://faust.cct.lsu.edu/trac/saga/ticket/534 for details." << std::endl;

    saga::url dir_url("gridftp://qb1.loni.org/home/oweidner");
    saga::name_space::directory ns_dir(dir_url);

	saga::name_space::directory ns_dir2 = ns_dir.open_dir(".ssh");
	saga::name_space::entry ns_entry = ns_dir2.open("known_hosts");
	
    std::cout << "\nPASSED!" << std::endl;
}
catch(saga::exception const & e)
{
    std::cerr << "\nFAILED: " << e.what() << std::endl;
}

}
