#!/bin/sh

cd "$(dirname $0)/.."


echo "[1] Running 'coupled' types..."

./tools/batch-run-coupled.sh

echo "Done (coupled)"



echo "[2] Running 'decoupled' types..."

./tools/batch-run-decoupled.sh

echo "Done (decoupled)"



echo "[3] Running 'factors' types..."

./tools/batch-run-step2-factors.sh

echo "Done (factors)"

