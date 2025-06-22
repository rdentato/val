
#  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
#  SPDX-License-Identifier: MIT

# Simple script to propagate the version in each file that stores it.

# Set the major, minor and patch numbers according to the "Semantic Versioning" rules (https://semver.org)
VER_MAJOR=0
VER_MINOR=4
VER_PATCH=9

VER_STAGE=C
# The development stage (and any additional version info) can be set in VER_STAGE.
# A -> alpha
# B -> beta
# C -> RC (release candidate)
# F -> Final (empty)
#
# Everything else will be left "as is"


## -- DO NOT TOUCH ANYTHING BELOW THIS LINE

VER_VAL_H=`printf "0x%02d%02d%03d%c" ${VER_MAJOR} ${VER_MINOR} ${VER_PATCH} ${VER_STAGE}`

case "$VER_STAGE" in
  C) STAGE="-RC"    ;;
  A) STAGE="-alpha" ;;
  B) STAGE="-beta"  ;;
  F) STAGE=""      ;;
  *) STAGE="-$VER_STAGE" ;;
esac

VER_SPDX=`printf "%d.%d.%03d" ${VER_MAJOR} ${VER_MINOR} ${VER_PATCH}`
VER_SPDX=${VER_SPDX}${STAGE}

VER_README="${VER_SPDX// /%20}"

VER_VAL_H_FULL="#define VAL_VERSION ${VER_VAL_H}"
VER_VAL_SPDX_FULL="PackageVersion: ${VER_SPDX}"
VER_README_FULL="[![Version](https:\/\/img.shields.io\/badge\/version-${VER_README}-blue.svg)](https:\/\/github.com\/rdentato\/val)"

sed -i -e "s/^#define VAL_VERSION .*/${VER_VAL_H_FULL}/" src/val.h
sed -i -e "s/^PackageVersion: .*/${VER_VAL_SPDX_FULL}/" val.spdx
sed -i -e "s/^\[!\[Version\].*/${VER_README_FULL}/" README.md

echo "Val version set to: ${VER_SPDX}"