/**
 * Set RAPL values.
 *
 * @author Connor Imes
 * @date 2017-08-24
 */
#define _GNU_SOURCE
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "powercap-rapl-sysfs.h"
#include "util-common.h"

static const char short_options[] = "hp:z:c:e:l:s:";
static const struct option long_options[] = {
  {"help",                no_argument,        NULL, 'h'},
  {"package",             required_argument,  NULL, 'p'},
  {"subzone",             required_argument,  NULL, 'z'},
  {"constraint",          required_argument,  NULL, 'c'},
  {"z-enabled",           required_argument,  NULL, 'e'},
  {"c-power-limit",       required_argument,  NULL, 'l'},
  {"c-time-window",       required_argument,  NULL, 's'},
};

static void print_usage(void) {
  printf("Usage: rapl-set [OPTION]...\n");
  printf("Options:\n");
  printf("  -h, --help                   Print this message and exit\n");
  printf("  -p, --package=PACKAGE        The package number to use (0 by default)\n");
  printf("  -z, --subzone=SUBZONE        The subzone number to use (none by default)\n");
  printf("  -c, --constraint=CONSTRAINT  The constraint number to use (none by default)\n");
  printf("The following is a zone-level argument. You may also specify -z/--subzone for a subzone:\n");
  printf("  -e, --z-enabled=1|0          Enable/disable a zone\n");
  printf("The following constraint-level arguments may be used together. The -c/--constraint option is required. You may also specify -z/--subzone for a subzone:\n");
  printf("  -l, --c-power-limit=UW       Set constraint power limit\n");
  printf("  -s, --c-time-window=US       Set constraint time window\n");
  printf("\nSubzones are a package's child domains, including power planes.\n");
  printf("\nPower units: microwatts (uW)\n");
  printf("Time units: microseconds (us)\n");
}

static void print_common_help(void) {
  printf("Considerations for common errors:\n");
  printf("- Ensure that you run with administrative (super-user) privileges\n");
  printf("- Ensure that the intel_rapl kernel module loaded.\n");
}

int main(int argc, char** argv) {
  u32_param package = {0, 0};
  u32_param subzone = {0, 0};
  u32_param constraint = {0, 0};
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
      ret = set_u32_param(&package, optarg, &cont);
      break;
    case 'z':
      ret = set_u32_param(&subzone, optarg, &cont);
      break;
    case 'c':
      ret = set_u32_param(&constraint, optarg, &cont);
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
  } else if (constraint.set && !(power_limit.set || time_window.set)) {
    fprintf(stderr, "Must set at least one constraint-level argument when using -c/--constraint\n");
    ret = -EINVAL;
  } else if (!constraint.set && (power_limit.set || time_window.set)) {
    fprintf(stderr, "Must specify -c/--constraint when using constraint-level arguments\n");
    ret = -EINVAL;
  } else if (!(enabled.set || power_limit.set || time_window.set)) {
    printf("Nothing to do\n");
    ret = -EINVAL;
  }
  if (ret) {
    print_usage();
    return ret;
  }

  /* Perform requested action(s) */
  if (enabled.set) {
    c = rapl_sysfs_zone_set_enabled(package.val, subzone.val, subzone.set, enabled.val);
    if (c) {
      perror("Error setting enabled/disabled");
      ret |= c;
    }
  }
  if (power_limit.set) {
    c = rapl_sysfs_constraint_set_power_limit_uw(package.val, subzone.val, subzone.set, constraint.val, power_limit.val);
    if (c) {
      perror("Error setting power limit");
      ret |= c;
    }
  }
  if (time_window.set) {
    c = rapl_sysfs_constraint_set_time_window_us(package.val, subzone.val, subzone.set, constraint.val, time_window.val);
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