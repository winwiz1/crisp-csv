/*
  The configuration used at build time.
*/

#pragma once

#define STRINGIZER(x) #x
#define STRINGIFY(x) STRINGIZER(x)
#define BOOLINGER(x) static_cast<bool>(x)

#if defined(SKIP_LOCALITIES)
  constexpr bool g_skipLocalitiesBelowStateOrProvince = BOOLINGER(SKIP_LOCALITIES);
#else
  constexpr bool g_skipLocalitiesBelowStateOrProvince = false;
#endif

struct CsvFieldCounts
{
  // How many fields are to be extracted from Google input data file
  static const unsigned s_inputGoogle;
  // How many fields are needed to build lookup dictionary from Google index file
  static const unsigned s_indexGoogle;
  // How many fields can be included into the output of one CSV field processing
  static const unsigned s_processingGoogle;
};

// 5 fields: date, index and 3 epidemiology cumulative counts
constexpr inline unsigned CsvFieldCounts::s_inputGoogle = 5;
// Either 4 or 6 fields: index, aggregation level, country name, state/province name
// and possibly locality data: both subregion2_name and locality_name.
constexpr inline unsigned CsvFieldCounts::s_indexGoogle = 4 + (g_skipLocalitiesBelowStateOrProvince ? 0 : 2);
// Either 4 or 5 fields: index, aggregation level, country name, state/province name and
// possibly locality data: either subregion2_name or locality_name.
constexpr inline unsigned CsvFieldCounts::s_processingGoogle = 4 + (g_skipLocalitiesBelowStateOrProvince ? 0 : 1);
