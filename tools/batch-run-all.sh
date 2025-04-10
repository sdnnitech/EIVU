#!/bin/sh

cd "$(dirname $0)/.."


echo "[1] Running 'coupled' types..."

./tools/run/batch-run-coupled.sh

echo "Done (coupled)"



echo "[2] Running 'decoupled' types..."

./tools/run/batch-run-decoupled.sh

echo "Done (decoupled)"



echo "[3] Running 'factors' types..."

./tools/run/batch-run-factors.sh

echo "Done (factors)"

