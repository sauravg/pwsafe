//
//  search.cpp
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 19/06/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#include "./search.h"
#include "./argutils.h"
#include "./strutils.h"
#include "./searchaction.h"
#include "./search-internal.h"

#include <vector>
#include <exception>

#include "../../core/Util.h"
#include "../../core/PWScore.h"

#include <assert.h>
#include <type_traits>

#include "../wxWidgets/SearchUtils.h"

using namespace std;


using CbType = function<void(const pws_os::CUUID &, const CItemData &, bool *)>;
void SearchForEntries(PWScore &core, const wstring &searchText, bool ignoreCase,
                      const Restriction &r, const CItemData::FieldBits &fieldsToSearch,
                      CbType cb)
{
  assert( !searchText.empty() );
  
  CItemData::FieldBits fields = fieldsToSearch;
  if (fields.none())
    fields.set();

  ::FindMatches(std2stringx(searchText), ignoreCase, fields, r.valid(), r.value, r.field, r.rule, r.caseSensitive,
                core.GetEntryIter(), core.GetEntryEndIter(), get_second<ItemList>{},
                  [&cb](ItemListIter itr, bool *keep_going){
                  cb(itr->first, itr->second, keep_going);
                });
}

int SaveAfterSearch(PWScore &core, const UserArgs &ua)
{
  switch (ua.SearchAction) {
    case UserArgs::Update:
    case UserArgs::Delete:
    case UserArgs::ClearFields:
    case UserArgs::ChangePassword:
      if ( core.IsChanged() ) return core.WriteCurFile();
      break;
    case UserArgs::Print:
      break;
  }
  return PWScore::SUCCESS;
}


wchar_t Confirm(const wstring &prompt, const wstring &ops,
            const wstring &help, const CItemData &item)
{
  wstring options{ops};
  options += L"p?";
  wchar_t choice{};
  do {
    wcout << st_GroupTitleUser{item.GetGroup(), item.GetTitle(), item.GetUser()} << endl;
    wcout << prompt << L" [" << options << L"]? ";
    wcin >> choice;
    switch( choice ) {
      case L'p':
      {
        auto fields = {CItem::GROUP, CItem::TITLE, CItem::USER,
                       CItem::EMAIL, CItem::URL, CItem::AUTOTYPE};
        for(auto f: fields)
          wcout << item.FieldName(f) << L": " << item.GetFieldValue(f) << endl;
        choice = 0;
        break;
      }
      case L'?':
        wcout << help << L"[p]rint - print all fields for this item" << endl
                      << L"[?]     - print this help message" << endl;
        choice = 0;
        break;
      default:
        if (ops.find(choice) != wstring::npos) return choice;
        wcerr << L"Huh (" << choice << L")?" << endl;
        choice = 0;
        break;
    }
  } while( !choice );
  return choice;
}

template <typename action_func_t>
int SearchAndConfirm(const wstring &prompt, PWScore &core, const wstring &text,
                     bool ignoreCase, const Restriction &subset, const CItemData::FieldBits &fields,
                     bool confirmed, action_func_t afn)
{
  ItemPtrVec matches;
  auto matchfn = [&matches](const pws_os::CUUID &/*uuid*/, const CItemData &data) {
    matches.push_back(&data);
  };

  const wchar_t help[] = L"[y]es   - yes for this item\n"
  "[n]o    - no for this item\n"
  "[a]ll   - yes for this item and all remaining items\n"
  "[q]uit  - no for this item all remaining items\n"
  "a[b]ort - abort operation, even for previous items\n";

  wchar_t choice{ confirmed? L'a': 0 };

  SearchForEntries(core, text, ignoreCase, subset, fields,
                   [matchfn, &choice, help, &prompt](const pws_os::CUUID &uuid,
                                                     const CItemData &data,
                                                     bool *keep_going) {

                     if( choice != L'a' )
                       choice = Confirm(prompt, L"ynaqb", help, data);

                     switch(choice) {
                       case L'y': case L'a':
                         matchfn(uuid, data);
                         break;
                       case L'n':
                         break;
                       case L'q': case L'b':
                         *keep_going = false;
                         break;
                       default:
                         assert(false);
                         break;
                     }

                   });

  if (choice != L'b')
    return afn(matches);

  return PWScore::SUCCESS;
}

template <typename action_func_t>
int SearchAndConfirm(const wstring &prompt, PWScore &core, const UserArgs &ua, action_func_t afn)
{
  return SearchAndConfirm(prompt, core, ua.opArg, ua.ignoreCase, ua.subset, ua.fields, ua.confirmed, afn);
}

template <int action, typename action_func_t>
struct SearchWithConfirmation
{
  int operator()(PWScore &core, const UserArgs &ua, action_func_t afn)
  {
    return SearchAndConfirm(SearchActionTraits<action>::prompt, core, ua, afn);
  }
};

template <typename action_func_t>
struct SearchWithoutConfirmation
{
  int operator()(PWScore &core, const UserArgs &ua, action_func_t afn) const
  {
    ItemPtrVec matches;
    SearchForEntries(core, ua.opArg, ua.ignoreCase, ua.subset, ua.fields,
                     [&matches](const pws_os::CUUID &/*uuid*/,
                                 const CItemData &data,
                                 bool */*keep_going*/) {
      matches.push_back(&data);
    });
    return afn(matches);
  }
};

template <typename T>
struct has_prompt_t {
  typedef char yes_type;
  typedef long no_type;

  template <typename C> static yes_type test( decltype(&C::prompt) );
  template <typename C> static no_type test(...);

  enum { value = (sizeof(decltype(test<T>(0))) == sizeof(yes_type))};
};

template <typename T>
constexpr bool has_prompt()
{
  return has_prompt_t<T>::value;
}

template <int action, typename action_func_t>
int DoSearch(PWScore &core, const UserArgs &ua, action_func_t afn)
{
  using Traits = SearchActionTraits<action>;
  using WithConfirm = SearchWithConfirmation<action, action_func_t>;
  using WithoutConfirm = SearchWithoutConfirmation<action_func_t>;

  using SearchFunction = typename conditional< has_prompt<Traits>(), WithConfirm, WithoutConfirm>::type;

  SearchFunction f{};
  return f(core, ua, afn);
}

int Search(PWScore &core, const UserArgs &ua)
{
  return SearchInternal(core, ua, wcout);
}

// Any new search operation must be added to this list
using SearchActions = std::tuple<Print, Delete, Update, ClearFields, ChangePassword>;

int SearchInternal(PWScore &core, const UserArgs &ua, wostream &os)
{
  switch( ua.SearchAction) {

    case UserArgs::Print:
    {
      CItemData::FieldBits ftp = ParseFields(ua.opArg2);
      return DoSearch<UserArgs::Print>(core, ua, [&core, &ftp, &os](const ItemPtrVec &matches) {
        return PrintSearchResults(matches, core, ftp, os);
      });
    }

    case UserArgs::Delete:
      return DoSearch<UserArgs::Delete>(core, ua, [&core](const ItemPtrVec &matches) {
        return DeleteSearchResults(matches, core);
      });

    case UserArgs::Update:
      return DoSearch<UserArgs::Update>(core, ua, [&core, &ua](const ItemPtrVec &matches) {
        return UpdateSearchResults(matches, core, ua.fieldValues);
      });

    case UserArgs::ClearFields:
    {
      CItemData::FieldBits ftp = ParseFields(ua.opArg2);
      return DoSearch<UserArgs::ClearFields>(core, ua, [&core, &ua, &ftp](const ItemPtrVec &matches) {
        return ClearFieldsOfSearchResults(matches, core, ftp);
      });
    }

    case UserArgs::ChangePassword:
      return DoSearch<UserArgs::ChangePassword>(core, ua, [&core, &ua](const ItemPtrVec &matches) {
        return ChangePasswordOfSearchResults(matches, core);
      });

    default:
      assert(false);
      return PWScore::FAILURE;
  }
}

string_vec make_long_help(tuple<>) {
  return string_vec{};
}

template <class Op, class... Rest>
string_vec make_long_help( tuple<Op, Rest...>)
{
  string_vec v = make_long_help(tuple<Rest...>{});
  v.push_back(Op::helpw);
  return v;
}

//  static
string_vec cli_search::long_help()
{
  return make_long_help(SearchActions{});
}

//  static
wstring cli_search::short_help()
{
    return L"--search=<text> [--ignore-case]"
           L" [--subset=<Field><OP><string>[/iI] [--fields=f1,f2,..]"
           L" [<operation-on-matched-entries>]";
}

inline int execute_search_op(const cli_search &search, const string &text, PWScore &core, tuple<>)
{
  return PWScore::FAILURE;
}

template <class Op, class... Rest>
int execute_search_op(const cli_search &search, const string &text, PWScore &core, tuple<Op, Rest...>)
{
  if ( search.action == Op::long_arg ) {
    ItemPtrVec vec;
    if (Op::needs_confirmation && !search.confirmed) {
      SearchAndConfirm(Op::prompt, core, str2wstr(text), search.ignore_case, search.subset,
                       search.fields, false, [&vec](const ItemPtrVec &matches){
                          vec = matches;
                          return PWScore::SUCCESS;
      });
    }
    else {
      SearchForEntries(core, str2wstr(text), search.ignore_case, search.subset, search.fields,
                       [&vec](const pws_os::CUUID &/*uuid*/, const CItemData &data, bool */*keep_going*/) {
                         vec.push_back(&data);
                       });
    }
    if (!vec.empty())
      return Op::execute(search.actionParam, core, vec);
    return PWScore::SUCCESS;
  }
  return execute_search_op(search, text, core, tuple<Rest...>{});
}

inline bool is_search_action(const wstring &/*name*/, const wstring& /*value*/, tuple<>)
{
  return false;
}

template <class Op, class... Rest>
inline bool is_search_action(const wstring &name, const wstring &value, tuple<Op, Rest...>)
{
  if ( Op::long_arg == name) {
    if (Op::Parse(value)) return true;
    throw new std::invalid_argument{Op::help};
  }
  return is_search_action(name, value, tuple<Rest...>{});
}

vector<option> cli_search::task_options() const
{
    return {
        {"subset",      required_argument,  0, 'b'},
        {"fields",      required_argument,  0, 'f'},
        {"ignore-case", no_argument,        0, 'i'},
        Print::long_opt,
        Delete::long_opt,
        Update::long_opt,
        ChangePassword::long_opt,
        ClearFields::long_opt
    };
}

//  virtual
bool cli_search::handle_arg( const char *name, const char *value)
{
  if ( strcmp("subset", name) == 0 ) {
    subset = ParseSubset(str2wstr(value));
    return true;
  }
  else if ( strcmp("fields", name) == 0 ) {
    fields = ParseFields(str2wstr(value));
    return true;
  }
  else if ( strcmp("ignore-case", name) == 0 ) {
    ignore_case = true;
    return true;
  }
  else {
    const wstring wname{str2wstr(name)}, wvalue{str2wstr(value)};
    if (is_search_action(wname, wvalue, SearchActions{})) {
      action = wname;
      actionParam = wvalue;
      return true;
    }
  }
  return false;
}

//  virtual
int cli_search::execute(PWScore &core, const string &op_param)
{
  if (action.empty()) action = L"print";
  return execute_search_op(*this, op_param, core, SearchActions{});
}

inline int save_core(PWScore &core, const wstring &action, bool dry_run, tuple<>)
{
  throw std::logic_error(string("no save implemented for action: ") + toutf8(action));
}

template <class Op, class... Rest>
inline int save_core(PWScore &core, const wstring &action, bool dry_run, tuple<Op, Rest...>)
{
  if ( Op::long_arg == action) {
    return save_core<Op>(core, dry_run);
  }
  return save_core(core, action, dry_run, tuple<Rest...>{});
}


template <>
int save_core<cli_search>(PWScore &core, const cli_search &s)
{
  return save_core(core, s.action, s.dry_run, SearchActions{});
}
