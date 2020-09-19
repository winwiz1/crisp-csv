#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <mutex>
#include "utility.h"

using namespace std;

namespace
{
  once_flag s_flag1;
  static const char *const s_selfLink = "/proc/self/exe";
}

[[gnu::pure]]
string utility::constructPath(const string& strPath)
{
  if (strPath.empty() || strPath.front() != '/')
  {
     throw_exception<std::invalid_argument>("the requested path is invalid");
  }

  char buf[PATH_MAX];
  ::memset(buf, '\0', sizeof(buf));
  auto len = readlink(s_selfLink, buf, sizeof(buf) - 1);

  if (len < 0)
  {
    throw_exception<std::runtime_error>("readlink failed");
  }

  string ret(buf);
  auto pos = ret.find_last_of('/');
  ret.replace(pos, string::npos, strPath);
  return ret;
}

[[gnu::pure]]
string utility::getConfigFilePath(const string& strExtension)
{
  char buf[PATH_MAX];
  ::memset(buf, '\0', sizeof(buf));
  auto len = readlink(s_selfLink, buf, sizeof(buf) - 1);

  if (len < 0)
  {
    throw_exception<std::runtime_error>("readlink failed");
  }

  string ret(buf);
  ret += strExtension;
  return ret;
}

[[gnu::pure]]
const wchar_t* utility::getGmtDate()
{
  static wstring ret;

  auto getGmt = [](wstring& wstr)
  { 
    wchar_t buf[256];
    time_t curtime;
    struct tm tmtime;

    memset(buf, 0, sizeof buf);
    memset (&tmtime, 0, sizeof(struct tm));

    curtime = ::time (nullptr);
    ::gmtime_r (&curtime, &tmtime);
    ::wcsftime (buf, sizeof(buf), L"%F", &tmtime);
    wstr.assign(buf);
  };

  //getGmt(ret);
  call_once(s_flag1, getGmt, ret);
  return ret.c_str();
}
