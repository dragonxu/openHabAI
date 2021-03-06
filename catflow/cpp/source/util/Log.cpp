/* 
 * File:   Log.h
 * Author: Joshua Johannson
 *
  */
#include <Catflow.h>
#include <cstdio>
#include <bitset>
#include "util/Log.h"


// static
char Log::logLevelMask    = LOG_LEVEL_ALL;
bool Log::useLongLogNames = false;
bool Log::useColors       = false;
int  Log::maxNameLongLen  = 0;
int  Log::logNameShortLongest = 0;


Log::Log()
{}

// -- SET LOG NAME ----------------------
Log::Log(string nameShort)
{
  setLogName(nameShort);
}

void Log::setLogName(string nameShort)
{
  this->logNameShort = nameShort;
  this->logNameLong  = "";
  if (logNameShortLongest < nameShort.length())
    logNameShortLongest = nameShort.length();
}
void Log::setLogName(string nameShort, string nameLong)
{
  setLogName(nameShort);
  this->logNameLong = nameLong;

  if (logNameLong.length() > maxNameLongLen)
    maxNameLongLen = logNameLong.length();
}

void Log::setLogLevel(char log_levelMask)
{
  char log_level = 0;
  switch(log_levelMask)
  {
    case LOG_LEVEL_ALL:
      log_level = LOG_LEVEL_ALL;
      break;
    case LOG_LEVEL_TRACE:
      log_level |= LOG_LEVEL_TRACE;
    case LOG_LEVEL_INFORMATION:
      log_level |= LOG_LEVEL_INFORMATION;
    case LOG_LEVEL_DEBUG:
      log_level |= LOG_LEVEL_DEBUG;
    case LOG_LEVEL_OK:
      log_level |= LOG_LEVEL_OK;
    case LOG_LEVEL_WARNING:
      log_level |= LOG_LEVEL_WARNING;
    case LOG_LEVEL_ERROR:
      log_level |= LOG_LEVEL_ERROR;
    case LOG_LEVEL_FATAL:
      log_level |= LOG_LEVEL_FATAL;
      break;
    default:
      log_level = LOG_LEVEL_ALL;
      break;
  }
  setLogMask(log_level);
  // cout << "set log level: " << bitset<8>(logLevelMask) << endl;
}

void Log::setLogMask(char log_mask)
{
  logLevelMask = log_mask;
}

void Log::setUseLongLogNames(bool use)
{
  useLongLogNames = use;
}

void Log::setUseColor(bool use)
{
  useColors = use;
}


// -- OUT -------------------------------
void Log::fatal(string text)
{
  print("[❎fatal ❎] ", text, LOG_LEVEL_ERROR);
//  Catflow::send("fatal", {{"message",  text + "''"}});
}

void Log::fatal(string text, string error)
{
  print("[❎fatal ❎] ", text + ": '" + error + "'", LOG_LEVEL_ERROR);
  //Catflow::send("fatal", {{"message",  text + "'" + error + "'"}});
}

void Log::err(string text)
{
  if ((logLevelMask & LOG_LEVEL_ERROR) > 0)
    print("[❌ err ❌] ", text, LOG_LEVEL_ERROR);
}

void Log::err(string text, string error)
{
  if ((logLevelMask & LOG_LEVEL_ERROR) > 0)
    print("[❌ err ❌] ", text + ": '" + error + "'", LOG_LEVEL_ERROR);
}

void Log::warn(string text)
{
  if ((logLevelMask & LOG_LEVEL_WARNING) > 0)
    print("[⚠ warn ⚠] ", text, LOG_LEVEL_WARNING);
}

void Log::ok(string text)
{
  if ((logLevelMask & LOG_LEVEL_OK) > 0)
    print("[  ok ✅ ] ", text, LOG_LEVEL_OK);
}

void Log::debug(string text)
{
  if ((logLevelMask & LOG_LEVEL_DEBUG) > 0)
    print("[ debug  ] ", text, LOG_LEVEL_DEBUG);
}

void Log::info(string text) const
{
  if ((logLevelMask & LOG_LEVEL_INFORMATION) > 0)
    print("[ info ❄ ] ", text, LOG_LEVEL_INFORMATION);
}

void Log::trace(string text)
{
  if ((logLevelMask & LOG_LEVEL_TRACE) > 0)
    print("[ trac   ] ", text, LOG_LEVEL_TRACE);
}

void Log::outErr(string text, bool error, string errorText)
{
  if (error && ((logLevelMask & LOG_LEVEL_ERROR) > 0))
    print("[❌ err ❌] ", text + ": '" + errorText + "'", LOG_LEVEL_ERROR);
  else
    this->ok(text);
}

void Log::outErr(string text, bool error)
{
  outErr(text, error, "");
}

void Log::outFat(string text, bool error, string errorText)
{
  if (error && ((logLevelMask & LOG_LEVEL_ERROR) > 0))
    print("[❎fatal ❎] ", text + ": '" + errorText + "'", LOG_LEVEL_ERROR);
  else
    this->ok(text);
}

void Log::outFat(string text, bool error)
{
  outErr(text, error, "");
}



void Log::print(string tag, string text, char tagMask) const
{
  // output
  char name[256];
  snprintf(name, 256, string("%-" + to_string(logNameShortLongest) + "s").c_str(), logNameShort.c_str());
  cout << "[ " << name << " ] " << tag << text  << endl;

  /*
  if (!useLongLogNames)
    cout << "[ " << logNameShort/*.substr(0, 14)*-/ << " ] " << tag << "" << text << endl;
  else
  {
    if (logNameLong != "")
      cout << "[ " << logNameLong.substr(0, maxNameLongLen) << " ] " << tag << text  << endl;
    else
      cout << "[ " << logNameShort.substr(0, maxNameLongLen) << " ] " << tag << text << endl;
  }
  */
}
