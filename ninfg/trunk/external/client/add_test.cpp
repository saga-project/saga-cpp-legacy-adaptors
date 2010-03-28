#include <saga/saga.hpp>
#include <iostream>
#include <vector>

int main (int argc, char **argv)
{
  // initialize the rpc handle
  saga::rpc::rpc rm (std::string ("rpc://sakura.hpcc.jp/simple/add_test"));

  // initialize the parameter stack
  int input = 100, output = -1;
  saga::rpc::parameter param1 (&input, 1, saga::rpc::In);
  saga::rpc::parameter param2 (&output, 1, saga::rpc::Out);

  std::vector <saga::rpc::parameter> parameters;
  parameters.push_back (param1);
  parameters.push_back (param2);

  // invoke the remote procedure
  rm.call (parameters);

  // when completed, the output parameters are available
  std::cout << "Output: " << output << std::endl;

  return (0);
}
