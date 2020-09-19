#include <array>
#include <tuple>
#include <sstream>
#include "catch.hpp"
#include "../CsvRowReader.h"
#include "../config/BuildConfig.h"
#include "../config/RuntimeConfig.h"

using namespace std;

TEST_CASE( "Test row reader", "[unit]" )
{
  auto fun = [] {
    constexpr unsigned DataFieldCount = CsvFieldCounts::s_inputGoogle;
    array<unsigned, DataFieldCount> arr{1,3,4,5,6};
    CsvRowReader<DataFieldCount> reader(arr);

    wistringstream is(L"a,b,c,d,e,\"f f\",g,h");
    is >> reader;
    return make_tuple(reader[1],reader[3],reader[4],reader[5],reader[6]);
  };

  const auto& [f1,f2,f3,f4,_] = fun();

  CHECK( f1 == L"b" );
  CHECK( f2 == L"d" );
  CHECK( f3 == L"e" );
  CHECK( f4 == L"f f" );
}

TEST_CASE( "Test run-time configuration", "[unit]" )
{
  const auto& cfg = RuntimeConfig::GetInstance();

  CHECK( cfg.getProcessedDataRowsThreshold() == 1 );
  CHECK( cfg.getRejectedIndexRowsThreshold() == 0 );
  CHECK( cfg.getRejectedDataRowsThreshold() == 0 );
  CHECK( cfg.getFilterUkNuts() );
  CHECK( cfg.getFilterAuData() );
}
