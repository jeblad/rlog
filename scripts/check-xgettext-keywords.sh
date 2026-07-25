#!/bin/bash
# Fails if the translatable macros defined in rlog.hpp.in and the
# --keyword= entries in RLOG_XGETTEXT_ARGS (CMakeLists.txt) go out of sync.
set -euo pipefail

HPP_IN="${1:?usage: check-xgettext-keywords.sh <rlog.hpp.in> <CMakeLists.txt>}"
CMAKE_LISTS="${2:?usage: check-xgettext-keywords.sh <rlog.hpp.in> <CMakeLists.txt>}"

# Translatable macros are every top-level #define that isn't one of the
# internal RLOG_*_DO/RLOG_GETTEXT/RLOG_VERSION helpers.
export LC_ALL=C

macros_in_header=$(grep -oP '^#define \K\w+(?=\()' "$HPP_IN" | grep -v '^RLOG_' | sort -u)

keywords_in_cmake=$(grep -oP -- '--keyword=\K\w+' "$CMAKE_LISTS" | sort -u)

missing_from_cmake=$(comm -23 <(echo "$macros_in_header") <(echo "$keywords_in_cmake"))
missing_from_header=$(comm -13 <(echo "$macros_in_header") <(echo "$keywords_in_cmake"))

if [[ -n "$missing_from_cmake" || -n "$missing_from_header" ]]; then
    echo "RLOG_XGETTEXT_ARGS in CMakeLists.txt is out of sync with the macros in rlog.hpp.in:"
    [[ -n "$missing_from_cmake" ]] && printf '  macro defined but missing --keyword=: %s\n' $missing_from_cmake
    [[ -n "$missing_from_header" ]] && printf '  --keyword= with no matching macro: %s\n' $missing_from_header
    exit 1
fi

echo "RLOG_XGETTEXT_ARGS keywords match the macros in rlog.hpp.in."
