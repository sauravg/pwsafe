//
//  cli_add_entry.cpp
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 11/09/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#include "./cli_add_entry.h"
#include "./safeutils.h"

#include "../../core/PWScore.h"

#include <iostream>

using std::wcerr;

string_vec cli_add_entry::long_help()
{
  string_vec help{ short_help() };
  const string_vec fields_help{get_fields_help()};
  help.insert(help.end(), fields_help.begin(), fields_help.end());
  return help;
}

int cli_add_entry::execute(PWScore &core, const string &op_param)
{
  int status = AddEntryWithFields(core, ParseFieldValues(str2wstr(op_param)), wcerr);
  if (status == PWScore::SUCCESS) dirty = true;
  return status;
}
