#pragma once

#include "./cli_task.h"

class UserArgs;
int Merge(PWScore &core, const UserArgs &ua);

class cli_merge: public cli_task
{
  cli_merge(int argc, char *argv[]);
public:
  virtual bool is_dirty() const                   override;
  virtual int execute(PWScore &core)              override;
};

