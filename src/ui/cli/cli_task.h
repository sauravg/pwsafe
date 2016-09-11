#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <getopt.h>

#include "../../core/StringX.h"
#include "./safeutils.h"

using std::vector;
using std::string;
using std::wstring;

using string_vec = vector<wstring>;

class PWScore;

// To define an operation implemented by pwsafe-cli, derive from this class.
class cli_task
{
  // These need to be passed in via ctor
  const char ch_op;
  const char *str_op;  // An appropriate instance (subclass) is searched for by this field
  const int atype;

public:
  cli_task(char s, const char *l, int t):
        ch_op{s}, str_op{l}, atype{t}  {}
  virtual ~cli_task()  {}

protected:

  // Set dirty to true if the safe was modified
  bool dirty{false};

  // These are modified after construction, based on cmd-line params
  bool dry_run{false};
  bool confirmed{false};

public:
  // This can be used to construct an array of 'option's 
  operator option () const { return {str_op, atype, 0, ch_op}; }

  // This can be used to print the help just for this operation, or
  // the usage of the entire app
  //static string_vec long_help();
  //static string short_help();

  // This can be overriden. If not, just change the protected dirty flag
  virtual bool is_dirty() const                 { return dirty; }

  // Base classes must implement this if they expect more arguments than the value
  // passed with the main operation. Return false if the argument was unexpected
  virtual bool handle_arg( const char * /*name*/, const char * /*value*/) {
    return false;
  }

  // Do the thing here. Return PWScore::SUCCESS etc.
  virtual int execute(PWScore &core, const string &op_param)            = 0;

  // This is called from main(). Should not be overriden
  virtual void parse_args(int argc, char *argv[]) throw(std::invalid_argument) final;

  template <class TaskType>
  friend int save_core(PWScore &core, const TaskType &t);
};

template <class TaskType>
int open_core(PWScore &core, const StringX &safe)
{
  return OpenCore(core, safe);
}

template <class TaskType>
int save_core(PWScore &core, const TaskType &t)
{
  const cli_task& base = t;
  return SaveCore(core, base.dry_run);
}
