#pragma once

#include "./cli_task.h"

class PWScore;
struct UserArgs;

int Import(PWScore &core, const UserArgs &ua);
int Export(PWScore &core, const UserArgs &ua);

class cli_impexp: public cli_task {

protected:
  enum class InputType { text, xml } format = InputType::text;

public:
  using cli_task::cli_task;
  virtual bool handle_arg( const char *name, const char *val) override;
};

class cli_import: public cli_impexp
{
public:
  cli_import(): cli_impexp('i', "import", optional_argument, {
    { "--import[=<file>] | -i [<file>] [ --text | --xml ]" }
  }) {}
  virtual int execute(PWScore &core)              override {return 0;}
};

class cli_export: public cli_impexp
{
public:
  cli_export(): cli_impexp('e', "export", optional_argument, {
    { "--export[=<file>] | -e [<file>] [ --text | --xml ]" }
  }) {}
  virtual bool is_dirty() const                   override { return false; }
  virtual int execute(PWScore &core)              override;
};
