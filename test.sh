#!/bin/bash
{ test -f build/crisp-csv && { objdump -s build/crisp-csv | grep CSV_TEST >/dev/null || make clean; }; }; make CSV_TEST=1
cp ./src/test/crisp-csv.cfg ./build/
./build/crisp-csv
