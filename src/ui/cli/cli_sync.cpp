#include "cli_sync.h"

#include "./argutils.h"
#include "./safeutils.h"
#include "../../core/PWScore.h"

int Sync(PWScore &core, const StringX &otherSafe,
              const Restriction &subset, const CItemData::FieldBits &fields,
              int &numUpdated)
{
  PWScore otherCore;
  int status = OpenCore(otherCore, otherSafe);
  if ( status == PWScore::SUCCESS ) {
    CReport rpt;
    core.Synchronize(&otherCore,
                      fields,          // fields to sync
                      subset.valid(),  // filter?
                      subset.value,    // filter value
                      subset.field,    // field to filter by
                      subset.rule,     // type of match rule for filtering
                      numUpdated,
                      &rpt,               // Must be non-null
                      NULL                // Cancel mechanism. We don't need one
    );
    otherCore.UnlockFile(otherSafe.c_str());
  }
  return status;
}
int Sync(PWScore &core, const UserArgs &ua)
{
  int numUpdated = 0;
  return Sync(core, std2stringx(ua.opArg), ua.subset, ua.fields, numUpdated);
}

bool cli_sync::handle_arg(const char *name, const char *value)
{
  if (strcmp(name, "subset") == 0 && value != nullptr && value[0]) {
    subset = ParseSubset(Utf82wstring(value));
    return true;
  }
  else if (strcmp(name, "fields") == 0 && value != nullptr && value[0]) {
    fields = ParseFields(Utf82wstring(value));
    return true;
  }
  return false;
}

//static
string_vec cli_sync::long_help()
{
  string_vec v = { short_help() };
  v.insert(v.end(), restrictions_help.begin(), restrictions_help.end());
  const string_vec fields_help{get_fields_help()};
  v.insert(v.end(), fields_help.begin(), fields_help.end());
  return v;
}

//  virtual
int cli_sync::execute(PWScore &core, const string &op_param)
{
  int numUpdated = 0;
  int status = Sync(core, str2StringX(op_param), subset, fields, numUpdated);
  assert( core.IsChanged() == (numUpdated > 0) );
  if (numUpdated) dirty = true;
  return status;
}
