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

auto null_op = [](PWScore &, const UserArgs &)-> int{ return PWScore::SUCCESS;};

struct pws_op {
  pre_op_fn pre_op;
  main_op_fn main_op;
  post_op_fn post_op;
};


const map<UserArgs::OpType, pws_op> pws_ops = {
  { UserArgs::Import,     {OpenCore,        Import,     SaveCore}},
  { UserArgs::Export,     {OpenCore,        Export,     null_op}},
  { UserArgs::CreateNew,  {CreateNewSafe,   null_op,    SaveCore}},
  { UserArgs::Add,        {OpenCore,        AddEntry,   SaveCore}},
  { UserArgs::Search,     {OpenCore,        Search,     SaveAfterSearch}},
  { UserArgs::Diff,       {OpenCore,        Diff,       null_op}},
  { UserArgs::Sync,       {OpenCore,        Sync,       SaveCore}},
  { UserArgs::Merge,       {OpenCore,       Merge,      SaveCore}},
};

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

bool parseArgs(int argc, char *argv[], UserArgs &ua)
{
  if (argc < 3) // must give us a safe and an operation
    return false;

  Utf82StringX(argv[1], ua.safe);

  while (1) {
    int option_index = 0;
    static constexpr struct option long_options[] = {
      // name, has_arg, flag, val
      {"import",      optional_argument,  0, 'i'},
      {"export",      optional_argument,  0, 'e'},
      {"text",        no_argument,        0, 't'},
      {"xml",         no_argument,        0, 'x'},
      {"create",      no_argument,        0, 'c'},
    //  {"new",         no_argument,        0, 'c'},
      {"search",      required_argument,  0, 's'},
      {"subset",      required_argument,  0, 'b'},
      {"fields",      required_argument,  0, 'f'},
      {"ignore-case", optional_argument,  0, 'o'},
      {"add",         required_argument,  0, 'a'},
      {"update",      required_argument,  0, 'u'},
      {"print",       optional_argument,  0, 'p'},
    //  {"remove",      no_argument,        0, 'r'},
      {"delete",      no_argument,        0, 'r'},
      {"clear",       required_argument,  0, 'l'},
      {"newpass",     no_argument,        0, 'v'},
      {"yes",         no_argument,        0, 'y'},
      {"diff",        required_argument,  0, 'd'},
      {"unified",     no_argument,        0, 'g'},
      {"context",     no_argument,        0, 'j'},
      {"sidebyside",  no_argument,        0, 'k'},
      {"dry-run",     no_argument,        0, 'n'},
      {"synchronize", required_argument,  0, 'z'},
    //  {"synch",       no_argument,        0, 'z'},
      {"merge",       required_argument,  0, 'm'},
      {"colwidth",    required_argument,  0, 'w'},
      {0, 0, 0, 0}
    };

    static_assert(no_dup_short_option(long_options), "Short option used twice");

    int c = getopt_long(argc-1, argv+1, "i::e::txcs:b:f:oa:u:pryd:gjknz:m:",
                        long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 'i':
      ua.SetMainOp(UserArgs::Import, optarg);
      break;
    case 'e':
      ua.SetMainOp(UserArgs::Export, optarg);
      break;
    case 'x':
      if (ua.Format == UserArgs::Unknown)
        ua.Format = UserArgs::XML;
      else
        return false;
      break;
    case 't':
      if (ua.Format == UserArgs::Unknown)
        ua.Format = UserArgs::Text;
      else
        return false;
      break;
    case 'c':
      ua.SetMainOp(UserArgs::CreateNew);
      break;

    case 's':
      assert(optarg);
      ua.SetMainOp(UserArgs::Search, optarg);
      break;

    case 'd':
      assert(optarg);
      ua.SetMainOp(UserArgs::Diff, optarg);
      break;

    case 'z':
      assert(optarg);
      ua.SetMainOp(UserArgs::Sync, optarg);
      break;

    case 'm':
      assert(optarg);
      ua.SetMainOp(UserArgs::Merge, optarg);
      break;

    case 'b':
        assert(optarg);
        ua.SetSubset(Utf82wstring(optarg));
        break;

    case 'f':
        assert(optarg);
        ua.SetFields(Utf82wstring(optarg));
        break;

    case 'o':
        if (optarg && std::regex_match(optarg, std::regex("yes|true", std::regex::icase)))
          ua.ignoreCase = true;
        break;

    case 'a':
        assert(optarg);
        ua.SetMainOp(UserArgs::Add, optarg);
        break;

    case 'y':
        ua.confirmed = true;
        break;

    case 'r':
        ua.SearchAction = UserArgs::Delete;
        break;

    case 'p':
        ua.SearchAction = UserArgs::Print;
        if (optarg) ua.opArg2 = Utf82wstring(optarg);
        break;

    case 'u':
        ua.SearchAction = UserArgs::Update;
        assert(optarg);
        ua.SetFieldValues(Utf82wstring(optarg));
        break;

    case 'l':
        ua.SearchAction = UserArgs::ClearFields;
        assert(optarg);
        ua.opArg2 = Utf82wstring(optarg);;
        break;

    case 'v':
        ua.SearchAction = UserArgs::ChangePassword;
        break;

      case 'g':
        ua.dfmt = UserArgs::DiffFmt::Unified;
        break;

      case 'j':
        ua.dfmt = UserArgs::DiffFmt::Context;
        break;

      case 'k':
        ua.dfmt = UserArgs::DiffFmt::SideBySide;
        break;

      case 'n':
        ua.dry_run = true;
        break;

      case 'w':
        assert(optarg);
        ua.colwidth = atoi(optarg);
        break;

    default:
      wcerr << L"Unknown option: " << static_cast<wchar_t>(c) << endl;
      return false;
    }
    if (ua.opArg.empty())
      ua.opArg = (ua.Format == UserArgs::XML) ? L"file.xml" : L"file.txt";
  }
  return true;
}


int main(int argc, char *argv[])
{
  UserArgs ua;
  if (!parseArgs(argc, argv, ua)) {
    usage(basename(argv[0]));
    return 1;
  }

  int status = 1;
  auto itr = pws_ops.find(ua.Operation);
  if (itr != pws_ops.end()) {
    PWScore core;
    try {
      status = itr->second.pre_op(core, ua.safe);
      if ( status == PWScore::SUCCESS) {
        status = itr->second.main_op(core, ua);
        if (status == PWScore::SUCCESS)
          status = itr->second.post_op(core, ua);
      }
    }
    catch(const exception &e) {
      wcerr << e.what() << endl;
      status = PWScore::FAILURE;
    }

    core.UnlockFile(ua.safe.c_str());
    return status;
  }
  wcerr << L"No main operation specified" << endl;
  return status;
}

