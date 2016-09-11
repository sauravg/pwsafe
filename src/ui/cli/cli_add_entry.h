//
//  cli_add_entry.h
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 11/09/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#include "./cli_task.h"

#ifndef pwsafe_xcode6_cli_add_entry_h
#define pwsafe_xcode6_cli_add_entry_h

class cli_add_entry: public cli_task
{
public:
  cli_add_entry(): cli_task{'a', "add", required_argument}{}
  static string_vec long_help();
  static wstring short_help() {
    return L"--add=field1=value1,field2=value2,...";
  }
  virtual int execute(PWScore &core, const string &op_param)              override;
};

#endif
