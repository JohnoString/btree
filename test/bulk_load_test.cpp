//  btree/test/bulk_load_test.cpp  -----------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#include "../volume_test/data.hpp"
#include <boost/btree/bulk_load.hpp>
#include <boost/timer/timer.hpp>
#include <boost/filesystem.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <locale>
#include <iostream>
#include <string>
#include <cstdlib>  // for atoll() or Microsoft equivalent
#include <cctype>   // for isdigit()
#ifndef _MSC_VER
# define BOOST_BTREE_ATOLL std::atoll
#else
# define BOOST_BTREE_ATOLL _atoi64
#endif

using namespace boost::btree;
using std::cout; using std::endl;

/*
TODO:

* need to specify directory for temporary files


*/

namespace
{
  boost::filesystem::path source_path;
  boost::filesystem::path btree_path;
  boost::filesystem::path temp_path(boost::filesystem::temp_directory_path());
  std::size_t max_memory_megabytes = 1000U;
  const std::size_t one_megabyte = 1000000U;
  std::size_t max_memory = max_memory_megabytes * one_megabyte;

  std::string command_args;
  int64_t log_point = 0;   // if != 0, log_point every lg iterations

  char thou_separator = ',';
  bool verbose (false);
  const int places = 2;
  std::string path("data.dat");

  std::ofstream out;

  const double sec = 1000000000.0;

  struct thousands_separator : std::numpunct<char> { 
   char do_thousands_sep() const { return thou_separator; } 
   std::string do_grouping() const { return "\3"; }
};
}

int cpp_main(int argc, char* argv[]) 
{
  for (int a = 0; a < argc; ++a)
  {
    command_args += argv[a];
    if (a != argc-1)
      command_args += ' ';
  }
  cout << command_args << '\n';
  const int req = 3;

  if (argc >= req)
  {
    source_path = argv[1];
    btree_path = argv[2];

    for (; argc > req; ++argv, --argc) 
    {
      if (*argv[req] != '-')
        temp_path = argv[req];
      else
      {
        if ( *(argv[req]+1) == 'm' && std::isdigit(*(argv[req]+2)) )
        {
          max_memory_megabytes = BOOST_BTREE_ATOLL( argv[req]+2 );
          max_memory = max_memory_megabytes * one_megabyte;
        }
        else if ( *(argv[req]+1) == 'l' && std::isdigit(*(argv[req]+2)) )
          log_point = BOOST_BTREE_ATOLL( argv[req]+2 );
        else if ( std::strncmp( argv[req]+1, "sep", 3 )==0
            && (std::ispunct(*(argv[req]+4)) || *(argv[req]+4)== '\0') )
          thou_separator = *(argv[req]+4) ? *(argv[req]+4) : ' ';
        else
        {
          cout << "Error - unknown option: " << argv[req] << "\n\n";
          argc = -1;
          break;
        }
      }
    }
  }

  if (argc < 3) 
  {
    cout << "Usage: bulk_load_test source-path target-path [Options]\n"
      "   source-path  File of binary key/mapped data pairs; must exist\n"
      "   target-path  BTree file the source data pairs will be inserted\n"
      "                into; error if already exists\n"
      " Options:\n"
      "   temp-path    Directory for temporary files; default " << temp_path << "\n"
      "   -m#          Maximum memory # megabytes; default " <<
                       max_memory_megabytes <<"\n"
      "   -l#          Log progress every # actions; default is no such logging\n"
      "   -sep[punct]  Thousands separator; space if punct omitted, default -sep,\n"
// TODO:      "   -v       Verbose output statistics\n"
      ;
    return 1;
  }

  cout.imbue(std::locale(std::locale(), new thousands_separator));
  boost::timer::auto_cpu_timer t(3);

  //bulk_load_map<uint32_t, uint32_t> map;    // KISS
  bulk_load_map<volume::u128_t, uint64_t> map;    
  map(source_path, btree_path, temp_path, cout, max_memory, log_point, flags::truncate);

  t.stop();
  t.report();

  return 0;
}