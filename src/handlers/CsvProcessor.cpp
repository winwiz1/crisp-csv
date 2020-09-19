#include <cassert>
#include <iostream>
#include "../utility.h"
#include "../config/BuildConfig.h"
#include "CsvProcessor.h"

using namespace std;

template <
  size_t InputFieldCount,
  size_t OutputFieldCount>
CsvProcessor<InputFieldCount, OutputFieldCount>::CsvProcessor(InputFields&& indices) :
  m_indices(move(indices))
{
  if (bool bValid = indices.size() > 0; !bValid)
  {
    assert(false);
    utility::throw_exception<invalid_argument>("invalid argument: no fields to process");
  }
}

template <
  size_t InputFieldCount,
  size_t OutputFieldCount>
CsvProcessor<InputFieldCount, OutputFieldCount>::~CsvProcessor()
{
  if (m_countRejected)
  {
    cout << APP_TITLE" - rejected " << m_countRejected << " index row" << (m_countRejected > 1? "s": "") << endl;
  }
  if (m_countFiltered)
  {
    cout << APP_TITLE" - filtered " << m_countFiltered << " index row" << (m_countFiltered > 1? "s": "") <<endl;
  }
}

template <
  size_t InputFieldCount,
  size_t OutputFieldCount>
auto CsvProcessor<InputFieldCount, OutputFieldCount>::processCsvField(const wstring& wField) const -> OutputFields
{
  if (wField.empty())
  {
    // The field that needs to be processed is important and cannot be missing
    utility::throw_exception<invalid_argument>("missing field to process");
  }
  
  auto ret = process_internal(wField);
  return ret;
}

template class CsvProcessor<
  CsvFieldCounts::s_indexGoogle,
  CsvFieldCounts::s_processingGoogle>;
