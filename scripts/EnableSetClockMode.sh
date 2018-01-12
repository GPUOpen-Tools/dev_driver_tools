#!/bin/bash

# Allow non-root applications to set GPU clock mode

sudo chmod ugo+w /sys/class/drm/card0/device/power_dpm_force_performance_level

echo "done"
