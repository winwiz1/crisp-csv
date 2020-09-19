/*
  Handler used by CsvFile to process a CSV field.
*/
#pragma once

#include <array>

/*
  Abstract class, defines non-virtual interface.
*/
template <
  // How many fields need to be extracted to build lookup dictionary
  std::size_t InputFieldCount,
  // How many fields can be included into lookup result
  std::size_t OutputFieldCount>
class CsvProcessor : public utility::Counter
{
public:
  typedef std::array<unsigned, InputFieldCount> InputFields;
  typedef std::array<std::wstring, OutputFieldCount> OutputFields;

  CsvProcessor(InputFields&& indices);
  ~CsvProcessor();

  // Public non-virtual interface
  OutputFields processCsvField(const std::wstring& field) const noexcept(false);

protected:
  InputFields m_indices;

private:
  // Private virtual interface meant to hide the existence of derived classes
  // (implementing this interface) from classes that use CsvProcessor
  virtual OutputFields process_internal(const std::wstring& field) const noexcept(false) = 0;
};
