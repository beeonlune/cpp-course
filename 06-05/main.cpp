#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <boost/config.hpp>
#include <boost/dll.hpp>

#if defined(BUILD_LIB_V1)

extern "C" BOOST_SYMBOL_EXPORT void test()
{
  std::cout << "v1\n";
}

#elif defined(BUILD_LIB_V2)

extern "C" BOOST_SYMBOL_EXPORT void test()
{
  std::cout << "v2\n";
}

#else

#include "06.18.hpp"

int main()
{
  library::test();

  std::vector < std::function < void() > > functions;

  auto path = "libshared.so";

  functions.push_back(boost::dll::import_symbol < void() > (path, "test_v1"));

  functions.push_back(boost::dll::import_symbol < void() > (path, "test_v2"));

  for (auto const & function : functions)
  {
    function();
  }

  boost::dll::import_alias < void() > (path, "test_v3")();

  std::string library_path;

  if (std::cin >> library_path)
  {
    auto imported_test = boost::dll::import_symbol < void() > (library_path, "test");

    imported_test();
  }
}

#endif

