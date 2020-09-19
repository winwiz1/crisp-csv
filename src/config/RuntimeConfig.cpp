#include <fstream>
#include <iostream>
#include "../utility.h"
#include "RuntimeConfig.h"

using namespace std;
namespace nh = nlohmann;

namespace
{
  const string filterAuDataLiteral("filterAuData");
  const string filterUkNutsLiteral("filterUkNuts");
  const string relaxIndexChecksLiteral("relaxIndexChecks");
  const string processedDataLiteral("processedDataRows");
  const string rejectedDataLiteral("rejectedDataRows");
  const string rejectedIndexLiteral("rejectedIndexRows");
}

RuntimeConfig::RuntimeConfig() :
  m_filterUkNuts(s_filterUkNuts),
  m_filterAuData(s_filterAuData),
  m_relaxIndexChecks(s_relaxIndexChecks),
  m_processedDataRowsThreshold(s_processedDataRowsThreshold),
  m_rejectedDataRowsThreshold(s_rejectedDataRowsThreshold),
  m_rejectedIndexRowsThreshold(s_rejectedDataRowsThreshold)
{
  readConfigFile();
};

RuntimeConfig::RuntimeConfig(const RuntimeConfig& c)
{
  *this = c;
}

RuntimeConfig& RuntimeConfig::operator=(const RuntimeConfig& c)
{
  m_filterUkNuts = c.m_filterUkNuts;
  m_filterAuData = c.m_filterAuData;
  m_relaxIndexChecks = c.m_relaxIndexChecks;
  m_processedDataRowsThreshold = c.m_processedDataRowsThreshold;
  m_rejectedDataRowsThreshold = c.m_rejectedDataRowsThreshold;
  m_rejectedIndexRowsThreshold = c.m_rejectedIndexRowsThreshold;
  return *this;
}

RuntimeConfig& RuntimeConfig::operator=(const nh::json& j)
{
  m_filterUkNuts = j[filterUkNutsLiteral];
  m_filterAuData = j[filterAuDataLiteral];
  m_relaxIndexChecks = j[relaxIndexChecksLiteral];
  m_processedDataRowsThreshold = j[processedDataLiteral];
  m_rejectedDataRowsThreshold = j[rejectedDataLiteral];
  m_rejectedIndexRowsThreshold = j[rejectedIndexLiteral];
  return *this;
}

void RuntimeConfig::readConfigFile()
{
  try
  {
    string configPath = utility::getConfigFilePath();

    if (ifstream ifs(configPath); !ifs.is_open())
    {
      return;
    }
    else
    {
      nh::json j = nh::json::parse(ifs);
      *this = j;
    }
  }
  catch (const exception& ex)
  {
    cerr << APP_TITLE " - Exception " << typeid(ex).name() << ": " << ex.what() << endl;
    throw;
  }
}
