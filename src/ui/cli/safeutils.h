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
  cli_create_safe();
  static string_vec long_help();
  static wstring short_help();
  virtual int execute(PWScore &core)              override;
  virtual bool handle_arg( const char *name, const char *value) override;
};

class cli_add_entry: public cli_task
{
public:
  cli_add_entry();
  static string_vec long_help();
  static wstring short_help();
  virtual bool is_dirty() const                   override;
  virtual int execute(PWScore &core)              override;
  virtual bool handle_arg( const char *name, const char *value) override;
};
