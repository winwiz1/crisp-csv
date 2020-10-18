#include <cassert>
#include "config/BuildConfig.h"
#include "CsvRowReader.h"

using namespace std;

template <size_t FieldCount>
const string CsvRowReader<FieldCount>::csv_error::s_strWhat{"Incorrectly quoted CSV field"};

template <size_t FieldCount>
const wregex CsvRowReader<FieldCount>::s_regex{L"^\"[^\"]*?(?:\"\")?[^\"]*?(?:\"\")?[^\"]*?\"(?:,|$)"};

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

  locale loc("C.UTF-8");
  m_lineStream.imbue(loc);
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
  wstring wstr;
  std::getline(inStream, wstr);

  m_lineStream.str(move(wstr));
  m_lineStream.clear();
  m_lineStream.seekg(0);

  wstring currentField;
  unsigned currentFieldIndex = 0;
  unsigned largestIndex = get_largest_index();
  clear();

  while (m_lineStream >> ws)
  {
    if (currentFieldIndex > largestIndex)
      return;

    bool bQuoted = m_lineStream.peek() == L'"';

    if (bQuoted)
    {
      const size_t quoteStart = m_lineStream.tellg();
      const wstring lineRemainder = m_lineStream.str().substr(quoteStart);

      wsmatch wsm;
      bool bMatch = regex_search(lineRemainder, wsm, s_regex);

      if (!bMatch)
      {
        throw csv_error(m_lineStream.str());
      }

      currentField = wsm.str();
      auto len = currentField.length();

      if (currentField.back() == L',')
      {
        currentField.pop_back();
      }

      m_lineStream.seekg(quoteStart + len);
    }
    else
    {
      std::getline(m_lineStream, currentField, L',');
    }

    typename decltype(m_map)::const_iterator it = m_map.find(currentFieldIndex);

    if (it != m_map.end())
    {
      store_data(it->second, move(currentField));
    }

    ++currentFieldIndex;
  }
}

template <size_t FieldCount>
void CsvRowReader<FieldCount>::store_data(unsigned ind, wstring&& data)
{
  if (ind >= FieldCount)
  {
    utility::throw_exception<out_of_range>("too many fields to extract");
  }

  m_data[ind] = move(data);
}

template class CsvRowReader<CsvFieldCounts::s_inputGoogle>;
template class CsvRowReader<CsvFieldCounts::s_indexGoogle>;
