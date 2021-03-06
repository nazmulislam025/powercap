/**
 * A simple interface for configuring powercaps using sysfs.
 * Parameters are never allowed to be NULL.
 * Unless otherwise stated, all functions return 0 on success or a negative value on error.
 *
 * These operations do basic file I/O.
 * It is expected that callers handle I/O errors.
 * Setter functions do not verify that written values are actually accepted by the kernel.
 *
 * @author Connor Imes
 * @date 2016-06-01
 */
#ifndef _POWERCAP_H_
#define _POWERCAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <unistd.h>

/**
 * File descriptors for a zone.
 */
typedef struct powercap_zone {
  int max_energy_range_uj;
  int energy_uj;
  int max_power_range_uw;
  int power_uw;
  int enabled;
  int name;
} powercap_zone;

/**
 * File descriptors for a constraint.
 */
typedef struct powercap_constraint {
  int power_limit_uw;
  int time_window_us;
  int max_power_uw;
  int min_power_uw;
  int max_time_window_us;
  int min_time_window_us;
  int name;
} powercap_constraint;

/**
 * Zone file enumeration.
 */
typedef enum powercap_zone_file {
  POWERCAP_ZONE_FILE_MAX_ENERGY_RANGE_UJ,
  POWERCAP_ZONE_FILE_ENERGY_UJ,
  POWERCAP_ZONE_FILE_MAX_POWER_RANGE_UW,
  POWERCAP_ZONE_FILE_POWER_UW,
  POWERCAP_ZONE_FILE_ENABLED,
  POWERCAP_ZONE_FILE_NAME
} powercap_zone_file;

/**
 * Constraint file enumeration.
 */
typedef enum powercap_constraint_file {
  POWERCAP_CONSTRAINT_FILE_POWER_LIMIT_UW,
  POWERCAP_CONSTRAINT_FILE_TIME_WINDOW_US,
  POWERCAP_CONSTRAINT_FILE_MAX_POWER_UW,
  POWERCAP_CONSTRAINT_FILE_MIN_POWER_UW,
  POWERCAP_CONSTRAINT_FILE_MAX_TIME_WINDOW_US,
  POWERCAP_CONSTRAINT_FILE_MIN_TIME_WINDOW_US,
  POWERCAP_CONSTRAINT_FILE_NAME
} powercap_constraint_file;

/**
 * Get the filename for a zone file type.
 * Returns the number of characters written excluding the terminating null byte, or a negative value in case of error.
 */
int powercap_zone_file_get_name(powercap_zone_file type, char* buf, size_t size);

/**
 * Get the filename for a constraint file type.
 * Returns the number of characters written excluding the terminating null byte, or a negative value in case of error.
 */
int powercap_constraint_file_get_name(powercap_constraint_file type, uint32_t constraint, char* buf, size_t size);

/**
 * Get the zone's maximum energy range in microjoules.
 */
int powercap_zone_get_max_energy_range_uj(const powercap_zone* zone, uint64_t* val);

/**
 * Get the zone's current energy in microjoules.
 */
int powercap_zone_get_energy_uj(const powercap_zone* zone, uint64_t* val);

/**
 * Reset the zone's energy_uj value to 0.
 */
int powercap_zone_reset_energy_uj(const powercap_zone* zone);

/**
 * Get the zone's maximum power range in microwatts.
 */
int powercap_zone_get_max_power_range_uw(const powercap_zone* zone, uint64_t* val);

/**
 * Get the zone's current power in microwatts.
 */
int powercap_zone_get_power_uw(const powercap_zone* zone, uint64_t* val);

/**
 * Set the zone's enabled value.
 */
int powercap_zone_set_enabled(const powercap_zone* zone, int val);

/**
 * Get the zone's enabled value.
 */
int powercap_zone_get_enabled(const powercap_zone* zone, int* val);

/**
 * Get the zone's name.
 * (There does not appear to be a get_name for zones defined in powercap, but we see it in sysfs anyway.)
 * Returns a non-negative value for the number of bytes read, a negative value in case of error.
 */
ssize_t powercap_zone_get_name(const powercap_zone* zone, char* buf, size_t size);

/**
 * Set the constraint's current power limit in microwatts.
 */
int powercap_constraint_set_power_limit_uw(const powercap_constraint* constraint, uint64_t val);

/**
 * Get the constraint's current power limit in microwatts.
 */
int powercap_constraint_get_power_limit_uw(const powercap_constraint* constraint, uint64_t* val);

/**
 * Set the constraint's current time window in microseconds.
 */
int powercap_constraint_set_time_window_us(const powercap_constraint* constraint, uint64_t val);

/**
 * Get the constraint's current time window in microseconds.
 */
int powercap_constraint_get_time_window_us(const powercap_constraint* constraint, uint64_t* val);

/**
 * Get the constraint's maximum power in microwatts.
 */
int powercap_constraint_get_max_power_uw(const powercap_constraint* constraint, uint64_t* val);

/**
 * Get the constraint's minimum power in microwatts.
 */
int powercap_constraint_get_min_power_uw(const powercap_constraint* constraint, uint64_t* val);

/**
 * Get the constraint's maximum time window in microseconds.
 */
int powercap_constraint_get_max_time_window_us(const powercap_constraint* constraint, uint64_t* val);

/**
 * Get the constraint's minimum time window in microseconds.
 */
int powercap_constraint_get_min_time_window_us(const powercap_constraint* constraint, uint64_t* val);

/**
 * Get the constraint's name.
 * Returns a non-negative value for the number of bytes read, a negative value in case of error.
 */
ssize_t powercap_constraint_get_name(const powercap_constraint* constraint, char* buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif
