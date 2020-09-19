#include <cassert>
#include <iomanip>
#include "config/BuildConfig.h"
#include "CsvRowReader.h"

using namespace std;

template <size_t FieldCount>
CsvRowReader<FieldCount>::CsvRowReader(const array<unsigned, FieldCount>& indices)
{
  if (indices.size() == 0)
  {
    assert(false);
    utility::throw_exception<invalid_argument>("no row indices requested");
  }

  unsigned ordinal = 0;
  for (const auto& ind : indices)
  {
    m_map.emplace(ind, ordinal++);
  }
}

template <size_t FieldCount>
void CsvRowReader<FieldCount>::clear()
{
  for (auto& s : m_data)
  {
    s.clear();
  }
}

template <size_t FieldCount>
unsigned CsvRowReader<FieldCount>::get_largest_index() const
{
  unsigned ret = m_map.crbegin()->first;
  return ret;
}

template <size_t FieldCount>
void CsvRowReader<FieldCount>::readNextRow(wistream& inStream)
{
  wstring line;
  getline(inStream, line);
  wstringstream lineStream(move(line));

  wstring cell;
  unsigned currentField = 0;
  unsigned largestIndex = get_largest_index();
  clear();

  while (lineStream >> ws)
  {
    if (currentField > largestIndex)
      return;

    if (lineStream.peek() == L'"')
    {
      lineStream >> quoted(cell);
      wstring strRemove;
      // remove the characters between closing double quote and comma
      getline(lineStream, strRemove, L',');
    }
    else
    {
      getline(lineStream, cell, L',');
    }

    typename decltype(m_map)::const_iterator it = m_map.find(currentField);

    if (it != m_map.end())
    {
      store_data(it->second, move(cell));
    }

    ++currentField;
  }
}

template <size_t FieldCount>
void CsvRowReader<FieldCount>::store_data(unsigned ind, wstring&& data)
{
  if (ind >= FieldCount)
  {
    utility::throw_exception<out_of_range>("too many fields to extract");
  }

  m_data[ind] = data;
}

template class CsvRowReader<CsvFieldCounts::s_inputGoogle>;
template class CsvRowReader<CsvFieldCounts::s_indexGoogle>;
