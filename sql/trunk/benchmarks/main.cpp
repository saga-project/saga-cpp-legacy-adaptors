#include <saga/advert.hpp>
#include <boost/timer.hpp>
#include <boost/progress.hpp>
#include <fstream>
#include <sys/time.h>

////////////////////////////
//// Global Vars          //
////////////////////////////

std::string advert_server_root("advert://SAGA:SAGA_client@cyder.cct.lsu.edu:8080/SagaAdvertPerfTest");
//std::string advert_server_root("advert://localhost/");

timeval start_time_val;
timeval stop_time_val;

///////////////////////////////
// Timing Helper functions   //
///////////////////////////////

void start_timer()
{
	gettimeofday(&start_time_val, 0);
}

void stop_timer()
{
	gettimeofday(&stop_time_val, 0);
}

long elapsed_msec()
{
	long seconds	= stop_time_val.tv_sec - start_time_val.tv_sec;
	long useconds	= stop_time_val.tv_usec - start_time_val.tv_usec;
	
	// ceiled milliseconds 
	return (seconds * 1000 + useconds/1000.0) + 0.5;
}

///////////////////////////
// DB Helper functions   //
//////////////////////////

void init_db()
{
	saga::url root_url(advert_server_root);
	saga::advert::directory root_dir(root_url, saga::advert::Create);
}


void clear_db()
{
	saga::url root_url(advert_server_root);
	saga::advert::directory root_dir(root_url);
	
	std::vector<saga::url> url_vector = root_dir.list();
	for (std::vector<saga::url>::iterator i = url_vector.begin(); i != url_vector.end(); ++i)
	{
		root_dir.remove(*i, saga::advert::Recursive);
	}
									 
}


saga::url create_url(int depth)
{
	std::string url_string(advert_server_root);
	
	for (int i = 0; i < depth; ++i)
	{
		url_string = url_string + "/directory_" + boost::lexical_cast<std::string>(i);
	}
	
	return url_string;
}

void create_attributes(saga::advert::directory dir, int attribute_count)
{
	for (int i = 0; i < attribute_count; ++i)
	{
		dir.set_attribute(std::string("attribute_key_") + boost::lexical_cast<std::string>(i), std::string("attribute_value_") + boost::lexical_cast<std::string>(i));
	}
}

//////////////////////////////////
// Preformance tests            //
/////////////////////////////////

void directory_create_test(int max_depth)
{
	// start with a fresh DB
	clear_db();
	
	
	std::ofstream outfile("directory_create_test.csv");
	boost::progress_display show_progress(max_depth);
	
	for (int i = 0; i < max_depth; ++i)
	{
		saga::url url = create_url(i);
		
		start_timer();
			saga::advert::directory::create(url, saga::advert::CreateParents);
		stop_timer();
		
		outfile << boost::lexical_cast<std::string>(i) << " " << boost::lexical_cast<std::string>(elapsed_msec()) << std::endl;
		++show_progress;
	}
	
	outfile.close();
}

void attribute_create_test(int max_depth, int attribute_count)
{
	// start with a fresh DB
	clear_db();
	
	std::ofstream outfile("attribute_create_test.csv");
	boost::progress_display show_progress(max_depth);
	
	for (int i = 0; i < max_depth; ++i)
	{
		saga::url url = create_url(i);
		
		start_timer();
			saga::advert::directory dir(url, saga::advert::CreateParents | saga::advert::ReadWrite);
			create_attributes(dir, attribute_count);
		stop_timer();
		
		outfile << boost::lexical_cast<std::string>(i) << " " << boost::lexical_cast<std::string>(elapsed_msec()) << std::endl;
		++show_progress;
	}
	
	outfile.close();
}

void directory_read_test(int max_depth)
{
	// start with a fresh DB
	clear_db();
	
	// create the DB to run test on 
	saga::url url = create_url(max_depth);
	saga::advert::directory::create(url, saga::advert::CreateParents);
	
	
	// perform the test 
	std::ofstream outfile("directory_read_test.csv");
	boost::progress_display show_progress(max_depth);
	
	for (int i = 0; i < max_depth; ++i)
	{
		saga::url url = create_url(i);
		
		start_timer();
			saga::advert::directory dir(url);
		stop_timer();
		
		outfile << boost::lexical_cast<std::string>(i) << " " << boost::lexical_cast<std::string>(elapsed_msec()) << std::endl;
		++show_progress;	
	}
	
	outfile.close();
}

void attribute_read_test(int max_depth, int attribute_count)
{
	// start with a fresh DB
	clear_db();
	
	// create the DB to run test on
	for (int i = 0; i < max_depth; ++i)
	{
		saga::url url = create_url(i);
		saga::advert::directory dir(url, saga::advert::CreateParents | saga::advert::ReadWrite);
		
		create_attributes(dir, attribute_count);
	}
	
	// perform test
	std::ofstream outfile("attribute_read_test.csv");
	boost::progress_display show_progress(max_depth);

	for (int i = 0; i < max_depth; ++i)
	{
		saga::url url = create_url(i);
		
		start_timer();
			saga::advert::directory dir(url);
		
			std::vector<std::string> attribute_kyes = dir.list_attributes();
			for (std::vector<std::string>::iterator j = attribute_kyes.begin(); j != attribute_kyes.end(); ++j)
			{
				std::string attribute = dir.get_attribute(*j);
			
			}
		stop_timer();
		
		outfile << boost::lexical_cast<std::string>(i) << " " << boost::lexical_cast<std::string>(elapsed_msec()) << std::endl;
		++show_progress;	
	}
	
	outfile.close();	
}

////////////////////////////
/// Main                  //
////////////////////////////

int main (int argc, char * const argv[])
{	
	init_db();
	
	std::cout << std::endl << "Starting directory create test ... " << std::endl << std::endl;
	directory_create_test(50);
	
	std::cout << std::endl << std::endl << "Starting attribute create test ..." << std::endl << std::endl;
	attribute_create_test(50, 10);
	 
	std::cout << std::endl << std::endl << "Starting directory read test ..." << std::endl << std::endl;
	directory_read_test(50);

	std::cout << std::endl << std::endl << "Starting attribute read test ..." << std::endl << std::endl;
	attribute_read_test(50, 10);
	
	return 0;
}

