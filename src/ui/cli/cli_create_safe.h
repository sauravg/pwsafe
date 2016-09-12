//
//  cli_create_safe.h
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 11/09/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#ifndef pwsafe_xcode6_cli_create_safe_h
#define pwsafe_xcode6_cli_create_safe_h

#include "./cli_task.h"

class cli_create_safe: public cli_task
{
public:
  cli_create_safe(): cli_task{'c', "new", no_argument}{}
  static string_vec long_help();
  static wstring short_help() {
    return L"--new ";
  }
  virtual int execute(PWScore &core, const string &op_param)              override;
  friend int open_core<cli_create_safe>(PWScore &core, const StringX &safe);
  friend int save_core<cli_create_safe>(PWScore &core, const cli_create_safe &t);
};

template <>
inline int open_core<cli_create_safe>(PWScore &core, const StringX &safe) {
  core.SetCurFile(safe);
  return PWScore::SUCCESS;
}

// A new safe is not marked as "changed", so we override this
// here to ensure a new safe is alway saved, unless it was a dry run
template <>
inline int save_core<cli_create_safe>(PWScore &core, const cli_create_safe &t)
{
  const cli_task& base = t;
  return !t.dry_run? SaveCore(core, base.dry_run):
              PWScore::SUCCESS;
}

#endif
