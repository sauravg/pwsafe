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
  wstring search_action;

public:
  cli_search(): cli_task{'s', "search", required_argument}{ fields.set(); }
  static string_vec long_help();
  static wstring short_help();
  virtual int execute(PWScore &core)              override;
  virtual bool handle_arg( const char *name, const char *value) override;
};

#endif /* defined(__pwsafe_xcode6__search__) */
