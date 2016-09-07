//
//  strutils.cpp
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 19/06/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#include <string>
#include <iostream>

#include "strutils.h"

#include "../../core/UTF8Conv.h"
#include "../../core/StringX.h"
#include "../../core/PWScore.h"

using namespace std;

void Utf82StringX(const char* str, StringX& sname)
{
  CUTF8Conv conv;
  if (!conv.FromUTF8((const unsigned char *)str, strlen(str),
                     sname)) {
    cerr << "Could not convert " << str << " to StringX" << endl;
    exit(2);
  }
}

wstring Utf82wstring(const char* utf8str)
{
  StringX sx;
  Utf82StringX(utf8str, sx);
  return stringx2std(sx);
}

std::wstring str2wstr(const std::string &str)
{
  if (str.empty())
    return std::wstring{};
  return Utf82wstring(str.c_str());
}

StringX str2StringX(const std::string &str)
{
  if (str.empty()) return StringX{};
  StringX sx;
  Utf82StringX(str.c_str(), sx);
  return sx;
}

const char *status_text(int status)
{
  switch (status) {
  case PWScore::SUCCESS: return "SUCCESS";
  case PWScore::FAILURE: return "FAILURE";
  case PWScore::CANT_OPEN_FILE: return "CANT_OPEN_FILE";
  case PWScore::USER_CANCEL: return "USER_CANCEL";
  case PWScore::WRONG_PASSWORD: return "WRONG_PASSWORD";
  case PWScore::BAD_DIGEST: return "BAD_DIGEST";
  case PWScore::UNKNOWN_VERSION: return "UNKNOWN_VERSION";
  case PWScore::NOT_SUCCESS: return "NOT_SUCCESS";
  case PWScore::ALREADY_OPEN: return "ALREADY_OPEN";
  case PWScore::INVALID_FORMAT: return "INVALID_FORMAT";
  case PWScore::USER_EXIT: return "USER_EXIT";
  case PWScore::XML_FAILED_VALIDATION: return "XML_FAILED_VALIDATION";
  case PWScore::XML_FAILED_IMPORT: return "XML_FAILED_IMPORT";
  case PWScore::LIMIT_REACHED: return "LIMIT_REACHED";
  case PWScore::UNIMPLEMENTED: return "UNIMPLEMENTED";
  case PWScore::NO_ENTRIES_EXPORTED: return "NO_ENTRIES_EXPORTED";
  default: return "UNKNOWN ?!";
  }
}

wostream & operator<<( std::wostream &os, const st_GroupTitleUser &gtu)
{
  if ( !gtu.group.empty() )
    os << gtu.group << L" >> ";

  assert( !gtu.title.empty() );
  os << gtu.title;

  if ( !gtu.user.empty() ) {
    os << L'[' << gtu.user << L']';
  }

  return os;
}

