#include <saga/saga.hpp>

int main(int argc, char** argv)
{

try 
{
    std::cout << "If #535 is not fixed, the code below will produce at least an exception. " << std::endl;
    std::cout << "Please check: http://faust.cct.lsu.edu/trac/saga/ticket/535 for details." << std::endl;

    // create a non-empty directory
    saga::url dir_url("gridftp://qb1.loni.org/tmp/ole_test");
    saga::filesystem::directory dir(dir_url, saga::filesystem::Create);
    dir.open_dir("another_dir", saga::filesystem::Create);
    dir.open_dir("and_another", saga::filesystem::Create);	
    

    //try to move it
    //saga::url from("gridftp://qb1.loni.org/tmp/ole_test");
    saga::url to("gridftp://qb1.loni.org/tmp/ole_test_copy_target");
    
    dir.move(to);

    std::cout << "\nPASSED!" << std::endl;
}
catch(saga::exception const & e)
{
    std::cerr << "\nFAILED: " << e.what() << std::endl;
}

}
