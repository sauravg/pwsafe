//
//  search.h
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 19/06/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#ifndef __pwsafe_xcode6__search__
#define __pwsafe_xcode6__search__

#include "./cli_task.h"
#include "./argutils.h"
#include "../../core/PWScore.h"

int Search(PWScore &core, const UserArgs &ua);
int SaveAfterSearch(PWScore &core, const UserArgs &ua);

class cli_search: public cli_task
{
  Restriction subset;
  CItemData::FieldBits fields;
  bool ignore_case{false};
  wstring action, actionParam;

public:
  cli_search(): cli_task{'s', "search", required_argument}{ fields.set(); }
  static string_vec long_help();
  static wstring short_help();
  virtual int execute(PWScore &core, const string &op_param)              override;
  virtual bool handle_arg( const char *name, const char *value) override;
  template <class Op, class... Rest>
  friend int execute_search_op(const cli_search &search, const string &text, PWScore &core, std::tuple<Op, Rest...>);
  friend int save_core<cli_search>(PWScore &core, const cli_search &s);
  virtual vector<option> task_options() const override;
};

template <>
int save_core<cli_search>(PWScore &core, const cli_search &s);


#endif /* defined(__pwsafe_xcode6__search__) */
