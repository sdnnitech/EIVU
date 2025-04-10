#!/bin/sh

cd "$(dirname $0)/.."


echo "[1] Building 'coupled' types..."

./tools/build/batch-build-coupled.sh

echo "Done (coupled)"



echo "[2] Building 'decoupled' types..."

./tools/build/batch-build-decoupled.sh

echo "Done (decoupled)"



echo "[3] Building 'factors' types..."

./tools/build/batch-build-factors.sh

echo "Done (factors)"

