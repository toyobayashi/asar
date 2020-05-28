
#ifdef _WIN32
#include "toyo/charset.hpp"
#endif

#include <string>
#include <vector>
#include <cstddef>
#include "toyo/console.hpp"
#include "asar/asar.h"

static void printHelp();
static void printVersion();
static int printRequireArgumentError(const std::string& arg) {
  toyo::console::error("asarcpp error: missing required argument '" + arg + "'");
  return 1;
}

static int printUnknownOptionError(const std::string& arg) {
  toyo::console::error("asarcpp error: unknown option '" + arg + "'");
  return 1;
}

static int printRequireOptionValueError(const std::string& arg) {
  toyo::console::error("asarcpp error: missing required option value '" + arg + "'");
  return 1;
}

static int asar_main(const std::vector<std::string>& args) {
  size_t argc = args.size();
  if (argc == 1 || args[1] == "-h" || args[1] == "--help") {
    printHelp();
    return 0;
  }

  const std::string& args1 = args[1];
  if (args1 == "-V" || args1 == "--version") {
    printVersion();
    return 0;
  }

  if (args1 == "pack" || args1 == "p") {
    std::string dir = "";
    std::string output = "";
    std::string re = "";
    size_t argstart = 2;
    if (argc < 3 || args[2] == "") {
      return printRequireArgumentError("dir");
    }
    if (args[2][0] == '-') {
      if (args[2] == "-u") {
        if (argc < 4 || args[3] == "") {
          return printRequireOptionValueError("-u");
        } else {
          re = args[3];
          argstart = 4;
        }
      } else {
        return printUnknownOptionError(args[2]);
      }
    }
    
    if (argc < argstart + 1 || args[argstart] == "") {
      if (args[argstart][0] == '-') {
        return printUnknownOptionError(args[argstart]);
      }
      return printRequireArgumentError("dir");
    } else {
      dir = args[argstart];
    }
    if (argc < argstart + 2 || args[argstart + 1] == "") {
      return printRequireArgumentError("output");
    } else {
      output = args[argstart + 1];
    }

    asar_status r = asar_pack(dir.c_str(), output.c_str(), re == "" ? nullptr : re.c_str(), nullptr);
    if (r != ok) {
      toyo::console::error(asar_get_last_error_message());
      return 1;
    }
    return 0;
  }

  if (args1 == "list" || args1 == "l") {
    std::string archive = "";
    if (argc < 3 || args[2] == "") {
      return printRequireArgumentError("archive");
    }
    
    archive = args[2];

    asar_t* p = asar_open(archive.c_str());
    if (p == nullptr) {
      toyo::console::error(asar_get_last_error_message());
      return 1;
    }

    asar_list(p);
    asar_close(p);
    return 0;
  }

  if (args1 == "extract" || args1 == "e") {
    std::string archive = "";
    std::string dest = "";
    std::string path = "";
    size_t argstart = 2;
    if (argc < 3 || args[2] == "") {
      return printRequireArgumentError("archive");
    }
    if (args[2][0] == '-') {
      if (args[2] == "-p") {
        if (argc < 4 || args[3] == "") {
          return printRequireOptionValueError("-p");
        } else {
          path = args[3];
          argstart = 4;
        }
      } else {
        return printUnknownOptionError(args[2]);
      }
    }
    
    if (argc < argstart + 1 || args[argstart] == "") {
      if (args[argstart][0] == '-') {
        return printUnknownOptionError(args[argstart]);
      }
      return printRequireArgumentError("archive");
    } else {
      archive = args[argstart];
    }
    if (argc < argstart + 2 || args[argstart + 1] == "") {
      return printRequireArgumentError("dest");
    } else {
      dest = args[argstart + 1];
    }

    asar_t* p = asar_open(archive.c_str());
    if (p == nullptr) {
      toyo::console::error(asar_get_last_error_message());
      return 1;
    }
    asar_status r = asar_extract(p, path != "" ? path.c_str() : "/", dest.c_str());
    if (r != ok) {
      toyo::console::error(asar_get_last_error_message());
      asar_close(p);
      return 1;
    }
    asar_close(p);
    
    return 0;
  }

  if (args1.length() == 0) {
    printHelp();
    return 0;
  }
  
  if (args1[0] == '-') {
    toyo::console::error("asarcpp: '" + args1 + "' is not an command. See 'asarcpp --help'");
    return 1;
  } else {
    return printUnknownOptionError(args1);
  }
}

#ifdef _WIN32
int wmain(int argc, wchar_t** argv) {
  std::vector<std::string> args;
  for (int i = 0; i < argc; i++) {
    args.push_back(toyo::charset::w2a(argv[i]));
  }
  return asar_main(args);
}
#else
int main(int argc, char** argv) {
  std::vector<std::string> args;
  for (int i = 0; i < argc; i++) {
    args.push_back(argv[i]);
  }
  return asar_main(args);
}
#endif

void printHelp() {
  using toyo::console;
  console::log("Usage: asarcpp [options] [command]");
  console::log("");
  console::log("Manipulate asar archive files");
  console::log("");
  console::log("Options:");
  console::log("  -V, --version                             output the version number");
  console::log("  -h, --help                                display help for command");
  console::log("");
  console::log("Commands:");
  console::log("  pack|p [-u <glob>] <dir> <output>         create asar archive");
  console::log("  list|l <archive>                          list files of asar archive");
  console::log("  extract|e [-p <path>] <archive> <dest>    extract files from archive");
}

void printVersion() {
  using toyo::console;
  console::log("v" ASAR_VERSION);
}
