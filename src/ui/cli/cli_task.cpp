#include "./cli_task.h"

#include <getopt.h>

void cli_task::parse_args(int argc, char *argv[]) throw(std::invalid_argument)
{
 // optind = 1;

  vector<option> topts = task_options();

  topts.insert( topts.end(), {  {"yes",         no_argument,        0, 'y'},
                                {"dry-run",     no_argument,        0, 'n'},
                                {0,             0,                  0,  0}});

  string opstr;
  for (const auto o: topts) opstr += o.val;

  opstr += ch_op;
  switch (atype) {
    case required_argument:
      opstr += ':'; break;
    case optional_argument:
      opstr += "::"; break;
    default:
      break;
  }

  for ( int argidx = 0; argidx < argc; argidx++) {
      int option_index;
      int c = getopt_long(argc, argv, opstr.c_str(), &topts[0], &option_index);
      switch( c ) {
        case 'n': dry_run = true; break;
        case 'y': confirmed = true; break;
        case -1:
            throw std::invalid_argument(argv[optind - 1]);
        case '?':
            break; // getopt will print a message
        default:
          if ( !handle_arg(topts[option_index].name, optarg) )
            throw std::logic_error("Incorrect command-line argument processing");
      }
  }
}
