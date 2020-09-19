/*
  Implementation of the CsvScanner interface for
  Google COVID-19 Open Data repository.
*/
#pragma once

#include <regex>
#include "CsvScanner.h"

class CsvScannerGoogle : public CsvScanner
{
public:

  CsvScannerGoogle(unsigned maxIndex);
  ~CsvScannerGoogle() = default;
  
private:
  CsvScanner::E_RESULT scan_internal(CsvScanner::Callback& callback) override;

  // Regex to check dates
  static const std::wregex m_regexDate;
  // Regex to find UK NUTS regions
  static const std::wregex m_regexUkNuts;
};
