#include <chrono>
#include "../utility.h"
#include "../config/BuildConfig.h"
#include "../config/RuntimeConfig.h"
#include "CsvScannerGoogle.h"

using namespace std;

const wregex CsvScannerGoogle::s_regexDate{L"^20[1-2][0-9]-\\d{2}-\\d{2}$"};
const wregex CsvScannerGoogle::s_regexUkNuts{L"^GB_UK[A-Z]$"};

CsvScannerGoogle::CsvScannerGoogle(unsigned maxIndex) : CsvScanner(maxIndex)
{
}

CsvScanner::E_RESULT CsvScannerGoogle::scan_internal(CsvScanner::Callback& callback)
{
  CsvScanner::E_RESULT ret = E_ACCEPT;

  // Reject rows with invalid index
  // Depending on a build, filter rows with index below state/province level
  wsmatch mr;
  bool bMatch = regex_match(callback(1), mr, utility::g_regexIndex);

  if (!bMatch)
  {
    ++m_countRejected;
    ret = E_REJECT;
  }
  else if (g_skipLocalitiesBelowStateOrProvince)
  {
    if (mr[2].matched)
    {
      ++m_countFiltered;
      ret = E_FILTER;
    }
  }

  if (ret != E_ACCEPT)
  {
    return ret;
  }

  bool bConfirmed = callback(6).empty();
  bool bRecovered = callback(7).empty();
  bool bDeaths = callback(8).empty();
  static bool filterUkNuts = RuntimeConfig::GetInstance().getFilterUkNuts();
  static bool filterAuData = RuntimeConfig::GetInstance().getFilterAuData();

  // Filter out the rows without all the three cumulative epidemiology metrics
  if (bConfirmed && bRecovered && bDeaths)
  {
    ++m_countFiltered;
    ret = E_FILTER;
  }
  else if (filterAuData && bRecovered && bDeaths &&
    callback(1).rfind(L"AU", 0) == 0 && callback(0) == utility::getGmtDate())
  {
    // Additionally filter out the rows for Australia today's data if
    // the last two metrics are missing regardless of the first metric
    // (cumulative confirmed case count) being present or not.
    ++m_countFiltered;
    ret = E_FILTER;
  }
  else if (filterUkNuts && regex_match(callback(1).c_str(), CsvScannerGoogle::s_regexUkNuts))
  {
    // Filter out UK NUTS regions otherwise 'calculated total' figures
    // get distorted
    ++m_countFiltered;
    ret = E_FILTER;
  }
  else if (!regex_match(callback(0).c_str(), CsvScannerGoogle::s_regexDate))
  {
    // Reject rows with invalid date
    ++m_countRejected;
    ret = E_REJECT;
  }

  return ret;
}
