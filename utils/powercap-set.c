/**
 * Set powercap values.
 *
 * @author Connor Imes
 * @date 2017-08-24
 */
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "powercap-sysfs.h"
#include "util-common.h"

static const char short_options[] = "hp:z:c:je:l:s:";
static const struct option long_options[] = {
  {"help",                no_argument,        NULL, 'h'},
  {"package",             required_argument,  NULL, 'p'},
  {"control-type",        required_argument,  NULL, 'p'},
  {"zone",                required_argument,  NULL, 'z'},
  {"constraint",          required_argument,  NULL, 'c'},
  {"z-energy",            no_argument      ,  NULL, 'j'},
  {"z-enabled",           required_argument,  NULL, 'e'},
  {"c-power-limit",       required_argument,  NULL, 'l'},
  {"c-time-window",       required_argument,  NULL, 's'},
  {0, 0, 0, 0}
};

static void print_usage(void) {
  printf("Usage: powercap-set -p NAME -z ZONE(S) [OPTION]...\n");
  printf("Options:\n");
  printf("  -h, --help                   Print this message and exit\n");
  printf("  -p, --control-type=NAME      [REQUIRED] The powercap control type name\n");
  printf("                               Must not be empty or contain a '.' or '/'\n");
  printf("  -z, --zone=ZONE(S)           [REQUIRED] The zone/subzone numbers in the\n");
  printf("                               control type's powercap tree\n");
  printf("                               Separate zones/subzones with a colon\n");
  printf("                               E.g., for zone 0, subzone 2: \"-z 0:2\"\n");
  printf("  -c, --constraint=CONSTRAINT  The constraint number (none by default)\n");
  printf("The following zone-level arguments may be used together:\n");
  printf("  -j, --z-energy               Reset zone energy counter\n");
  printf("  -e, --z-enabled=1|0          Enable/disable a zone\n");
  printf("The following constraint-level arguments may be used together and require -c/--constraint:\n");
  printf("  -l, --c-power-limit=UW       Set constraint power limit\n");
  printf("  -s, --c-time-window=US       Set constraint time window\n");
  printf("\nPower units: microwatts (uW)\n");
  printf("Time units: microseconds (us)\n");
}

static void print_common_help(void) {
  printf("Considerations for common errors:\n");
  printf("- Ensure that the control type exists (may require loading a kernel module, e.g., intel_rapl)\n");
  printf("- Ensure that you run with administrative (super-user) privileges\n");
}

int main(int argc, char** argv) {
  const char* control_type = NULL;
  uint32_t zones[MAX_ZONE_DEPTH] = { 0 };
  uint32_t depth = 0;
  u32_param constraint = {0, 0};
  int reset_energy = 0;
  u32_param enabled = {0, 0};
  u64_param power_limit = {0, 0};
  u64_param time_window = {0, 0};
  int c;
  int cont = 1;
  int ret = 0;

  /* Parse command-line arguments */
  while (cont) {
    c = getopt_long(argc, argv, short_options, long_options, NULL);
    switch (c) {
    case -1:
      cont = 0;
      break;
    case 'h':
      print_usage();
      return 0;
    case 'p':
      if (control_type) {
        cont = 0;
        ret = -EINVAL;
      }
      control_type = optarg;
      break;
    case 'z':
      ret = parse_zones(optarg, zones, MAX_ZONE_DEPTH, &depth, &cont);
      break;
    case 'c':
      ret = set_u32_param(&constraint, optarg, &cont);
      break;
    case 'j':
      if (reset_energy) {
        cont = 0;
        ret = -EINVAL;
      }
      reset_energy = 1;
      break;
    case 'e':
      ret = set_u32_param(&enabled, optarg, &cont);
      break;
    case 'l':
      ret = set_u64_param(&power_limit, optarg, &cont);
      break;
    case 's':
      ret = set_u64_param(&time_window, optarg, &cont);
      break;
    case '?':
    default:
      cont = 0;
      ret = -EINVAL;
      break;
    }
  }

  /* Verify argument combinations */
  if (ret) {
    fprintf(stderr, "Invalid arguments\n");
  } else if (!is_valid_control_type(control_type)) {
    fprintf(stderr, "Must specify -p/--control-type; value must not be empty or contain any '.' or '/' characters\n");
    ret = -EINVAL;
  } else if (!depth) {
    fprintf(stderr, "Must specify -z/--zone\n");
    ret = -EINVAL;
  } else if (constraint.set && !(power_limit.set || time_window.set)) {
    fprintf(stderr, "Must set at least one constraint-level argument when using -c/--constraint\n");
    ret = -EINVAL;
  } else if (!constraint.set) {
    if (power_limit.set || time_window.set) {
      fprintf(stderr, "Must specify -c/--constraint when using constraint-level arguments\n");
      ret = -EINVAL;
    } else if (!(reset_energy || enabled.set)) {
      fprintf(stderr, "Nothing to do\n");
      ret = -EINVAL;
    }
  }
  if (ret) {
    print_usage();
    return ret;
  }

  /* Check if control type/zones/constraint exist */
  if (powercap_sysfs_control_type_exists(control_type)) {
    fprintf(stderr, "Control type does not exist\n");
    ret = -EINVAL;
  } else if (depth && powercap_sysfs_zone_exists(control_type, zones, depth)) {
    fprintf(stderr, "Zone does not exist\n");
    ret = -EINVAL;
  } else if (constraint.set && powercap_sysfs_constraint_exists(control_type, zones, depth, constraint.val)) {
    fprintf(stderr, "Constraint does not exist\n");
    ret = -EINVAL;
  }
  if (ret) {
    print_common_help();
    return ret;
  }

  /* Perform requested action(s) */
  if (reset_energy) {
    c = powercap_sysfs_zone_reset_energy_uj(control_type, zones, depth);
    if (c) {
      perror("Error setting energy counter");
      ret |= c;
    }
  }
  if (enabled.set) {
    c = powercap_sysfs_zone_set_enabled(control_type, zones, depth, enabled.val);
    if (c) {
      perror("Error setting enabled/disabled");
      ret |= c;
    }
  }
  if (power_limit.set) {
    c = powercap_sysfs_constraint_set_power_limit_uw(control_type, zones, depth, constraint.val, power_limit.val);
    if (c) {
      perror("Error setting power limit");
      ret |= c;
    }
  }
  if (time_window.set) {
    c = powercap_sysfs_constraint_set_time_window_us(control_type, zones, depth, constraint.val, time_window.val);
    if (c) {
      perror("Error setting time window");
      ret |= c;
    }
  }
  if (ret) {
    print_common_help();
  }

  return ret;
}
