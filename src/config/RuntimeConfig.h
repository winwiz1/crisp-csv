/*
  Run-time configuration.
  Uses JSON configuration file.
*/

#pragma once

#include "json.hpp"

struct IRuntimeConfig
{
  virtual ~IRuntimeConfig() {}
  virtual bool getFilterAuData() const = 0;
  virtual bool getFilterUkNuts() const = 0;
  virtual bool getRelaxIndexChecks() const = 0;
  virtual unsigned getProcessedDataRowsThreshold() const = 0;
  virtual unsigned getRejectedDataRowsThreshold() const = 0;
  virtual unsigned getRejectedIndexRowsThreshold() const = 0;
};

class RuntimeConfig : public IRuntimeConfig
{
public:
  static const IRuntimeConfig& GetInstance()
  {
    static RuntimeConfig instance;
    return instance;
  }

protected:
  bool getFilterAuData() const override { return m_filterAuData; }
  bool getFilterUkNuts() const override { return m_filterUkNuts; }
  bool getRelaxIndexChecks() const override { return m_relaxIndexChecks; }
  unsigned getProcessedDataRowsThreshold() const override { return m_processedDataRowsThreshold; }
  unsigned getRejectedDataRowsThreshold() const override { return m_rejectedDataRowsThreshold; }
  unsigned getRejectedIndexRowsThreshold() const override { return m_rejectedIndexRowsThreshold; }

protected:
  RuntimeConfig();
  RuntimeConfig(const RuntimeConfig&);

  RuntimeConfig& operator=(const RuntimeConfig&);
  RuntimeConfig& operator=(const nlohmann::json&);

  void readConfigFile();

  // Filter UK NUTS regions
  bool m_filterUkNuts;
  // Conditionally filter today's AU data
  bool m_filterAuData;
  // Skip some index checks
  bool m_relaxIndexChecks;
  // Thresholds that affect program's exit code
  unsigned m_processedDataRowsThreshold;
  unsigned m_rejectedDataRowsThreshold;
  unsigned m_rejectedIndexRowsThreshold;

  // The default values used if the configuration file is not found
  static const bool s_filterUkNuts = true;
  static const bool s_filterAuData = true;
  static const bool s_relaxIndexChecks = false;
  static const unsigned s_processedDataRowsThreshold = 1500000;
  static const unsigned s_rejectedDataRowsThreshold = 100;
  static const unsigned s_rejectedIndexRowsThreshold = 10;
};
