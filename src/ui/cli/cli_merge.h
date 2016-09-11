#pragma once

#include "./cli_task.h"
#include "./argutils.h"

int Merge(PWScore &core, const UserArgs &ua);

class cli_merge: public cli_task
{
  Restriction subset;
public:
  cli_merge(): cli_task{'m', "merge", required_argument}{}
  static string_vec long_help();
  static wstring short_help() {
    return L"--merge=<other-safe> [--subset=<Field><OP><Value>[/iI]]";
  }
  virtual int execute(PWScore &core, const string &op_param)              override;
  virtual bool handle_arg(const char *name, const char *value) override;
};

