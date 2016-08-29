#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <getopt.h>

using std::vector;
using std::string;
using string_vec = vector<string>;

class PWScore;

// To define an operation implemented by pwsafe-cli, derive from this class.
class cli_task
{
  // These need to be passed in via ctor
  const char ch_op;
  const std::string str_op;  // An appropriate instance (subclass) is searched for by this field
  const int atype;
  const string_vec help_str;

public:
  cli_task(char s, const std::string &l, int t, const string_vec &h):
        ch_op{s}, str_op{l}, atype{t}, help_str{h}  {}
  virtual ~cli_task()  {}

protected:

  // Set dirty to true if the safe was modified
  bool dirty{false};

  // These are modified after construction, based on cmd-line params
  bool dry_run{false};
  bool confirmed{false};

  // This is the value (optval) passed to the main cmdline flag
  string op_param;

public:
  // This can be used to construct an array of 'option's 
  operator option () const { return {str_op.c_str(), atype, 0, ch_op}; }

  // This can be used to print the help just for this operation, or
  // the usage of the entire app
  string_vec help() const { return help_str; }

  // This can be overriden. If not, just change the protected dirty flag
  virtual bool is_dirty() const                 { return dirty; }

  // Base classes must implement this. Return false if the argument was unexpected 
  virtual bool handle_arg( const char *name, const char *value) = 0;

  // Do the thing here. Return PWScore::SUCCESS etc.
  virtual int execute(PWScore &core)            = 0;

  // This is called from main(). Should not be overriden
  virtual void parse_args(const char *val, int argc, char *argv[]) throw(std::invalid_argument) final;
};


class cli_search: public cli_task
{
  cli_search(int argc, char *argv[]);
public:
  virtual bool is_dirty() const                   override;
  virtual int execute(PWScore &core)              override;
};
