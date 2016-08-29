#pragma once

#include "./cli_task.h"

class UserArgs;
int Sync(PWScore &core, const UserArgs &ua);

class cli_sync: public cli_task
{
  cli_sync(int argc, char *argv[]);
public:
  virtual bool is_dirty() const                   override;
  virtual int execute(PWScore &core)              override;
};

