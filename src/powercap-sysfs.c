/**
 * Read/write powercap sysfs files.
 *
 * @author Connor Imes
 * @date 2017-08-24
 */
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
/* Main powercap header only used for enums, not functions! */
#include "powercap.h"
#include "powercap-common.h"
#include "powercap-sysfs.h"

static int zone_read_u64(const char* control_type, const uint32_t* zones, uint32_t depth, uint64_t* val,
                         powercap_zone_file type) {
  int ret;
  int fd;
  if ((fd = open_zone_file(control_type, zones, depth, type, O_RDONLY)) < 0) {
    return -errno;
  }
  ret = read_u64(fd, val);
  close(fd);
  return ret;
}

static int constraint_read_u64(const char* control_type, const uint32_t* zones, uint32_t depth, uint32_t constraint,
                               uint64_t* val, powercap_constraint_file type) {
  int ret;
  int fd;
  if ((fd = open_constraint_file(control_type, zones, depth, constraint, type, O_RDONLY)) < 0) {
    return -errno;
  }
  ret = read_u64(fd, val);
  close(fd);
  return ret;
}

static int constraint_write_u64(const char* control_type, const uint32_t* zones, uint32_t depth, uint32_t constraint,
                                uint64_t val, powercap_constraint_file type) {
  int ret;
  int fd;
  if ((fd = open_constraint_file(control_type, zones, depth, constraint, type, O_WRONLY)) < 0) {
    return -errno;
  }
  ret = write_u64(fd, val);
  close(fd);
  return ret;
}

int powercap_sysfs_control_type_exists(const char* control_type) {
  return powercap_sysfs_zone_exists(control_type, NULL, 0);
}

int powercap_sysfs_zone_exists(const char* control_type, const uint32_t* zones, uint32_t depth) {
  char path[PATH_MAX];
  struct stat ss;
  if (!get_base_path(control_type, zones, depth, path, sizeof(path))) {
    return -errno;
  }
  if (stat(path, &ss) || !S_ISDIR(ss.st_mode)) {
    errno = ENOSYS;
    return -errno;
  }
  return 0;
}

int powercap_sysfs_constraint_exists(const char* control_type, const uint32_t* zones, uint32_t depth, uint32_t constraint) {
  char path[PATH_MAX];
  struct stat ss;
  /* power_limit_uw file must exist */
  if (!get_constraint_file_path(control_type, zones, depth, constraint, POWERCAP_CONSTRAINT_FILE_POWER_LIMIT_UW, path, sizeof(path))) {
    return -errno;
  }
  if (stat(path, &ss) || !S_ISREG(ss.st_mode)) {
    errno = ENOSYS;
    return -errno;
  }
  return 0;
}

int powercap_sysfs_zone_get_max_energy_range_uj(const char* control_type, const uint32_t* zones, uint32_t depth, uint64_t* val) {
  return zone_read_u64(control_type, zones, depth, val, POWERCAP_ZONE_FILE_MAX_ENERGY_RANGE_UJ);
}

int powercap_sysfs_zone_reset_energy_uj(const char* control_type, const uint32_t* zones, uint32_t depth) {
  int ret;
  int fd;
  if ((fd = open_zone_file(control_type, zones, depth, POWERCAP_ZONE_FILE_ENERGY_UJ, O_WRONLY)) < 0) {
    return -errno;
  }
  ret = write_u64(fd, 0);
  close(fd);
  return ret;
}

int powercap_sysfs_zone_get_energy_uj(const char* control_type, const uint32_t* zones, uint32_t depth, uint64_t* val) {
  return zone_read_u64(control_type, zones, depth, val, POWERCAP_ZONE_FILE_ENERGY_UJ);
}

int powercap_sysfs_zone_get_max_power_range_uw(const char* control_type, const uint32_t* zones, uint32_t depth, uint64_t* val) {
  return zone_read_u64(control_type, zones, depth, val, POWERCAP_ZONE_FILE_MAX_POWER_RANGE_UW);
}

int powercap_sysfs_zone_get_power_uw(const char* control_type, const uint32_t* zones, uint32_t depth, uint64_t* val) {
  return zone_read_u64(control_type, zones, depth, val, POWERCAP_ZONE_FILE_POWER_UW);
}

int powercap_sysfs_zone_set_enabled(const char* control_type, const uint32_t* zones, uint32_t depth, uint32_t val) {
  int ret;
  int fd;
  if ((fd = open_zone_file(control_type, zones, depth, POWERCAP_ZONE_FILE_ENABLED, O_WRONLY)) < 0) {
    return -errno;
  }
  ret = write_u64(fd, (uint64_t) val);
  close(fd);
  return ret;
}

int powercap_sysfs_zone_get_enabled(const char* control_type, const uint32_t* zones, uint32_t depth, uint32_t* val) {
  uint64_t enabled;
  int ret;
  if (val) {
    if (!(ret = zone_read_u64(control_type, zones, depth, &enabled, POWERCAP_ZONE_FILE_ENABLED))) {
      *val = (uint32_t) enabled;
    }
  } else {
    errno = EINVAL;
    ret = -errno;
  }
  return ret;
}

ssize_t powercap_sysfs_zone_get_name(const char* control_type, const uint32_t* zones, uint32_t depth, char* buf, size_t size) {
  ssize_t ret;
  int fd;
  if ((fd = open_zone_file(control_type, zones, depth, POWERCAP_ZONE_FILE_NAME, O_RDONLY)) < 0) {
    return -errno;
  }
  ret = read_string(fd, buf, size);
  close(fd);
  return ret;
}

int powercap_sysfs_constraint_set_power_limit_uw(const char* control_type, const uint32_t* zones, uint32_t depth, uint32_t constraint, uint64_t val) {
  return constraint_write_u64(control_type, zones, depth, constraint, val, POWERCAP_CONSTRAINT_FILE_POWER_LIMIT_UW);
}

int powercap_sysfs_constraint_get_power_limit_uw(const char* control_type, const uint32_t* zones, uint32_t depth, uint32_t constraint, uint64_t* val) {
  return constraint_read_u64(control_type, zones, depth, constraint, val, POWERCAP_CONSTRAINT_FILE_POWER_LIMIT_UW);
}

int powercap_sysfs_constraint_set_time_window_us(const char* control_type, const uint32_t* zones, uint32_t depth, uint32_t constraint, uint64_t val) {
  return constraint_write_u64(control_type, zones, depth, constraint, val, POWERCAP_CONSTRAINT_FILE_TIME_WINDOW_US);
}

int powercap_sysfs_constraint_get_time_window_us(const char* control_type, const uint32_t* zones, uint32_t depth, uint32_t constraint, uint64_t* val) {
  return constraint_read_u64(control_type, zones, depth, constraint, val, POWERCAP_CONSTRAINT_FILE_TIME_WINDOW_US);
}

int powercap_sysfs_constraint_get_max_power_uw(const char* control_type, const uint32_t* zones, uint32_t depth, uint32_t constraint, uint64_t* val) {
  return constraint_read_u64(control_type, zones, depth, constraint, val, POWERCAP_CONSTRAINT_FILE_MAX_POWER_UW);
}

int powercap_sysfs_constraint_get_min_power_uw(const char* control_type, const uint32_t* zones, uint32_t depth, uint32_t constraint, uint64_t* val) {
  return constraint_read_u64(control_type, zones, depth, constraint, val, POWERCAP_CONSTRAINT_FILE_MIN_POWER_UW);
}

int powercap_sysfs_constraint_get_max_time_window_us(const char* control_type, const uint32_t* zones, uint32_t depth, uint32_t constraint, uint64_t* val) {
  return constraint_read_u64(control_type, zones, depth, constraint, val, POWERCAP_CONSTRAINT_FILE_MAX_TIME_WINDOW_US);
}

int powercap_sysfs_constraint_get_min_time_window_us(const char* control_type, const uint32_t* zones, uint32_t depth, uint32_t constraint, uint64_t* val) {
  return constraint_read_u64(control_type, zones, depth, constraint, val, POWERCAP_CONSTRAINT_FILE_MIN_TIME_WINDOW_US);
}

ssize_t powercap_sysfs_constraint_get_name(const char* control_type, const uint32_t* zones, uint32_t depth, uint32_t constraint, char* buf, size_t size) {
  ssize_t ret;
  int fd;
  if ((fd = open_constraint_file(control_type, zones, depth, constraint, POWERCAP_CONSTRAINT_FILE_NAME, O_RDONLY)) < 0) {
    return -errno;
  }
  ret = read_string(fd, buf, size);
  close(fd);
  return ret;
}
