#pragma once

#include "../../core/StringX.h"
#include "./argutils.h"

class PWScore;
struct PWPolicy;

int OpenCore(PWScore& core, const StringX& safe);
int SaveCore(PWScore &core, const UserArgs &);
int SaveCore(PWScore &core, bool dry_run);

StringX GetNewPassphrase();

int CreateNewSafe(PWScore &core, const StringX& filename);

int AddEntry(PWScore &core, const UserArgs &ua);
int AddEntryWithFields(PWScore &core, const UserArgs::FieldUpdates &fieldValues,
                       std::wostream &errstream);

int InitPWPolicy(PWPolicy &pwp, PWScore &core);

