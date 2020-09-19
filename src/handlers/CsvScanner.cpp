#include <iostream>
#include "../utility.h"
#include "CsvScanner.h"

using namespace std;

CsvScanner::CsvScanner(unsigned maxIndex) : m_maxIndex(maxIndex)
{
}

CsvScanner::~CsvScanner()
{
  if (m_countRejected)
  {
    cout << APP_TITLE" - rejected " << m_countRejected << " data row" << (m_countRejected > 1? "s": "") << endl;
  }
  if (m_countFiltered)
  {
    cout << APP_TITLE" - filtered " << m_countFiltered << " data row" << (m_countFiltered > 1? "s": "") << endl;
  }
}

CsvScanner::E_RESULT CsvScanner::scan(Callback& callback)
{
  auto ret = scan_internal(callback);
  return ret;
}