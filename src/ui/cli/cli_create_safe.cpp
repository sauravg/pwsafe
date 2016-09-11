//
//  cli_create_safe.cpp
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 11/09/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#include "./cli_create_safe.h"
#include "./safeutils.h"

#include <string>
#include "../../core/PWScore.h"

string_vec cli_create_safe::long_help()
{
  string_vec help{ short_help() };
  return help;
}

int cli_create_safe::execute(PWScore &core, const string &/*op_param*/)
{
  int status = CreateNewSafe(core, core.GetCurFile());
  if ( status == PWScore::SUCCESS) dirty = true;
  return status;
}

