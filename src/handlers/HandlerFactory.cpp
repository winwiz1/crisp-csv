#include "../utility.h"
#include "CsvProcessorGoogle.h"
#include "CsvScannerGoogle.h"
#include "../config/BuildConfig.h"
#include "HandlerFactory.h"

using namespace std;

template <size_t InputProcessorFieldCount>
shared_ptr<void> HandlerFactory::createCsvProcessor(HandlerType handlerType, const string& indexFilePath)
{
  switch (handlerType)
  {
    case E_GoogleCsvProcessor:
    {
      string indexFile(utility::constructPath(indexFilePath));

      if constexpr (g_skipLocalitiesBelowStateOrProvince == true)
      {
        CsvProcessor<CsvFieldCounts::s_indexGoogle, CsvFieldCounts::s_processingGoogle>::InputFields indices{0,4,6,13};
        auto ptr = shared_ptr<GoogleCsvProcessor>(new CsvProcessorGoogle<>(indexFile, move(indices)));
        return ptr;
      }
      else
      {
        CsvProcessor<CsvFieldCounts::s_indexGoogle, CsvFieldCounts::s_processingGoogle>::InputFields indices{0,4,6,8,10,13};
        auto ptr = shared_ptr<GoogleCsvProcessor>(new CsvProcessorGoogle<>(indexFile, move(indices)));
        return ptr;
      }
    }

    default:
      if (handlerType < E_Sentinel)
      {
        throwNotImplemented();
      }
      else
      {
        throwInvalidRequest();
      }
  }
}

shared_ptr<CsvScanner> HandlerFactory::createCsvScanner(HandlerType handlerType)
{
  switch (handlerType)
  {
    case E_GoogleCsvScanner:
    {
      const unsigned maxIndex = CsvFieldCounts::s_inputGoogle;
      auto ptr = shared_ptr<CsvScanner>(new CsvScannerGoogle(maxIndex));
      return ptr;
    }

    default:
      if (handlerType < E_Sentinel)
      {
        throwNotImplemented();
      }
      else
      {
        throwInvalidRequest();
      }
  }

}

template
shared_ptr<void> HandlerFactory::createCsvProcessor<CsvFieldCounts::s_indexGoogle>(
  HandlerFactory::HandlerType,
  const string&);
