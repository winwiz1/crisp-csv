#!/bin/bash
{ test -f build/crisp-csv && { objdump -s build/crisp-csv | grep CSV_TEST >/dev/null && make clean; }; }; make
