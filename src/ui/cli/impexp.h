#pragma once

#include "./cli_task.h"

class PWScore;
struct UserArgs;

int Import(PWScore &core, const UserArgs &ua);
int Export(PWScore &core, const UserArgs &ua);

class cli_impexp: public cli_task {

protected:
  enum class FileType { text, xml } format = FileType::text;

public:
  using cli_task::cli_task;
  virtual bool handle_arg( const char *name, const char *val) override;
};

class cli_import: public cli_impexp
{
public:
  cli_import(): cli_impexp('i', "import", optional_argument) {}
  static string_vec long_help();
  static wstring short_help() {
    return L"--import[=<file>] | -i [<file>] [ --text | --xml ]";
  }
  virtual int execute(PWScore &core);
};

class cli_export: public cli_impexp
{
public:
  cli_export(): cli_impexp('e', "export", optional_argument) {}
  static string_vec long_help();
  static wstring short_help() {
    return L"--export[=<file>] | -e [<file>] [ --text | --xml ]";
  }
  virtual bool is_dirty() const                   override { return false; }
  virtual int execute(PWScore &core)              override;
};
