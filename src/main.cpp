/*
  main() is conditionally compiled depending on the preprocessor
   macro that in turn is defined by the build configuration.
*/
#include <time.h>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include "utility.h"
#include "WorkUnit.h"
#include "WorkFactory.h"
#include "config/BuildConfig.h"
#include "config/RuntimeConfig.h"
#include "main.h"

using namespace std;

volatile sig_atomic_t g_SIGINT = 0;

#if !defined(CSV_TEST)

namespace
{
  clock_t s_timeStart;

  void sig_handler(int)
  {
    g_SIGINT = 1;
  }

  void print_time()
  {
    if (g_SIGINT == 0)
    {
      clock_t timeFinish = clock();
      double timeTaken = (double)(timeFinish - s_timeStart) / CLOCKS_PER_SEC;
      printf(APP_TITLE" - execution time: %d seconds\n", static_cast<int>(round(timeTaken)));
    }
  }

}  //namespace


int main()
{
  s_timeStart = clock();
  atexit(print_time);
  signal(SIGINT, sig_handler);

  ExitCode ret;
  ios::sync_with_stdio(false);
  cout << APP_TITLE" - version " << STRINGIFY(CSV_VERSION) << endl;

  try
  {
    auto csv = WorkFactory::createWorkUnit(WorkFactory::E_GoogleCsvFile, "/csv/epidemiology.csv", "/csv/out.csv");
    ret = csv->process();

    if (ret == ExitCode::E_SUCCESS)
    {
      unsigned rejectedIndexRows = csv->getRejectedIndexCount();
      unsigned rejectedDataRows = csv->getRejectedCount();
      unsigned processedDataRows = csv->getProcessedCount();
      const auto& cfg = RuntimeConfig::GetInstance();

      if (processedDataRows < cfg.getProcessedDataRowsThreshold() ||
          rejectedIndexRows > cfg.getRejectedIndexRowsThreshold() ||
          rejectedDataRows > cfg.getRejectedDataRowsThreshold())
      {
        ret = ExitCode::E_ERROR;
        cerr << APP_TITLE" - data verification failure due to counts. Processed row count: " << processedDataRows <<
          ", rejected data row count: " << rejectedDataRows << ", rejected index row count: " << rejectedIndexRows << endl;
      }
    }
  }
  catch (const exception& ex)
  {
    cerr << APP_TITLE" - Exception " << typeid(ex).name() << ": " << ex.what() << endl;
    ret = ExitCode::E_EXCEPTION;
  }

  return static_cast<int>(ret);
}

#endif
