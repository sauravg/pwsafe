#pragma once

#include "./cli_task.h"
#include "./argutils.h"

class PWScore;

int Diff(PWScore &core, const UserArgs &ua);

class cli_diff: public cli_task
{
  UserArgs::FieldUpdates fieldValues;
  Restriction subset;
  UserArgs::DiffFmt fmt;

public:
  cli_diff();
  static string_vec long_help();
  static wstring short_help();
  virtual bool is_dirty() const                   override {return false; }
  virtual int execute(PWScore &core)              override;
  virtual bool handle_arg( const char *name, const char *value) override;
};

