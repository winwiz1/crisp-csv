/*
  The factory creates units of work such as instances of CsvFile.
  The classes using the factory get a smart pointer to abstract unit of work
  and do not know which implementation class does the pointer points to.
*/
#pragma once

#include <memory>

struct IWorkUnit;

class WorkFactory
{
public:
  typedef enum {
    E_GoogleCsvFile,
    // ...
    E_Sentinel
  } WorkUnitType;

  static std::shared_ptr<IWorkUnit> createWorkUnit(
    WorkUnitType workUnitType,
    const std::string& inFile,
    const std::string& outFile,
    const std::string& indexFile = "/csv/index.csv");
  
protected:

  [[ noreturn ]]
  static void throwNotImplemented()
  {
    utility::throw_exception<std::logic_error>("not implemented by WorkFactory");
  }

  [[ noreturn ]]
  static void throwInvalidRequest()
  {
    utility::throw_exception<std::out_of_range>("invalid WorkFactory request");
  }

public:
  WorkFactory() = delete;
  WorkFactory(WorkFactory const&) = delete;
  WorkFactory(WorkFactory&&) = delete;
};
