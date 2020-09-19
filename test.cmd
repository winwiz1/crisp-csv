@echo OFF
wsl bash -c "{ test -f build/crisp-csv && { objdump -s build/crisp-csv | grep CSV_TEST >/dev/null || make clean; }; }; make CSV_TEST=1"
copy /V /Y src\test\crisp-csv.cfg build\
wsl build/crisp-csv
