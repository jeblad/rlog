#!/bin/bash
set -e

# Get the project root directory
REPO_ROOT=$(git rev-parse --show-toplevel)
cd "$REPO_ROOT"

echo "Running pre-commit version sync and tests..."

# 1. Ensure build directory exists and is configured
cmake -B build -S .

# 2. Update the header file (triggered by configuration)
# This is handled by configure_file in CMakeLists.txt

# 3. Stage the updated header so it's included in the current commit
git add include/rlog/rlog.hpp

# 4. Run tests to ensure the versioning and code are valid
ctest --test-dir build --output-on-failure

echo "Pre-commit checks passed!"
exit 0
