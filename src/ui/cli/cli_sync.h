#pragma once

#include "./cli_task.h"
#include "./argutils.h"

int Sync(PWScore &core, const UserArgs &ua);

class cli_sync: public cli_task
{
  Restriction subset;
  CItemData::FieldBits fields;
public:
  cli_sync(): cli_task{'z', "synchronize", required_argument}{ fields.set(); }
  static string_vec long_help();
  static wstring short_help() {
    return L"--sync=<other-safe>  [--subset=<Field><OP><string>[/iI]] [--fields=f1,f2,..]";
  }
  virtual int execute(PWScore &core)              override;
  virtual bool handle_arg( const char *name, const char *value) override;
};

