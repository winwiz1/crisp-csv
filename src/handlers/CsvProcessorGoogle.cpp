#include <cassert>
#include <thread>
#include <codecvt>
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
const set<wstring>
CsvProcessorGoogle<InputFieldCount, OutputFieldCount>::s_shortLocalities{ L"Bo" };

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
  const locale utf8_loc = locale(std::locale(), new std::codecvt_utf8<wchar_t>);
  m_inStream.imbue(utf8_loc);
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

  typedef enum {
                  E_UNSPECIFIED,
                  E_REPETITION,
                  E_AGG_LEVEL,
                  E_REGEX,
                  E_MISMATCH,
                  E_DATA,
                  E_LENGTH,
                  E_LOCALITY,
                  E_LOCALITY_LENGTH
                } E_REJECTION_REASON;

  auto saveRejectedRow = [this, &rowReader](E_REJECTION_REASON reason = E_UNSPECIFIED) {
      const auto& row = rowReader.getReadonlyRow();
      switch (reason)
      {
        case E_REPETITION:
          m_rejectStream << L"Repetition: ";
          break;
        case E_AGG_LEVEL:
          m_rejectStream << L"Invalid aggregation level: ";
          break;
        case E_REGEX:
          m_rejectStream << L"Invalid index literal: ";
          break;
        case E_MISMATCH:
          m_rejectStream << L"Mismatch between aggregation level and index literal: ";
          break;
        case E_DATA:
          m_rejectStream << L"State/province data is inconsistent with index literal: ";
          break;
        case E_LENGTH:
          m_rejectStream << L"Invalid country/state/province length: ";
          break;
        case E_LOCALITY:
          m_rejectStream << L"Locality (subregion2_name and/or locality_name) data is inconsistent with index literal: ";
          break;
        case E_LOCALITY_LENGTH:
          m_rejectStream << L"Locality (subregion2_name and/or locality_name) data has invalid length: ";
          break;
        case E_UNSPECIFIED:
          break;
        default:
          m_rejectStream << L"Unexpected rejection reason: ";
          break;
      };
      copy(row.cbegin(), row.cend(), ostream_custom_iterator<wstring>(m_rejectStream, L","));
      m_rejectStream << L'\n';
  };

  cout << APP_TITLE" - processing index" << endl;

  while (g_SIGINT == 0)
  {
    using csv_error = typename CsvRowReader<InputFieldCount>::csv_error;
    utility::ScopedAction sa(incrementRowCount);

    try
    {
      if (!(m_inStream >> rowReader))
        break;
    }
    catch (const csv_error& ex)
    {
      ++m_countRejected;
      m_rejectStream << L"Invalid CSV data: " << ex.data() << L'\n';
      continue;
    }

    const auto& aggLevel = rowReader[m_indices.back()];
    unsigned long level = 0L;
    try
    {
      level = std::stoul(aggLevel);
    }
    catch (const exception&)
    {
      ++m_countRejected;
      saveRejectedRow(E_AGG_LEVEL);
      continue;
    }

    const auto& index = rowReader[m_indices.front()];
    wsmatch mr;
    if (bool bMatch = regex_match(index, mr, utility::g_regexIndex); !bMatch)
    {
      ++m_countRejected;
      saveRejectedRow(E_REGEX);
      continue;
    }

    static bool relaxIndexChecks = RuntimeConfig::GetInstance().getRelaxIndexChecks();

    if (level > 3L ||
        (level == 0L && (mr[1].matched || mr[2].matched)) ||
        (level == 1L && (!mr[1].matched || mr[2].matched)) ||
        (level >= 2L && (!mr[1].matched || (!mr[2].matched && !relaxIndexChecks))))
    {
      ++m_countRejected;
      saveRejectedRow(E_MISMATCH);
      continue;
    }

    const auto& countryName = rowReader[m_indices[1]];
    const auto& stateName = rowReader[m_indices[2]];

    if (!relaxIndexChecks && mr[1].matched == stateName.empty())
    {
      // Reject rows with state/province data and index asserting absense of this data
      // Reject rows with missing state/province data and index asserting presense of this data
      ++m_countRejected;
      saveRejectedRow(E_DATA);
      continue;     
    }

    if (countryName.length() < s_minCountryNameLen ||
        (!stateName.empty() && stateName.length() < s_minStateNameLen))
    {
      ++m_countRejected;
      saveRejectedRow(E_LENGTH);
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
        const auto& outcome = m_map.emplace(make_pair(index, make_tuple(countryName, stateName)));

        if (!outcome.second)
        {
          ++m_countRejected;
          saveRejectedRow(E_REPETITION);
          continue;
        }
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
        saveRejectedRow(E_LOCALITY);
        continue;
      }

      if (!localityName.empty() && localityName.length() < s_minLocalityNameLen)
      {
        const bool bFound = s_shortLocalities.find(localityName) != s_shortLocalities.end();

        if (!bFound)
        {
          ++m_countRejected;
          saveRejectedRow(E_LOCALITY_LENGTH);
          continue;
        }
      }

      const auto& outcome = m_map.emplace(make_pair(index, make_tuple(countryName, stateName, localityName, level)));

      if (!outcome.second)
      {
        ++m_countRejected;
        saveRejectedRow(E_REPETITION);
        continue;
      }
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

  if (g_SIGINT)
  {
     cerr << APP_TITLE" - index processing has been interrupted and is incomplete" << endl;
  }
  else
  {
    cout << APP_TITLE" - index processing finished" << endl;
    cout << APP_TITLE" - processed " << rowCount << " index rows" << endl;
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
    typename Base::OutputFields ret{wField, country, state, locality, aggLevel};
    return ret;
  }
}

template class CsvProcessorGoogle<
  CsvFieldCounts::s_indexGoogle,
  CsvFieldCounts::s_processingGoogle>;
