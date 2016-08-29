#pragma once

#include "./cli_task.h"

struct UserArgs;
class PWScore;

int Diff(PWScore &core, const UserArgs &ua);

class cli_diff: public cli_task
{
  cli_diff(int argc, char *argv[]);
public:
  virtual bool is_dirty() const                   override {return false; }
  virtual int execute(PWScore &core)              override;
};

