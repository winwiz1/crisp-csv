/*
  IWorkUnit represents a unit of work (possibly queueable to thread pool
  in the future versions).
*/
#pragma once

enum struct ExitCode
{
  E_SUCCESS,
  E_SIGINT,
  E_EXCEPTION,
  E_ERROR
};

struct IWorkUnit
{
  virtual ~IWorkUnit() {};
  virtual ExitCode process() = 0;
  virtual unsigned getProcessedCount() = 0;
  virtual unsigned getRejectedCount() = 0;
  virtual unsigned getRejectedIndexCount() = 0;
};
