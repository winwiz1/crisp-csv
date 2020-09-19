/*
  CsvRowReader extracts the selected subset of CSV fields from CSV row.
*/
#pragma once

#include <map>
#include <sstream>
#include <array>
#include "utility.h"

// FieldCount is the count of the requested fields
template <std::size_t FieldCount>
class CsvRowReader
{
public:
  // Ctor takes an array of the requested CSV field indices
  CsvRowReader(const std::array<unsigned, FieldCount>& indices);
  CsvRowReader(const CsvRowReader&) = delete;

  void readNextRow(std::wistream&);

  inline const std::wstring& operator[] (std::size_t index) const;
  auto getReadonlyRow() const -> const std::array<std::wstring, FieldCount>& { return m_data; }

private:
  void clear();
  void store_data(unsigned ind, std::wstring&& data);
  unsigned get_largest_index() const;

  std::map<unsigned, unsigned> m_map;
  std::array<std::wstring, FieldCount> m_data;
};

template <std::size_t FieldCount>
const std::wstring& CsvRowReader<FieldCount>::operator[] (std::size_t index) const
{
  typename decltype(m_map)::const_iterator it = m_map.find(index);

  if (it == m_map.end())
  {
    utility::throw_exception<std::invalid_argument>("the requested row index is invalid");
  }

  unsigned ordinal = std::distance(m_map.cbegin(), it);

  if (ordinal >= FieldCount)
  {
    utility::throw_exception<std::out_of_range>("the requested row index is out of range");
  }

  return m_data[ordinal];
}

template <std::size_t FieldCount>
std::wistream& operator>> (std::wistream& inStream, CsvRowReader<FieldCount>& rowReader)
{
  rowReader.readNextRow(inStream);
  return inStream;
}
