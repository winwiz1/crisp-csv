/*
  The Factory shields its callers from knowing the implementation and
  creation details of the handlers it creates. The existence of classes
  implementing handler interfaces is also hidden from external classes
  that use the Factory.
*/
#pragma once

#include <memory>

class CsvScanner;

class HandlerFactory
{
public:
  typedef enum {
    E_GoogleCsvProcessor,
    E_GoogleCsvScanner,
    // ...
    E_Sentinel
  } HandlerType;

  typedef class CsvProcessor<
    CsvFieldCounts::s_indexGoogle,
    CsvFieldCounts::s_processingGoogle
  > GoogleCsvProcessor;

  // Use template to make GCC discard the unused branch of 'if constexpr',
  // otherwise syntax error is triggered as per C++ standard
  template <std::size_t InputProcessorFieldCount>
  static std::shared_ptr<void> createCsvProcessor(HandlerType type, const std::string& indexFile);

  static std::shared_ptr<CsvScanner> createCsvScanner(HandlerType handlerType);

protected:

  [[ noreturn ]]
  static void throwNotImplemented()
  {
    utility::throw_exception<std::logic_error>("not implemented by HandlerFactory");
  }

  [[ noreturn ]]
  static void throwInvalidRequest()
  {
    utility::throw_exception<std::out_of_range>("invalid HandlerFactory request");
  }

public:
  HandlerFactory() = delete;
  HandlerFactory(HandlerFactory const&) = delete;
  HandlerFactory(HandlerFactory&&) = delete;
};
