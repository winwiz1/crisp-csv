#include <fstream>
#include "CsvFile.h"
#include "handlers/HandlerFactory.h"
#include "WorkFactory.h"

using namespace std;

namespace
{
  typedef CsvFile<
    CsvFieldCounts::s_inputGoogle,
    CsvFieldCounts::s_indexGoogle,
    CsvFieldCounts::s_processingGoogle
  > GoogleCsvFile;
}

shared_ptr<IWorkUnit> WorkFactory::createWorkUnit(
  WorkUnitType workUnitType,
  const string& inFile,
  const string& outFile,
  const string& indexFile)
{
  if (inFile.empty() || outFile.empty() || indexFile.empty())
  {
    utility::throw_exception<invalid_argument>("WorkFactory: invalid file argument(s)");
  }

  switch (workUnitType)
  {
    case E_GoogleCsvFile:
    {
      GoogleCsvFile::DataFields fields{
        make_tuple(0,false),
        make_tuple(1,true),
        make_tuple(6,false),
        make_tuple(7,false),
        make_tuple(8,false)};
      auto pHandler = HandlerFactory::createCsvProcessor<CsvFieldCounts::s_indexGoogle>(
        HandlerFactory::E_GoogleCsvProcessor,
        indexFile);
      auto pProcessor = static_pointer_cast<HandlerFactory::GoogleCsvProcessor>(pHandler);
      auto pScanner = HandlerFactory::createCsvScanner(HandlerFactory::E_GoogleCsvScanner);
      string dataFile(utility::constructPath(inFile));
      string outputFile(utility::constructPath(outFile));
      auto ptr = std::shared_ptr<IWorkUnit>(new GoogleCsvFile(dataFile, outputFile, move(fields), move(pProcessor), move(pScanner)));
      return ptr;
    }

    default:
      if (workUnitType < E_Sentinel)
      {
        throwNotImplemented();
      }
      else
      {
        throwInvalidRequest();
      }
  }

}
