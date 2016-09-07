#include "./cli_merge.h"
#include "./argutils.h"
#include "./safeutils.h"

#include "../../core/StringX.h"

int Merge(PWScore &core, const StringX& otherSafe, const Restriction &subset)
{
  PWScore otherCore;
  int status = OpenCore(otherCore, otherSafe);
  if ( status == PWScore::SUCCESS ) {
    CReport rpt;
    core.Merge(&otherCore,
               subset.valid(),  // filter?
               subset.value,    // filter value
               subset.field,    // field to filter by
               subset.rule,     // type of match rule for filtering
               &rpt,               // Must be non-null
               NULL                // Cancel mechanism. We don't need one
    );
    otherCore.UnlockFile(otherSafe.c_str());
  }
  return status;
}

int Merge(PWScore &core, const UserArgs &ua)
{
  return Merge(core, std2stringx(ua.opArg), ua.subset);
}

bool cli_merge::handle_arg(const char *name, const char *value)
{
  if (strcmp(name, "subset") == 0 && value != nullptr && value[0]) {
    subset = ParseSubset(Utf82wstring(value));
    return true;
  }
  return false;
}

//static
string_vec cli_merge::long_help()
{
  string_vec sh =  {  short_help() };
  sh.insert(sh.end(), restrictions_help.begin(), restrictions_help.end());
  return sh;
}

// override
int cli_merge::execute(PWScore &core)
{
  int status = Merge(core, str2StringX(op_param), subset);
  if ( status == PWScore::SUCCESS )
    dirty = core.IsChanged();
  return status;
}
