#include <saga/saga.hpp>

int main(int argc, char** argv)
{

try 
{
    std::cout << "If #546 is not fixed, the code below will produce at least an exception. " << std::endl;
    std::cout << "Please check: http://faust.cct.lsu.edu/trac/saga/ticket/546 for details." << std::endl;

    saga::url dir_url("gridftp://qb1.loni.org/home/");
    saga::filesystem::directory dir(dir_url, saga::filesystem::Create);

   	std::cout << "# entries: " << dir.get_num_entries() << std::endl;

    std::cout << "\nPASSED!" << std::endl;
}
catch(saga::exception const & e)
{
    std::cerr << "\nFAILED: " << e.what() << std::endl;
}

}
