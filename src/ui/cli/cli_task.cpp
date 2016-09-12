#include "./cli_task.h"

#include <getopt.h>

void cli_task::parse_args(int argc, char *argv[]) throw(std::invalid_argument)
{
  struct option long_options[] = {
    {"yes",         no_argument,        0, 'y'},
    {"dry-run",     no_argument,        0, 'n'},
    {str_op,        atype,              0, ch_op}
  };
  string opstr{"yn"};
  opstr += ch_op;
  switch (atype) {
    case required_argument:
      opstr += ':'; break;
    case optional_argument:
      opstr += "::"; break;
    default:
      break;
  }
  int option_index;
  int c = getopt_long(argc, argv, opstr.c_str(), long_options, &option_index);
  switch( c ) {
    case 'n': dry_run = true; break;
    case 'y': confirmed = true; break;
    case -1: break;
    default:
      if ( !handle_arg(long_options[option_index].name, optarg) )
        throw std::invalid_argument(argv[optind - 1]);
  }
}
