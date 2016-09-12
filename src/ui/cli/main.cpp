/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include <iostream>
#include <sstream>
#include <getopt.h>
#include <libgen.h>
#include <string>
#include <map>
#include <array>

#include "./search.h"
#include "./argutils.h"
#include "./searchaction.h"
#include "./strutils.h"
#include "./diff.h"
#include "./safeutils.h"
#include "./impexp.h"
#include "./cli_task.h"
#include "./cli_sync.h"
#include "./cli_merge.h"
#include "./cli_create_safe.h"
#include "./cli_add_entry.h"

#include "../../core/PWScore.h"
#include "os/file.h"
#include "core/UTF8Conv.h"
#include "core/Report.h"
#include "core/XML/XMLDefs.h"

using namespace std;


//-----------------------------------------------------------------

using pre_op_fn = function<int(PWScore &, const StringX &)>;
using main_op_fn = function<int(PWScore &, const UserArgs &)>;
using post_op_fn = function<int(PWScore &, const UserArgs &)>;

wostream& usage(const char *process, wostream &os, tuple<>) { return os << endl; }

template <typename OpType, typename... Rest>
wostream& usage(const char *process, wostream &os, tuple<OpType, Rest...>) {
  os << process << L" <safe> " << OpType::short_help() << endl;
  return usage(process, os, tuple<Rest...>{});
}

using OperationTypes = std::tuple<cli_import, cli_export, cli_create_safe, cli_add_entry,
                                  cli_search, cli_diff, cli_sync, cli_merge>;

static void usage(char *pname)
{
  usage(pname, wcerr, OperationTypes{});
}

constexpr bool no_dup_short_option2(uint32_t bits, const option *p)
{
  return p->name == 0 ||
          (!(bits & (1 << (p->val - 'a'))) && no_dup_short_option2(bits | (1 << (p->val - 'a')), p+1));
}

constexpr bool no_dup_short_option(const struct option *p)
{
  return no_dup_short_option2(uint32_t{}, p);
}

// +1 to fill the last element with zeroes
using OptionsArray = array<option, std::tuple_size<OperationTypes>::value + 1>;
constexpr auto OptionsArraySize = std::tuple_size<OperationTypes>::value;

ostream& operator<<(ostream& os, const OptionsArray &ops)
{
  auto argtype = [](int a) {
    switch (a) {
      case no_argument: return "no_argument";
      case optional_argument: return "optional_argument";
      case required_argument: return "required_argument";
      default: return "invalid has_arg";
    }
  };

  for( const auto &o: ops ) {
    os << "{ " << (o.name? o.name : "NULL") << ", "
       << argtype(o.has_arg) << ", "
       << static_cast<char>(o.val? o.val: '0') << " }" << endl;
  }
  return os;
}

template <size_t N>
typename std::enable_if< (N == OptionsArraySize)>::type
GetOptions(OptionsArray &opts, char *p)
{
  opts[N] = {0};
  *p = 0;
}

template <size_t N>
typename std::enable_if< (N < OptionsArraySize)>::type
GetOptions(OptionsArray &opts, char *p)
{
  using OpType = typename std::tuple_element<N, OperationTypes>::type;
  opts[N] = OpType{};
  *p++ = opts[N].val;
  switch( opts[N].has_arg ) {
    case no_argument:
      break;
    case required_argument:
      *p++ = ':';
      break;
    case optional_argument:
      *p++ = ':';
      *p++ = ':';
      break;
    default:
      assert(false);
      throw std::logic_error("incorrectly initialized option array");
  }
  GetOptions<N+1>(opts, p);
}

int execute_cli_op(const StringX &safe, const char *opname, const char *oparg, int argc, char *argv[], tuple<>)
{
  throw invalid_argument(string("no such operation: ") + opname);
}

template <class Oper, class... Rest>
int execute_cli_op(const StringX &safe, const char *opname, const char *oparg, int argc, char *argv[], tuple<Oper, Rest...>)
{
  Oper op{};
  if ( strcmp(opname, static_cast<option>(op).name) == 0 ) {
    op.parse_args(argc, argv);
    PWScore core;
    try {
#pragma message("Handle new safe creation for --new")
      int status = open_core<Oper>(core, safe);
      if ( status == PWScore::SUCCESS) {
        status = op.execute(core, oparg? oparg: "");
        if (status == PWScore::SUCCESS)
          status = save_core<Oper>(core, op);
      }
    }
    catch(const exception &e) {
      wcerr << e.what() << endl;
      return PWScore::FAILURE;
    }

    core.UnlockFile(safe.c_str());
    return PWScore::SUCCESS;
  }
  else {
    return execute_cli_op(safe, opname, oparg, argc, argv, tuple<Rest...>{});
  }
}

int main(int argc, char *argv[])
{
  char optstring[OptionsArraySize*3 + 1] = {0};

  OptionsArray opts;
  GetOptions<0>(opts, optstring);


  if (argc < 3) {
    // must give us a safe and an operation
    usage(basename(argv[0]));
    return -1;
  }

  const StringX safe{str2StringX(argv[1])};

  int option_index = 0;

  int c = getopt_long(argc, argv, optstring, opts.cbegin(), &option_index);
  switch (c) {
    case -1:
      // even if the arg is invalid, we should get '?'
      assert(false);
      return PWScore::FAILURE;
    case ':':
    case '?':
      usage(basename(argv[0]));
      return PWScore::FAILURE;
    default:
      try {
        return execute_cli_op(safe, opts[option_index].name, optarg, argc-3, argv+3, OperationTypes{});
      }
      catch( const std::exception &e ) {
        wcerr << L"Error: " << str2wstr(e.what()) << endl;
        return PWScore::FAILURE;
      }
  }
}
