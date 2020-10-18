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
  wstring csvFields[] = {
    L"123",                  // non-quoted field
    L"abc (xyz)",            // non-quoted field
    L"",                     // empty field
    L"\"abc xyz\"",          // quoted field without comma
    L"\"abc, xyz\"",         // quoted field with comma
    L"\"abc \"\" xyz\"",     // quoted field with escaped quote
    L"  leading space",
    L"trailing space   "
  };

  wstringstream stream;

  for (const auto& field: csvFields)
  {
    stream << field << L',';
  }

  stream << L"end";

  auto fun = [&stream] {
    constexpr unsigned DataFieldCount = CsvFieldCounts::s_inputGoogle;
    array<unsigned, DataFieldCount> arr{1,3,4,5,6};
    CsvRowReader<DataFieldCount> reader(arr);

    stream >> reader;
    return make_tuple(reader[1],reader[3],reader[4],reader[5],reader[6]);
  };

  const auto& [f1,f2,f3,f4,_] = fun();

  CHECK( f1 == csvFields[1] );
  CHECK( f2 == csvFields[3]);
  CHECK( f3 == csvFields[4] );
  CHECK( f4 == csvFields[5] );
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
