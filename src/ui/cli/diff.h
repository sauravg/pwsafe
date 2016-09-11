#pragma once

#include "./cli_task.h"
#include "./argutils.h"

class PWScore;

int Diff(PWScore &core, const UserArgs &ua);

class cli_diff: public cli_task
{
  CItemData::FieldBits fields;
  Restriction subset;
  UserArgs::DiffFmt fmt{UserArgs::DiffFmt::Unified};
  unsigned int colwidth{60};
public:
  cli_diff(): cli_task{'c', "diff", required_argument}{}
  static string_vec long_help();
  static wstring short_help() {
    return L"--diff=<other-safe>  [--subset=<Field><OP><Value>[/iI]"
           L" [--fields=f1,f2,..] [--unified|--context|--sidebyside]"
           L" [--colwidth=column-size]";
  }
  virtual bool is_dirty() const                   override {return false; }
  virtual int execute(PWScore &core)              override;
  virtual bool handle_arg( const char *name, const char *value) override;
};

template <>
inline int save_core<cli_diff>(PWScore &core, const cli_diff &) {
  return PWScore::SUCCESS;
}

