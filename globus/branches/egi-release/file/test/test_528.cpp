#include <saga/saga.hpp>

int main(int argc, char** argv)
{

try 
{
    std::cout << "If #528 is not fixed, the code below will produce at least an exception. " << std::endl;
    std::cout << "Please check: http://faust.cct.lsu.edu/trac/saga/ticket/528 for details." << std::endl;

    saga::url dir_url("gridftp://qb1.loni.org/home/oweidner");
    saga::filesystem::directory dir(dir_url);

	std::vector<saga::url> entries = dir.list();
	std::vector<saga::url>::const_iterator cit = entries.begin();
	
	while(cit != entries.end())
	{
		std::string::size_type idx;
        idx = (*cit).get_url().find("..");
        if( idx != std::string::npos )
        {
            std::cout << "FAILED: found entry " << (*cit) << std::endl;
        }

		++cit;
	}
	
    std::cout << "\nPASSED!" << std::endl;
}
catch(saga::exception const & e)
{
    std::cerr << "\nFAILED: " << e.what() << std::endl;
}

}
