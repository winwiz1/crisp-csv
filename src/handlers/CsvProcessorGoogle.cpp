#include <cassert>
#include <thread>
#include "../main.h"
#include "../CsvRowReader.h"
#include "../iterator.h"
#include "../utility.h"
#include "../config/RuntimeConfig.h"
#include "CsvProcessorGoogle.h"

using namespace std;

template <
  size_t InputFieldCount,
  size_t OutputFieldCount>
CsvProcessorGoogle<InputFieldCount, OutputFieldCount>::CsvProcessorGoogle(const std::string& inFile, typename Base::InputFields&& indices) : 
  Base(move(indices))
{
  if (inFile.empty())
  {
    assert(false);
    utility::throw_exception<invalid_argument>("invalid index file");
  }

  m_inStream.open(inFile.c_str(), ios::binary);
  
  string rejectFile = inFile;
  const char* insertion = "-reject";
  size_t ind = rejectFile.find_last_of(".'");
  rejectFile.insert(ind, insertion);
  m_rejectStream.open(rejectFile.c_str(), ios::binary | ios::trunc);

  locale loc("C.UTF-8");
  m_inStream.imbue(loc);
  m_rejectStream.imbue(loc);

  if (!check_streams())
  {
    assert(false);
    utility::throw_exception<runtime_error>("failed to open index files");
  }

  if (!build_dictionary())
  {
    utility::throw_exception<runtime_error>("failed to build lookup dictionary");
  }
}

template <
  size_t InputFieldCount,
  size_t OutputFieldCount>
bool CsvProcessorGoogle<InputFieldCount, OutputFieldCount>::check_streams() const
{
  bool ret = m_inStream.is_open() && !m_inStream.bad() &&
    m_rejectStream.is_open() && !m_rejectStream.bad();

  return ret;
}

template <
  size_t InputFieldCount,
  size_t OutputFieldCount>
bool CsvProcessorGoogle<InputFieldCount, OutputFieldCount>::build_dictionary()
{
  int rowCount = 0;
  CsvRowReader<InputFieldCount> rowReader(m_indices);

  auto incrementRowCount = [&rowCount]() { ++rowCount; };
  auto saveRejectedRow = [this, &rowReader]() {
      const auto& row = rowReader.getReadonlyRow();
      copy(row.cbegin(), row.cend(), ostream_custom_iterator<wstring>(m_rejectStream, L","));
      m_rejectStream << L'\n';
  };

  while (g_SIGINT == 0 && m_inStream >> rowReader)
  {
    utility::ScopedAction sa(incrementRowCount);

    const auto& aggLevel = rowReader[m_indices.back()];
    unsigned long level = 0L;
    try
    {
      level = std::stoul(aggLevel);
    }
    catch (const exception&)
    {
      ++m_countRejected;
      saveRejectedRow();
      continue;
    }

    const auto& index = rowReader[m_indices.front()];
    wsmatch mr;
    if (bool bMatch = regex_match(index, mr, utility::g_regexIndex); !bMatch)
    {
      ++m_countRejected;
      saveRejectedRow();
      continue;
    }

    static bool relaxIndexChecks = RuntimeConfig::GetInstance().getRelaxIndexChecks();

    if (level > 3L ||
        (level == 0L && (mr[1].matched || mr[2].matched)) ||
        (level == 1L && (!mr[1].matched || mr[2].matched)) ||
        (level >= 2L && (!mr[1].matched || (!mr[2].matched && !relaxIndexChecks))))
    {
      ++m_countRejected;
      saveRejectedRow();
      continue;
    }

    const auto& countryName = rowReader[m_indices[1]];
    const auto& stateName = rowReader[m_indices[2]];

    if (!relaxIndexChecks && mr[1].matched == stateName.empty())
    {
      // Reject rows with state/province data and index asserting absense of this data
      // Reject rows with missing state/province data and index asserting presense of this data
      ++m_countRejected;
      saveRejectedRow();
      continue;     
    }

    if (countryName.length() < s_minCountryNameLen ||
        (!stateName.empty() && stateName.length() < s_minStateNameLen))
    {
      ++m_countRejected;
      saveRejectedRow();
      continue;
    }

    if constexpr (g_skipLocalitiesBelowStateOrProvince == true)
    {
      if (mr[2].matched)
      {
        ++m_countFiltered;
      }
      else
      {
        m_map.emplace(make_pair(index, make_tuple(countryName, stateName)));
      }

      continue;
    }
    else
    { // Check locality data: both subregion2_name and locality_name
      const auto& subregion2_name = rowReader[m_indices[3]];
      const auto& locality_name = rowReader[m_indices[4]];
      const auto& localityName = level == 2L? subregion2_name : locality_name;

      if ((!relaxIndexChecks && mr[2].matched == localityName.empty()) ||
          (level == 2L && !locality_name.empty()) ||
          (level == 3L && !relaxIndexChecks && !subregion2_name.empty()))
      {
        // Reject rows with locality data and index asserting absense of this data
        // Reject rows with missing locality data and index asserting presense of this data
        // Reject rows with inconsistent locality data
        ++m_countRejected;
        saveRejectedRow();
        continue;
      }

      if (!localityName.empty() && localityName.length() < s_minLocalityNameLen)
      {
        ++m_countRejected;
        saveRejectedRow();
        continue;
      }

      m_map.emplace(make_pair(index, make_tuple(countryName, stateName, localityName, level)));
    }

    if (rowCount % s_yieldFrequency == 0)
    {
      this_thread::yield();
    }
  }

  if (!check_streams())
  {
    assert(false);
    utility::throw_exception<runtime_error>("I/O error during index processing");
  }

  return g_SIGINT? false: true;
}

template <
  size_t InputFieldCount,
  size_t OutputFieldCount>
auto CsvProcessorGoogle<InputFieldCount, OutputFieldCount>::process_internal(const wstring& wField) const -> typename Base::OutputFields
{
  assert(!wField.empty());

  const auto it = m_map.find(wField);

  if (it == m_map.end())
  {
    string msg("lookup failed: index \'");
    const string field(wField.begin(), wField.end());
    const string notFound("\' not found");
    msg += field;
    msg += notFound;
    utility::throw_exception<invalid_argument>(msg.c_str());
  }

  if constexpr (g_skipLocalitiesBelowStateOrProvince == true)
  {
    const tuple<wstring,wstring>& tpl = it -> second;
    const auto& [country, state] = tpl;

    typename Base::OutputFields ret{wField, country, state, (state.empty()? L"0" : L"1")};
    return ret;
  }
  else
  {
    const tuple<wstring,wstring,wstring,unsigned long>& tpl = it -> second;
    const auto& [country, state, locality, level] = tpl;
    const auto aggLevel = to_wstring(level);
    typename Base::OutputFields ret{wField, country, state, quoteLocality(locality), aggLevel};
    return ret;
  }
}

template <
  size_t InputFieldCount,
  size_t OutputFieldCount>
std::wstring CsvProcessorGoogle<InputFieldCount, OutputFieldCount>::quoteLocality(const std::wstring& str) const
{
  if (str.find_first_of(L',') == wstring::npos)
  {
    return str;
  }

  wstring ret(L"\"");
  ret += str;
  ret.push_back(L'\"');
  return ret;
}

template class CsvProcessorGoogle<
  CsvFieldCounts::s_indexGoogle,
  CsvFieldCounts::s_processingGoogle>;
