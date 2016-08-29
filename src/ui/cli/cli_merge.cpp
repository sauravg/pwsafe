#include "./cli_merge.h"
#include "./argutils.h"
#include "./safeutils.h"

#include "../../core/StringX.h"

int Merge(PWScore &core, const UserArgs &ua)
{
  const StringX otherSafe{std2stringx(ua.opArg)};
  PWScore otherCore;
  int status = OpenCore(otherCore, otherSafe);
  if ( status == PWScore::SUCCESS ) {
    CReport rpt;
    core.Merge(&otherCore,
               ua.subset.valid(),  // filter?
               ua.subset.value,    // filter value
               ua.subset.field,    // field to filter by
               ua.subset.rule,     // type of match rule for filtering
               &rpt,               // Must be non-null
               NULL                // Cancel mechanism. We don't need one
    );
    otherCore.UnlockFile(otherSafe.c_str());
  }
  return status;
}
