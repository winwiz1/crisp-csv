#include <cassert>
#include <thread>
#include <fstream>
#include "CsvFile.h"
#include "CsvRowReader.h"
#include "iterator.h"
#include "utility.h"
#include "main.h"

using namespace std;

template <
  size_t DataFieldCount, 
  size_t ProcessorInputFieldCount,
  size_t ProcessorOutputFieldCount>
CsvFile<DataFieldCount,ProcessorInputFieldCount,ProcessorOutputFieldCount>::CsvFile(
  const string& inFile,
  const string& outFile,
  DataFields&& indices,
  ProcessorPtr&& pProcessor,
  ScannerPtr&& pScanner
  ) :
  m_indices(move(indices)), m_pProcessor(move(pProcessor)), m_pScanner(move(pScanner)),
  m_countProcessed(0), m_countRejected(0), m_countRejectedIndex(0)
{
  bool bValid = static_cast<bool>(m_pProcessor) &&
    !inFile.empty() && !outFile.empty() && indices.size();

  if (!bValid)
  {
    assert(false);
    utility::throw_exception<invalid_argument>("invalid CsvFile argument(s)");
  }

  m_inStream.open(inFile.c_str(), ios::binary);
  m_outStream.open(outFile.c_str(), ios::binary | ios::trunc);

  string rejectFile = outFile;
  const char* insertion = "-reject";
  size_t ind = rejectFile.find_last_of(".'");
  rejectFile.insert(ind, insertion);
  m_rejectStream.open(rejectFile.c_str(), ios::binary | ios::trunc);

  locale loc("C.UTF-8");
  m_inStream.imbue(loc);
  m_outStream.imbue(loc);
  m_rejectStream.imbue(loc);

  if (!check_streams())
  {
    assert(false);
    utility::throw_exception<runtime_error>("failed to open data files");
  }
}

template <
  size_t DataFieldCount,
  size_t ProcessorInputFieldCount,
  size_t ProcessorOutputFieldCount>
CsvFile<DataFieldCount,ProcessorInputFieldCount,ProcessorOutputFieldCount>::~CsvFile()
{
  if (!check_streams())
  {
    assert(false);
    cerr << APP_TITLE" - I/O error";
  }
}

template <
  size_t DataFieldCount,
  size_t ProcessorInputFieldCount,
  size_t ProcessorOutputFieldCount>
bool CsvFile<DataFieldCount,ProcessorInputFieldCount,ProcessorOutputFieldCount>::check_streams() const
{
  bool ret = m_inStream.is_open() && !m_inStream.bad() &&
    m_outStream.is_open() && !m_outStream.bad() &&
    m_rejectStream.is_open() && !m_rejectStream.bad();

  return ret;
}

template <
  size_t DataFieldCount,
  size_t ProcessorInputFieldCount,
  size_t ProcessorOutputFieldCount>
ExitCode CsvFile<DataFieldCount,ProcessorInputFieldCount,ProcessorOutputFieldCount>::process()
{
  // Create a row reader
  array<unsigned, DataFieldCount> arr;

  for (unsigned i = 0; i < m_indices.size(); ++i)
  {
    arr[i] = get<0>(m_indices[i]);
  }

  CsvRowReader<DataFieldCount> rowReader(arr);

  // Loop through the CSV file
  auto incrementRowCount = [this]() { ++m_countProcessed; };

  while (g_SIGINT == 0 && m_inStream >> rowReader)
  {
    utility::ScopedAction sa(incrementRowCount);

    // Perform record scan
    using namespace std::placeholders;
    function<const wstring&(unsigned)> callback = bind(&CsvRowReader<DataFieldCount>::operator[], &rowReader, _1);
    auto scanResult = m_pScanner->scan(callback);

    if (scanResult != CsvScanner::E_ACCEPT)
    {
      if (scanResult == CsvScanner::E_REJECT)
      {
        const auto& row = rowReader.getReadonlyRow();
        copy(row.cbegin(), row.cend(), ostream_custom_iterator<wstring>(m_rejectStream, L","));
        m_rejectStream << L'\n';
      }
      
      continue;
    }

    // Loop through the CSV fields
    unsigned i = 0;
    bool exceptionCaught = false;
    wstringstream wsRow;

    for (const auto& fieldIndex : m_indices)
    {
      try 
      {
        const auto& fieldContent = rowReader[get<0>(fieldIndex)];
        // extend lifetime of rvalue returned by performFieldProcessing()
        const auto& processingResult = get<1>(fieldIndex)? performFieldProcessing(fieldContent): fieldContent;

        wsRow << processingResult;
      }
      catch (const exception& ex)
      {
        exceptionCaught = true;

        /*
        cerr << APP_TITLE" - Exception " << typeid(ex).name() << ": " << ex.what() << 
          " Row: " << m_countProcessed << ", field: " << get<0>(fieldIndex) << '\n';
        */

        // If the exception was caused by the above stream operation then terminate.
        // Otherwise stop current row processing.
        if (!check_streams())
        {
          utility::throw_exception<runtime_error>("I/O error during data processing");
        }
        ++m_countRejected;
        string reason(ex.what());
        m_rejectStream << L"Processing failure for row " << m_countProcessed << L": " << wstring(reason.begin(), reason.end()) << L'\n';
        break;
      }

      if (++i < m_indices.size())
      {
        wsRow << L',';
      }
    }

    if (!exceptionCaught)
    {
      // do not use: << endl;
      m_outStream << wsRow.str() << L'\n';
    }

    if (m_countProcessed % s_yieldFrequency == 0)
    {
      this_thread::yield();
    }
  }

  if (!check_streams())
  {
    utility::throw_exception<runtime_error>("I/O error during data processing");
  }

  ExitCode ret = ExitCode::E_SUCCESS;

  if (g_SIGINT)
  {
    cerr << APP_TITLE" - terminating on signal, leaving incomplete output files" << endl;
    ret = ExitCode::E_SIGINT;
  }
  else
  {
    cout << APP_TITLE" - processed " << m_countProcessed << " data rows" << endl;

    if (m_countRejected)
    {
      cout << APP_TITLE" - rejected " << m_countRejected << " data rows due to index processing failure" << endl;
    }
  }
  
  m_countRejected += m_pScanner->getRejectedCount();
  m_countRejectedIndex = m_pProcessor->getRejectedCount();
  
  return ret;
}

template <
  size_t DataFieldCount,
  size_t ProcessorInputFieldCount,
  size_t ProcessorOutputFieldCount>
wstring CsvFile<DataFieldCount,ProcessorInputFieldCount,ProcessorOutputFieldCount>::performFieldProcessing(const wstring& strIn) const
{
  if (strIn.empty())
  {
    // The field(s) that are subject to processing by CsvProcessor are important and cannot be missing
    utility::throw_exception<invalid_argument>("no field to process");
  }

  // extend lifetime of rvalue returned by processCsvField()
  const auto& processingResult = m_pProcessor->processCsvField(strIn);    // throws if processCsvField fails
  assert(!processingResult[0].empty());

  wostringstream strStream;

  copy(processingResult.cbegin(), processingResult.cend(),
    ostream_custom_iterator<wstring>(strStream, L","));

  return strStream.str();
}

template class CsvFile<
  CsvFieldCounts::s_inputGoogle,
  CsvFieldCounts::s_indexGoogle,
  CsvFieldCounts::s_processingGoogle>;
