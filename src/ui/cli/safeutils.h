#pragma once

#include "../../core/StringX.h"
#include "./argutils.h"
#include "./cli_task.h"

class PWScore;
struct PWPolicy;

int OpenCore(PWScore& core, const StringX& safe);
int SaveCore(PWScore &core, const UserArgs &);

StringX GetNewPassphrase();

int CreateNewSafe(PWScore &core, const StringX& filename);

int AddEntry(PWScore &core, const UserArgs &ua);

int InitPWPolicy(PWPolicy &pwp, PWScore &core);

class cli_create_safe: public cli_task
{
public:
  cli_create_safe(): cli_task{'c', "new", no_argument}{}
  static string_vec long_help();
  static wstring short_help() {
    return L"--new ";
  }
  virtual int execute(PWScore &core)              override;
};

class cli_add_entry: public cli_task
{
public:
  cli_add_entry(): cli_task{'a', "add", required_argument}{}
  static string_vec long_help();
  static wstring short_help() {
    return L"--add=field1=value1,field2=value2,...";
  }
  virtual int execute(PWScore &core)              override;
};
