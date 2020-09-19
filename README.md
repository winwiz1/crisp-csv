# Crisp CSV
This repository helps to build an ETL (Extract-Transform-Load) utility that works with CSV files. The utility applies a range of checks to the input data, merges the files and creates a sanitized, denormalized upload that could be imported into a non-SQL database as a part of an ETL process. The data validation checks are meant to ensure the integrity of the data and avoid ingestion errors triggered by the database engine.

The utility can also be used to read specific CSV fields from the input data and reorder the fields.

Off-the-shelf the utility works with data from the Google COVID-19 Open Data [repository](https://github.com/GoogleCloudPlatform/covid-19-open-data). However it has been designed from the outset to be easily customisable and uses templates to simplify the task of reconfiguring the program to work with selected pieces (e.g. fields) of CSV data in other projects. The details are provided under the [Customisation](#customisation) heading.

The utility is written in C++ to achieve the high performance required to process large amount of data. This is especially beneficial if your ETL process needs to be recurring and bound to a certain time schedule. The specific performance metrics can be found below.

The utility can be built under Linux or on Windows with [WSL](https://docs.microsoft.com/en-us/windows/wsl/about). If you have GCC 8 or later installed, then after cloning the repository execute either `test.sh/test.cmd` to build and test the program or `build.sh/build.cmd` to perform the production build - it's that simple.

Deployment and usage are straightforward as well. All the work is done by one Linux program that consists of a single executable with an optional configuration file.

## Table of Contents
- [Functionality](#functionality)
  - [Data Validation](#data-validation)
  - [Data Filtration](#data-filtration)
- [Performance](#performance)
- [Build](#build)
  - [Prerequisites](#prerequisites)
  - [Build Steps ](#build-steps )
  - [Testing](#testing)
- [Usage](#usage)
  - [Data Location](#data-location)
  - [Running the Utility](#running-the-utility)
  - [Configuration](#configuration)
- [Customisation](#customisation)
  - [Existing Design](#existing-design)
  - [Making Changes](#making-changes)
- [Credits](#credits)
- [License](#license)

## Functionality
Out of the box the utility works with two Google repository files: [`epidemiology.csv`](https://storage.googleapis.com/covid19-open-data/v2/epidemiology.csv) and [`index.csv`](https://storage.googleapis.com/covid19-open-data/v2/index.csv). The former contains daily records (e.g. data rows) of COVID-19 case counts applicable to a certain geographical or administrative region denoted by the record's `index` field. The latter provides a description for the area behind each index.

> The CSV field called `index` will be referred to as `geoindex`.

The utility performs a set of checks to validate each row, identifies the rows that need to be rejected as invalid, filters the data (by skipping certain rows) and creates an output file by merging the selected CSV fields taken from both files. The output file contains the following columns:
```
date, geoindex, country, state/province, locality, confirmed cases, recovered cases, deaths, aggregation level
```
Before exiting the utility prints out a summary with the counts of rejected and filtered rows. Pressing Ctrl+C terminates execution, the utility displays a warning about incomplete output files and exits with non-zero exit code.

### Data Validation
The following checks are performed on each row of data:

1. Epidemiology.csv
   - Check the `date` against a regular expression to ensure the date is in the format that can be ingested during import.
   - Check the `geoindex` against a regular expression to ensure the literal is formed correctly e.g. contains a country code, certain number of correctly positioned underscore characters reflecting its geographical/administrative hierarchy etc.
   - Ensure the `geoindex` can be found in the `index.csv` file.

2. Index.csv
   - Check the `geoindex` against a regular expression to ensure it is formed correctly.
   - Check the value contained in the `aggregation_level` field is valid.
   -  Verify the geographical/administrative hierarchy of the `geoindex` is consistent with the value contained in the `aggregation_level` field.
    - Perform a range of checks to ensure the record is structurally sound. It includes checking the record has `country_name` data followed by checking it has state/province (e.g. `subregion1_name`) data if it must be present for this particular geoindex or doesn’t have it in case this data must be missing for a country-wide geoindex. Similar present-or-missing checks for the locality data, e.g. both `subregion2_name` and `locality_name` fields which must be either present or missing depending on the `aggregation_level` value.
    - Check that each `xxx_name` field mentioned above has length no less than the minimal length applicable to this particular field.

If any check fails the row is rejected. The utility creates two error files (next to the output file) used to store the rejected epidemiology and index rows.

### Data Filtration
The utility can be built to work with records related to the two levels of geographical (or administrative) hierarchy only: country-wide level (L0) and state/province level (L1). This build configuration filters out records pertaining to the COVID-19 case counts that apply to the localities at the levels L3 and L4. Another build configuration works with all levels - see the [Configuration](#configuration) section.

The following data rows are filtered:

1. Epidemiology.csv
   - Records below the state/province level - depending on the build configuration.
   - Records with all the three cumulative epidemiology metrics missing.
   - Records for today's Australian data if the cumulative confirmed case count is present with the other two cumulative metrics missing.
   - Records pertaining to UK [NUTS](https://en.wikipedia.org/wiki/NUTS_statistical_regions_of_the_United_Kingdom) regions.

2. Index.csv
   - Records below the state/province level - depending on the build configuration.

The counts of the filtered rows are included into the summary printed out when the utility finishes, however the rows are not saved.
## Performance
On Google Cloud Platform's `f1-micro` VM  the utility running inside a Docker container processes ~2 mil epidemiology rows with 18000  index rows in 15 seconds producing 140 MB output file and generating the following summary upon exit:
```
crisp-csv - version 1.1.5
crisp-csv - processed 2164142 data rows
crisp-csv - rejected 266 data rows due to index processing failure
crisp-csv - rejected 1 data row
crisp-csv - filtered 2073 data rows
crisp-csv - rejected 5 index rows
crisp-csv - execution time: 15 seconds
```

> The first row in both input files contains column names and is rejected as invalid data. It explains the rejection of 1 data row and 1 index row. The remaining 4 rejected index rows are not included into the validated geoindex which causes 266 data rows (with the rejected geoindices) to be rejected as well.

Building the configuration that filters out records below the state/province level cuts the execution time approximately in half.
## Build
The utility is built and runs under Linux. It can be built on Windows with WSL in which case [install](https://docs.microsoft.com/en-us/windows/wsl/install-win10#install-your-linux-distribution-of-choice) Debian or Ubuntu 20.04 LTS from Microsoft Store, alternatively perform a [manual](https://docs.microsoft.com/en-us/windows/wsl/install-manual) installation.
### Prerequisites
On Debian 10 or Ubunty 20.04 the only build prerequisite is the `build-essential` package.
   - To install the package on Debian/Ubunty run the following command:<br/>
     `sudo apt update && apt install build-essential`

     >If you have an earlier version of Linux distribution with older GCC installed then it might not support C++17 features. In this case you will need to install GCC 8 by following the `install` steps in the `.travis.yml` file.  The steps ensure that Travis CI VM, which comes with Ubuntu 18.04 and GCC 7, gets upgraded to GCC 8.

   - To install the package on WSL follow the steps described in this [link](https://code.visualstudio.com/docs/cpp/config-wsl#_set-up-your-linux-environment).

If you intend to use VS Code, install the Remote - WSL [extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-wsl).
### Build Steps
   - On Linux execute: `./build.sh`
   - On Windows execute: `build.cmd`
   - If you prefer to build using VS Code execute: `ide.cmd`. <br/>Once VS Code starts, press `Ctrl+Shift+B` to execute the Default Build Task or simply type `make` in the Terminal window.

After the build process finishes, the `crisp-csv` executable can be found in the `./build/` subdirectory. It is named after the top-level project directory so in order to rename the program you can change the directory name. Then run `make clean && make` to rebuild. Alternatively edit the `.cmd` files replacing `crisp-csv` with the new name and execute `clean.cmd` followed by `build.cmd`.
### Testing
   - On Linux execute: `./test.sh`
   - On Windows execute: `test.cmd`

This will build the test configuration and overwrite the executable located in the `build/` subdirectory. Executing `build.cmd` builds the production configuration overwriting the same executable.

> Switching from one configuration to another triggers a full rebuild that takes more time than an incremental build typically facilitated by `make` during development.

The repository is integrated with [travis-ci.com](https://travis-ci.com/) for Continuous Integration so that every push causes Travis CI to start a VM, clone the repository, perform a build and run the tests. The build/test outcome is shown by the CI icon at the top (next to the last commit hash). To access the build/test log, click on the icon.

## Usage
### Data Location
At run-time the production build of the utility requires a readable and writeable subdirectory `csv/` to exist in the directory that contains the executable. It will look for the `epidemiology.csv`.and `index.csv` files in this subdirectory. To satisfy this requirement for the cloned repository download the [`epidemiology.csv`](https://storage.googleapis.com/covid19-open-data/v2/epidemiology.csv) and [`index.csv`](https://storage.googleapis.com/covid19-open-data/v2/index.csv) files into the `crisp-csv/build/csv/` directory.
### Running the Utility
- On Linux execute: `./run.sh`
- On Windows execute: `run.cmd`
    The utility will run in WSL.
 - If using VS Code (started by `ide.cmd`), type `build/crisp-csv` in the Terminal window.

The output file and the two error files with rejected epidemiology and index records will be created in the `csv/` subdirectory. Already existing files will be overwritten.
### Configuration
The functionality provided by the utility can be customised during builds and at run-time.

1. Build time configuration<br/>
Out of the box the utility processes all levels of the geographical (or administrative) hierarchy. It can be restricted to the first two (country and state/province) levels by editing the `makefile` and changing the `CFLAGS` variable from `-DSKIP_LOCALITIES=0` to `-DSKIP_LOCALITIES=1`. This requires a full rebuild so execute `clean.cmd` or its Linux equivalent after changing the `makefile`.

2. Run-time configuration<br/>
At run-time the utility looks for a configuration file `<executable-file-name>.cfg` e.g. `crisp-csv.cfg` located in the same directory. The file contains a JSON object with the following keys:
    ````
    {
     "processedDataRows": 1500000,
     "rejectedDataRows": 200,
     "rejectedIndexRows": 20,
     "relaxIndexChecks": false,
     "filterUkNuts": true,
     "filterAuData": true
    }
    ````
    The first three keys represent the thresholds that affect the utility exit code. If the respective row counts are less than the first threshold or greater than the other two thresholds, the utility returns a non-zero exit code indicating a failure. This can be used to terminate an ETL pipeline, disable data copying from a staging environment to production, etc.

    The last two keys affect filtering described in the [Data Filtration](#data-filtration) section. The `relaxIndexChecks` setting, if set to `true`, drops several geoindex checks and makes the utility accept `UA_KBP` as a valid geoindex, see [this](https://github.com/GoogleCloudPlatform/covid-19-open-data/issues/156) issue for more details.

    If the configuration file cannot be found, the utility falls back to the defaults specified in`RuntimeConfig.h` In case the configuration file is found but cannot be parsed the utility terminates.

## Customisation
Customising the utility to work with CSV data structured differently requires familiarity with the existing design.
### Existing Design
The class `CsvFile` acts as CSV processing engine and reads the input file with epidemiology data line by line. The class is configured to read only certain CSV fields from the input file and combined together, those selected fields form a CSV record that needs to be processed further. The class constructor gets an array of CSV field indices from a Factory that creates an instance of the class. Each index denotes a CSV field from the input file.

`CsvFile` is data-agnostic e.g. it has no knowledge of what data is contained in which CSV field.  Therefore it is unable to perform data processing on its own and needs to delegate the work to handlers which are the classes derived from `CsvScanner` or `CsvProcessor`.  Both handlers are created by a Factory.

The handler derived from `CsvScanner` is called by `CsvFile` to scan each CSV record and decide if the record should be rejected as invalid or filtered as not needed or accepted. If the record is accepted then `CsvFile` passes the selected fields of the CSV record to `CsvProcessor` for additional processing. The array of indices passed to `CsvFile` constructor actually contains tuples so it's an array of tuples. Each tuple consists of a CSV field index and a boolean flag. If set to true, the flag tells `CsvFile` to call `CsvProcessor` and pass the field's content to it for processing.

The handler derived from `CsvProcessor` reads the secondary data file (which is the `index.csv` file in the implementation related to Google COVID-19 Open Data repository), sanitizes the geoindex by rejecting or filtering or accepting index rows and then responds to calls from `CsvFile` by merging geoindex related information into the CSV record.
### Making Changes
Create your custom CSV record scanner and field processor. Extend the Factory to produce both and inject smart pointers holding their instances into the `CsvFile` along with a modified array of CSV field indices. Decide which CSV field(s) require further processing and alter the tuples accordingly. Add or replace members of the `CsvFieldCounts` structure to adjust the lengths of the CSV records and modify the `RuntimeConfig` class as necessary.

Adding new `.h` or `.cpp` files and renaming the existing source files doesn't require changing the `makefile`. It requires changes if you add a subdirectory to the `src/` directory in which case the changes should reflect the actions applied in the `makefile` to the existing subdirectories, namely `config/`, `handlers/` and `test/`.
## Credits
The source code includes the `json.hpp` file taken from the [JSON for Modern C++]( https://github.com/nlohmann/json) repository to implement parsing of the JSON configuration file.

Tests include the file `catch.hpp`  taken from the [Catch2](https://github.com/catchorg/Catch2) test framework repository.

The `makefile` utilises  several recipes mentioned in the comments.

## License
The software is licensed under the [MIT License](./LICENSE).
