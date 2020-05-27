# asar

C++ implemention of [asar](https://github.com/electron/asar).

## Usage

```
Usage: asarcpp [options] [command]

Manipulate asar archive files

Options:
  -V, --version                             output the version number
  -h, --help                                display help for command

Commands:
  pack|p [-u <regex>] <dir> <output>        create asar archive
  list|l <archive>                          list files of asar archive
  extract|e [-p <path>] <archive> <dest>    extract files from archive
```

## Build

Require Node.js, CMake, VC++ / GCC

Windows:

``` cmd
> npm install
> .\build.bat Win32 Release static
```

``` bash
$ npm install
$ ./build.sh Release
```
