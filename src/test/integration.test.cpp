#include "catch.hpp"
#include "../utility.h"
#include "../WorkUnit.h"
#include "../WorkFactory.h"
#include "../config/BuildConfig.h"

TEST_CASE( "Integration test - valid data", "[integration]" )
{
  auto csv = WorkFactory::createWorkUnit(
    WorkFactory::E_GoogleCsvFile,
    "/../src/test/data/data-valid.csv",
    "/../src/test/data/out.csv",
    "/../src/test/data/index-valid.csv");

  auto ret = csv->process();
  REQUIRE( ret == ExitCode::E_SUCCESS );

  unsigned rejectedIndexRows = csv->getRejectedIndexCount();
  unsigned rejectedDataRows = csv->getRejectedCount();
  unsigned processedDataRows = csv->getProcessedCount();

  CHECK( rejectedIndexRows == 0 );
  CHECK( rejectedDataRows == 0 );
  CHECK( processedDataRows == 3 );
}

TEST_CASE( "Integration test - invalid file paths", "[integration]" )
{
  REQUIRE_THROWS (
    WorkFactory::createWorkUnit(
      WorkFactory::E_GoogleCsvFile,
      "/badpath/",
      "/src//test/data/out.csv",
      ".csv")
  );
}

TEST_CASE( "Integration test - invalid data", "[integration]" )
{
  auto csv = WorkFactory::createWorkUnit(
    WorkFactory::E_GoogleCsvFile,
    "/../src/test/data/data-invalid.csv",
    "/../src/test/data/out.csv",
    "/../src/test/data/index-invalid.csv");

  auto ret = csv->process();
  REQUIRE( ret == ExitCode::E_SUCCESS );

  unsigned rejectedIndexRows = csv->getRejectedIndexCount();
  unsigned rejectedDataRows = csv->getRejectedCount();
  unsigned processedDataRows = csv->getProcessedCount();

  if constexpr (g_skipLocalitiesBelowStateOrProvince == true)
  {
    CHECK( rejectedIndexRows == 16 );
    CHECK( rejectedDataRows == 2 );
    CHECK( processedDataRows == 3 );
  }
  else
  {
    CHECK( rejectedIndexRows == 19 );
    CHECK( rejectedDataRows == 3 );
    CHECK( processedDataRows == 3 );
  }
}
