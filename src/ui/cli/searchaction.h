//
//  searchaction.h
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 19/06/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#ifndef __pwsafe_xcode6__searchaction__
#define __pwsafe_xcode6__searchaction__

#include <vector>

#include "../../core/PWScore.h"
#include "./argutils.h"

using std::wstring;

using ItemPtrVec = std::vector<const CItemData *>;
using FieldUpdates = UserArgs::FieldUpdates ;

int PrintSearchResults(const ItemPtrVec &items, PWScore &core, const CItemData::FieldBits &ftp, std::wostream &os);
int DeleteSearchResults(const ItemPtrVec &items, PWScore &core);
int UpdateSearchResults(const ItemPtrVec &items, PWScore &core, const FieldUpdates &updates);
int ClearFieldsOfSearchResults(const ItemPtrVec &items, PWScore &core, const CItemData::FieldBits &ftp);
int ChangePasswordOfSearchResults(const ItemPtrVec &items, PWScore &core);

template <int action>
struct SearchActionTraits
{};

template <>
struct SearchActionTraits<UserArgs::Print>
{
};

template <>
struct SearchActionTraits<UserArgs::Delete>
{
  static constexpr wchar_t prompt[] = L"Delete Item";
};

template <>
struct SearchActionTraits<UserArgs::Update>
{
  static constexpr wchar_t prompt[] = L"Update Item";
};

template <>
struct SearchActionTraits<UserArgs::ClearFields>
{
  static constexpr wchar_t prompt[] = L"Clear files of item";
};

template <>
struct SearchActionTraits<UserArgs::ChangePassword>
{
  static constexpr wchar_t prompt[] = L"Change password of item";
};

// This hack lets me avoid converting between string literals at runtime
#define DECLARE_STR(name, val) static constexpr const char *name = val; static constexpr const wchar_t *name##w = L##val;

struct Print {
  static constexpr bool needs_confirmation{false};
  static bool Parse(const wstring &arg){return arg.empty();}
  DECLARE_STR(help, "--print=field1,field2,...")
  static constexpr const wchar_t *long_arg= L"print";
  static int execute(const wstring &arg, PWScore &core, const ItemPtrVec &matches);
  static constexpr const wchar_t *prompt = L"Should never be used";
};

struct Delete {
  static constexpr bool needs_confirmation{true};
  static bool Parse(const wstring &arg){return arg.empty();}
  DECLARE_STR(help, "--delete")
  static constexpr const wchar_t *long_arg= L"delete";
  static int execute(const wstring &arg, PWScore &core, const ItemPtrVec &matches);
  static constexpr const wchar_t *prompt = L"Delete Item";
};

struct Update {
  static constexpr bool needs_confirmation{true};
  static bool Parse(const wstring &values) {
    if (values.empty()) return false;
    ParseFieldValues(values);
    return true;
  }
  DECLARE_STR(help, "--update=field1=value1,field2=value2,...")
  static constexpr const wchar_t *long_arg= L"update";
  static int execute(const wstring &arg, PWScore &core, const ItemPtrVec &matches);
  static constexpr const wchar_t *prompt = L"Update Item";
};

struct ClearFields {
  static constexpr bool needs_confirmation{true};
  static bool Parse(const wstring &fs) {
    if (fs.empty()) return false;
    ParseFields(fs);
    return true;
  }
  DECLARE_STR(help, "--clear=field1,field2,...")
  static constexpr const wchar_t *long_arg= L"clear";
  static int execute(const wstring &arg, PWScore &core, const ItemPtrVec &matches);
  static constexpr const wchar_t *prompt = L"Clear files of item";
};

struct ChangePassword {
  static constexpr bool needs_confirmation{true};
  static bool Parse(const wstring &args){ return args.empty();}
  DECLARE_STR(help, "--newpass")
  static constexpr const wchar_t *long_arg= L"newpass";
  static int execute(const wstring &arg, PWScore &core, const ItemPtrVec &matches);
  static constexpr const wchar_t *prompt = L"Change password of item";
};


#endif /* defined(__pwsafe_xcode6__searchaction__) */
