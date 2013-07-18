//  index_test.cpp  --------------------------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//--------------------------------------------------------------------------------------//
//                                                                                      //
//  These tests lightly exercise many portions of the interface. They do not attempt    //
//  to stress the many combinations of control paths possible in large scale use.       //
//                                                                                      //
//--------------------------------------------------------------------------------------//

#include <boost/btree/index.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <boost/detail/lightweight_test.hpp>

#include <iostream>
#include <iomanip>
#include <utility>
#include <set>
#include <algorithm>

using namespace boost;
namespace fs = boost::filesystem;
using std::cout;
using std::endl;
using std::make_pair;

namespace
{
bool dump_dot(false);

filesystem::path file_path("test.file");
filesystem::path idx1_path("test.1.idx");
filesystem::path idx2_path("test.2.idx");

struct stuff
{
  int x;
  int y;
  char unused[24];

  stuff() : x(-1), y(-2) {}
  stuff(int x_, int y_) : x(x_), y(y_) {}
  stuff(const stuff& rhs) : x(rhs.x), y(rhs.y) {}
  stuff& operator=(const stuff& rhs) {x = rhs.x; y = rhs.y; return *this; }
  stuff& assign(int x_, int y_) {x = x_; y = y_; return *this; }

  bool operator<(const stuff& rhs) const
    {return x < rhs.x || (x == rhs.x && y < rhs.y);}
  bool operator==(const stuff& rhs) const {return x == rhs.x && y == rhs.y;}
  bool operator!=(const stuff& rhs) const {return x != rhs.x || y != rhs.y;}
};

std::ostream& operator<<(std::ostream& os, const stuff& s)
{
  os << s.x << "," << s.y;
  return os;
}

struct stuff_reverse_order
{
  bool operator()(const stuff& lhs, const stuff& rhs) const
  {
    return rhs<lhs;
  }
};

//-------------------------------------- instantiate -----------------------------------//

void instantiate_test()
{
  cout << "  instantiate_test..." << endl;

  // set
  {
    btree::btree_index<int> x;
    BOOST_TEST(!x.is_open());
    BOOST_TEST(x.index_size() == 0U);
    BOOST_TEST(x.index_empty());
  }
 
  cout << "    instantiate_test complete" << endl;
}


//---------------------------------  open_all_new_test  --------------------------------//

void  open_all_new_test()
{
  cout << "  open_all_new_test..." << endl;

  {
    cout << "      default construct, then open..." << endl;
    btree::btree_index<int> idx;
    idx.open(file_path, 1000000u, idx1_path, btree::flags::truncate, -1, 128);
    BOOST_TEST(idx.is_open());
    BOOST_TEST_EQ(idx.file_path(), file_path);
    BOOST_TEST_EQ(idx.file_size(), 0u);
    BOOST_TEST_EQ(idx.file_reserve(), 1000000u);
    BOOST_TEST_EQ(idx.index_path(), idx1_path);
    BOOST_TEST_EQ(idx.index_size(), 0u);
  }

  {
    cout << "      open via constructor..." << endl;
    btree::btree_index<int> idx(file_path, 1000000,
      idx1_path, btree::flags::truncate, -1, 128);
    BOOST_TEST(idx.is_open());
    BOOST_TEST_EQ(idx.file_path(), file_path);
    BOOST_TEST_EQ(idx.file_size(), 0u);
    BOOST_TEST_EQ(idx.file_reserve(), 1000000u);
    BOOST_TEST_EQ(idx.index_path(), idx1_path);
    BOOST_TEST_EQ(idx.index_size(), 0u);
  }

  cout << "    open_all_new_test complete" << endl;
}

//-------------------------------  simple_insert_test  ---------------------------------//
                                                                 
void  simple_insert_test()
{
  cout << "  simple_insert_test..." << endl;

  {
    btree::btree_index<stuff> idx(file_path, 1000000, idx1_path,
                                btree::flags::truncate, -1, 128);
    stuff x(2,2);
    btree::btree_index<stuff>::position_type pos = idx.push_back(x);
    BOOST_TEST_EQ(pos, 0u);
    BOOST_TEST_EQ(idx.file_size(), sizeof(stuff));
    idx.insert_position(pos);
    BOOST_TEST_EQ(idx.index_size(), 1u);

    x.assign(1,3);
    pos = idx.push_back(x);
    BOOST_TEST_EQ(pos, sizeof(stuff));
    BOOST_TEST_EQ(idx.file_size(), 2*sizeof(stuff));
    idx.insert_position(pos);
    BOOST_TEST_EQ(idx.index_size(), 2u);
  }
  BOOST_TEST_EQ(boost::filesystem::file_size(file_path), 2*sizeof(stuff));


  cout << "     simple_insert_test complete" << endl;
}

//-------------------------------  simple_iterator_test  -------------------------------//

void  simple_iterator_test()
{
  cout << "  simple_iterator_test..." << endl;

  typedef btree::btree_index<stuff> index_type;
  index_type idx(file_path, 0, idx1_path);

  index_type::iterator itr = idx.begin();
  index_type::iterator end = idx.end();

  BOOST_TEST(itr != end);
  stuff s = *itr;
  BOOST_TEST_EQ(s.x, 1);
  BOOST_TEST_EQ(s.y, 3);

  ++itr;
  BOOST_TEST(itr != end);
  s = *itr;
  BOOST_TEST_EQ(s.x, 2);
  BOOST_TEST_EQ(s.y, 2);

  ++itr;
  BOOST_TEST(itr == end);

  cout << "     simple_iterator_test complete" << endl;
}

//-------------------------------  open_new_index_test  --------------------------------//

void  open_new_index_test()
{
  cout << "  open_new_index_test with existing flat file..." << endl;

  cout << "    open_new_index_test with existing flat file complete" << endl;
}

//---------------------------------  two_index_test  -----------------------------------//

void  two_index_test()
{
  cout << "  two_index_test..." << endl;

  {
    btree::btree_index<stuff> idx1(file_path, 1000000,
      idx1_path, btree::flags::truncate, -1, 128);
    btree::btree_index<stuff, btree::default_traits, stuff_reverse_order>
      idx2(idx1.file(), idx2_path, btree::flags::truncate, -1, 128);

    stuff x(2,2);
    btree::btree_index<stuff>::position_type pos = idx1.push_back(x);
    idx1.insert_position(pos);
    idx2.insert_position(pos);

    x.assign(1,3);
    pos = idx1.push_back(x);
    idx1.insert_position(pos);
    idx2.insert_position(pos);

    x.assign(3,1);
    pos = idx1.push_back(x);
    idx1.insert_position(pos);
    idx2.insert_position(pos);
  }


  cout << "     two_index_test complete" << endl;
}

//-------------------------------------  _test  ----------------------------------------//

void  _test()
{
  cout << "  _test..." << endl;

  cout << "     _test complete" << endl;
}

}  // unnamed namespace

//------------------------------------ cpp_main ----------------------------------------//

int cpp_main(int argc, char* argv[])
{
  std::string command_args;

  for (int a = 0; a < argc; ++a)
  {
    command_args += argv[a];
    if (a != argc-1)
      command_args += ' ';
  }

  cout << command_args << '\n';;

  for (; argc > 1; ++argv, --argc) 
  {
    if ( std::strcmp( argv[1]+1, "b" )==0 )
      dump_dot = true;
    else
    {
      cout << "Error - unknown option: " << argv[1] << "\n\n";
      argc = -1;
      break;
    }
  }

  if (argc < 1) 
  {
    cout << "Usage: index_test [Options]\n"
//      " The argument n specifies the number of test cases to run\n"
      " Options:\n"
//      "   path     Specifies the test file path; default test.btree\n"
      "   -d       Dump tree using Graphviz dot format; default is no dump\n"
      ;
    return 1;
  }

  instantiate_test();
  open_all_new_test();
  simple_insert_test();
  simple_iterator_test();
  //open_new_index_test();
  //two_index_test();

  cout << "all tests complete" << endl;

  return boost::report_errors();
}


