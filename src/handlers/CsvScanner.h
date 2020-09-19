/*
  Handler used by CsvFile to scan a CSV record and decide if the record (being
  a collection of the extracted CSV fields) needs to be processed further or
  rejected or filtered out.
*/
#pragma once

#include <functional>

/*
  Defines non-virtual interface used for data validation
*/
class CsvScanner : public utility::Counter
{
public:
  typedef std::function<const std::wstring&(unsigned)> Callback;
  typedef enum { E_ACCEPT, E_FILTER, E_REJECT } E_RESULT;

  CsvScanner(unsigned maxIndex);
  ~CsvScanner();

  // Public non-virtual interface
  E_RESULT scan(Callback& callback);

protected:
  const unsigned m_maxIndex;

private:
  // Private virtual interface meant to hide the existence of derived classes
  // (implementing this interface) from external callers that use CsvScanner.
  virtual E_RESULT scan_internal(Callback& callback) = 0;
  // It's ok for a virtual method to be private since it doesn't need to be
  // called by the derived class method. And there is no need to call it
  // because we chose not to provide a body so there is no code to reuse
  // (abstract method can have a body).
};
