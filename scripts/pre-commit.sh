#!/bin/bash
set -e

# Get the project root directory
REPO_ROOT=$(git rev-parse --show-toplevel)
cd "$REPO_ROOT"

echo "Running pre-commit version sync and tests..."

# 1. Regenerate the header from package.json version via configure_file.
cmake -B build -S .

# 2. Stage the freshly generated header.
git add include/rlog/rlog.hpp

# 3. Run tests to ensure the versioning and code are valid
ctest --test-dir build --output-on-failure

echo "Pre-commit checks passed!"
exit 0
