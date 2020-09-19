/*
  Various assorted routines the program's toolbox is comprised of.
*/
#pragma once

#include <regex>
#include <functional>
#include <string.h>

namespace utility
{
  struct ScopedAction
  {
    ScopedAction(std::function<void()> action) : m_action(action)
    {
    }
    ~ScopedAction()
    {
      m_action();
    }

  private:
    std::function<void()> m_action;
  };

  struct Counter
  {
    Counter() : m_countRejected(0), m_countFiltered(0) {}
    virtual ~Counter() {}

    auto getRejectedCount() { return m_countRejected; }
    auto filteredCount() { return m_countFiltered; }

  protected:
    unsigned m_countRejected;
    unsigned m_countFiltered;
  };

  template <typename T>
  [[noreturn]] void throw_exception(const char* msg)
  {
    char buf[256] = "CsvProcessor - ";
    unsigned len = strlen(buf);
    strncat(buf, msg, sizeof(buf) - (len + 1));
    throw T(buf);
  }

  std::string constructPath(const std::string& strPath);
  std::string getConfigFilePath(const std::string& strExtension = ".cfg");
  const wchar_t* getGmtDate();

  const inline std::wregex g_regexIndex{L"^[A-Z]{2}(_[A-Z0-9]{1,3})?(_[A-Za-z0-9\\u0080-\\uDB7F]{1,12})?$"};
} // namespace utility
