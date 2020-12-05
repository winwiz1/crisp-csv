/*
  CsvFile acts as the CSV processing engine, however the class is data-agnostic
  e.g. it has no knowledge of what data is contained in which CSV field.
  Therefore this class needs to delegate the data processing work to handlers
  which are the classes derived from CsvScanner and CsvProcessor.
*/
#pragma once

#include <memory>
#include "WorkUnit.h"
#include "utility.h"
#include "config/BuildConfig.h"
#include "handlers/CsvProcessor.h"
#include "handlers/CsvScanner.h"

template <
  std::size_t DataFieldCount,
  std::size_t ProcessorInputFieldCount,
  std::size_t ProcessorOutputFieldCount>
class CsvFile : public IWorkUnit
{
public:

  // The DataFields array contains a tuple for each CSV field to be extracted
  // from input CSV file. The tuple holds the ordinal/index of the field
  // along with a boolean that indicates (when true) the content of the field
  // needs to be processed by CsvProcessor.
  typedef std::array<std::tuple<unsigned,bool>, DataFieldCount> DataFields;
  // Smart pointer to abstract class that performs CSV field processing
  typedef std::shared_ptr<CsvProcessor<ProcessorInputFieldCount,ProcessorOutputFieldCount>> ProcessorPtr;
  // Smart pointer to helper abstract class that performs CSV record scanning
  typedef std::shared_ptr<CsvScanner> ScannerPtr;

  CsvFile(const std::string& inFile, 
          const std::string& outFile,
          DataFields&& indices,
          ProcessorPtr&& pProcessor,
          ScannerPtr&& pScanner);
  ~CsvFile();

  ExitCode process() override;
  unsigned getProcessedCount() override { return m_countProcessed; }
  unsigned getRejectedCount() override { return m_countRejected; }
  unsigned getRejectedIndexCount() override { return m_countRejectedIndex; }

protected:
  bool check_streams() const;
  std::wstring performFieldProcessing(const std::wstring&) const noexcept(false);

  DataFields m_indices;
  std::wifstream m_inStream;
  std::wofstream m_outStream;
  std::wofstream m_rejectStream;
  ProcessorPtr m_pProcessor;
  ScannerPtr m_pScanner;
  unsigned m_countProcessed;
  unsigned m_countRejected;
  unsigned m_countRejectedIndex;

  static const int s_yieldFrequency = 1000;
};
