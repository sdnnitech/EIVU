#!/bin/sh

cd "$(dirname $0)/.."


echo "[1] Building 'coupled' types..."

./tools/batch-build-coupled.sh

echo "Done (coupled)"



echo "[2] Building 'decoupled' types..."

./tools/batch-build-decoupled.sh

echo "Done (decoupled)"



echo "[3] Building 'factors' types..."

./tools/batch-build-factors.sh

echo "Done (factors)"

