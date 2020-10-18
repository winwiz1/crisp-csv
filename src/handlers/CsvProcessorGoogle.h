/*
  Implementation of the CsvProcessor interface for
  Google COVID-19 Open Data repository.
*/
#pragma once

#include <regex>
#include <fstream>
#include <unordered_map>
#include <type_traits>

#include "../config/BuildConfig.h"
#include "CsvProcessor.h"
#include "../utility.h"

template <
  std::size_t InputFieldCount = CsvFieldCounts::s_indexGoogle,
  std::size_t OutputFieldCount = CsvFieldCounts::s_processingGoogle>
class CsvProcessorGoogle : public CsvProcessor<InputFieldCount, OutputFieldCount>
{
public:

  typedef CsvProcessor<InputFieldCount, OutputFieldCount> Base;
  typedef std::conditional<
    g_skipLocalitiesBelowStateOrProvince == true,
    // key: index, value: tuple<country name, state name>
    std::unordered_map<std::wstring, std::tuple<std::wstring,std::wstring>>,
    // key: index, value: tuple<country name, state name, locality_name, aggregation_level>
    // Need aggregation level otherwise it's not clear if the 3rd tuple's string is
    // subregion2_name (L2) or locality_name (L3)
    std::unordered_map<std::wstring, std::tuple<std::wstring,std::wstring,std::wstring,unsigned long>>
  >::type LookupMap;

  CsvProcessorGoogle(const std::string& inFile, typename Base::InputFields&& indices);
  ~CsvProcessorGoogle() = default;

protected:
  using Base::m_indices;
  using Base::m_countRejected;
  using Base::m_countFiltered;
  bool check_streams() const;
  bool build_dictionary() noexcept(false);

  LookupMap m_map;
  std::wifstream m_inStream;
  std::wofstream m_rejectStream;

  static const int s_yieldFrequency = 100;
  static const unsigned s_minCountryNameLen = 4;
  static const unsigned s_minStateNameLen = 3;
  static const unsigned s_minLocalityNameLen = 3;
  
private:
  typename Base::OutputFields process_internal(const std::wstring& field) const noexcept(false) override;
};
