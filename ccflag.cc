#include "ccflag.h"
#include <stdlib.h>
#include <string.h>
#include <map>
#include <iostream>
#include <fstream>

namespace ccflag {
namespace xx {
struct FlagInfo {
    FlagInfo(const char* _type_str, const char* _name, const char* _value,
             const char* _help, const char* _file, void* _addr, int _type)
        : type_str(_type_str), name(_name), value(_value), help(_help),
          file(_file), addr(_addr), type(_type) {
    }

    const char* const type_str;
    const char* const name;
    const char* const value;    // default value
    const char* const help;
    const char* const file;     // file where the flag is defined
    void* const addr;           // pointer to the flag variable
    int type;
};

class Flagger {
  public:
    typedef std::map<std::string, FlagInfo*> Map;
    typedef Map::iterator Iter;

    static Flagger* Instance() {
        static Flagger kFlagger;
        return &kFlagger;
    }

    ~Flagger() {
        for (Iter it = _map.begin(); it != _map.end(); ++it) {
            delete it->second;
        }
        _map.clear();
    }

    void AddFlag(const char* type_str, const char* name, const char* value,
                 const char* help, const char* file, void* addr, int type);

    bool SetFlagValue(const std::string& flg, const std::string& val,
                      std::string& err);

    bool SetBoolFlags(const std::string& flg, std::string& err);

    void ShowFlagsInfo(std::ostream& os);

  private:
    Map _map;
    Flagger() {
    }
};

static bool StrToBool(const std::string& v, bool* p, std::string& err) {
    if (v == "true" || v == "1") {
        *p = true;
        return true;
    }

    if (v == "false" || v == "0") {
        *p = false;
        return true;
    }

    err = "invalid value for bool";
    return false;
}

static bool StrToDouble(const std::string& v, double* p, std::string& err) {
    char* end = NULL;
    *p = ::strtod(v.c_str(), &end);

    bool ok = (end == v.c_str() + v.size());
    if (!ok) err = "invalid value for double";

    return ok;
}

static bool StrToU64(const std::string& value, uint64* p, bool* negative,
                     std::string& err) {
    char unit = '\0';
    std::string v(value), s("KkMmGgTt");

    if (!v.empty() && s.find(*v.rbegin()) != std::string::npos) {
        unit = *v.rbegin();
        v.resize(v.size() - 1);
    }

    err = "invalid value for integer";

    ::size_t pos = v.find_first_not_of('-');
    if (pos > 1) return false;

    if (negative != NULL) *negative = (pos == 1);

    char* end = NULL;
    const char* beg = (pos == 1) ? v.c_str() + 1 : v.c_str();
    uint64 u = ::strtoul(beg, &end, 0);
    if (end != v.c_str() + v.size()) return false;

    switch (::tolower(unit)) {
      case '\0':
        break;
      case 'k':
        u <<= 10;
        break;
      case 'm':
        u <<= 20;
        break;
      case 'g':
        u <<= 30;
        break;
      case 't':
        u <<= 40;
        break;
    }

    err.clear();
    *p = (pos == 1 ? -static_cast<int64>(u) : u);
    return true;
}

static bool StrToU32(const std::string& v, uint32* p, std::string& err) {
    uint64 u;
    bool negative;

    bool ret = StrToU64(v, &u, &negative, err);
    if (!ret) return false;

    uint64 abs = (negative ? -static_cast<int64>(u) : u);
    if (abs > static_cast<uint32>(-1)) {
        err = "overflow for 32 bit integer";
        return false;
    }

    uint32 u32 = static_cast<uint32>(abs);
    *p = negative ? -static_cast<int32>(u32) : u32;
    return true;
}

inline bool& ErrToFile() {
    static bool kErrToFile = false;
    return kErrToFile;
}

static void PrintErrMsg(const std::string& err, const std::string& param) {
    if (ErrToFile()) {
        std::ofstream ofs("err.log", std::ofstream::out | std::ofstream::app);
        if (ofs) ofs << err << ": " << param << std::endl;
    } else {
        std::cerr << err << ": " << param << std::endl;
    }
}

#define EXIT_ON_ERR(err, param) \
    do { \
        ::ccflag::xx::PrintErrMsg(err, param); \
        ::exit(0); \
    } while (0);

void Flagger::AddFlag(const char* type_str, const char* name, const char* value,
                      const char* help, const char* file, void* addr,
                      int type) {
    FlagInfo* fi = new FlagInfo(type_str, name, value, help, file, addr, type);

    std::pair<Iter, bool> ret = _map.insert(Map::value_type(name, fi));
    if (!ret.second) {
        EXIT_ON_ERR("more than one flags defined with the same name", name);
    }
}

bool Flagger::SetFlagValue(const std::string& flg, const std::string& val,
                           std::string& err) {
    Iter it = _map.find(flg);
    if (it == _map.end()) {
        err = "flag not found";
        return false;
    }

    FlagInfo* fi = it->second;
    switch (fi->type) {
      case TYPE_string:
        *static_cast<std::string*>(fi->addr) = val;
        return true;

      case TYPE_bool:
        return StrToBool(val, static_cast<bool*>(fi->addr), err);

      case TYPE_int32:
      case TYPE_uint32:
        return StrToU32(val, static_cast<uint32*>(fi->addr), err);

      case TYPE_int64:
      case TYPE_uint64:
        return StrToU64(val, static_cast<uint64*>(fi->addr), NULL, err);

      case TYPE_double:
        return StrToDouble(val, static_cast<double*>(fi->addr), err);
    }

    err = "unknown flag type";
    return false;
}

bool Flagger::SetBoolFlags(const std::string& flg, std::string& err) {
    Iter it = _map.find(flg);
    if (it != _map.end()) {
        if (it->second->type != TYPE_bool) {
            err = "value not set for non-bool flag";
            return false;
        }

        *static_cast<bool*>(it->second->addr) = true;
        return true;
    }

    for (::size_t i = 0; i < flg.size(); ++i) {
        it = _map.find(flg.substr(i, 1));
        if (it == _map.end()) {
            err = "invalid combination of bool flags";
            return false;
        }

        *static_cast<bool*>(it->second->addr) = true;
    }

    return true;
}

void Flagger::ShowFlagsInfo(std::ostream& os) {
    for (Iter it = _map.begin(); it != _map.end(); ++it) {
        const FlagInfo* fi = it->second;
        if (fi->help[0] == '\0') continue;  // ignore flags with empty help info

        os << "--" << fi->name << ": " << fi->help << "\n\t type: "
           << fi->type_str << "\t default: " << fi->value << "\n\t from: "
           << fi->file << '\n';
    }

    std::flush(os);
}

FlagSaver::FlagSaver(const char* type_str, const char* name, const char* value,
                     const char* help, const char* file, void* addr, int type) {
    Flagger::Instance()->AddFlag(type_str, name, value, help, file, addr, type);
}

// remove the heading and trailing whitespace
static void TrimString(std::string& s) {
    for (::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\"' || s[i] == '\t') s[i] = ' ';
    }

    if (s.empty() || (s[0] != ' ' && *s.rbegin() != ' ')) return;

    ::size_t bp = s.find_first_not_of(' ');
    if (bp == std::string::npos) {
        s.clear();
        return;
    }

    ::size_t ep = s.find_last_not_of(' ');
    s = s.substr(bp, ep - bp + 1);
}

static char** CommandLineToArgv(const std::string& args, int* argc) {
    char* buf = (char*) ::malloc(args.size() + 1);
    ::memset(buf, 0, args.size() + 1);

    char* argv0 = buf;

    bool in_qm = false;
    bool after_space = true;
    std::vector<char*> v;

    const char* s = args.c_str();
    while (*s == ' ' || *s == '\t') {
        ++s;
    }

    for (; *s != '\0'; ++s) {
        switch (*s) {
          case '\"':
            in_qm = !in_qm;
            break;

          case ' ':
          case '\t':
            if (in_qm) {
                *buf++ = *s;  // reserve space in quotation marks
            } else if (*(buf - 1) != '\0') {
                *buf++ = '\0';
                after_space = true;
            }
            break;

        default:
            *buf++ = *s;
            if (after_space) {
                v.push_back(buf - 1);
                after_space = false;
            }
        }
    }

    *argc = static_cast<int>(v.size()) + 1;
    char** argv = (char**) ::malloc(sizeof(char*) * (v.size() + 1));

    argv[0] = argv0;

    for (::size_t i = 0; i < v.size(); ++i) {
        argv[i + 1] = v[i];
    }

    return argv;
}
}  // namespace xx

void init_ccflag(int argc, char** argv, std::vector<std::string>* v) {
    if (argc <= 1) return;

    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    if (args.size() == 1 && args[0].find_first_not_of('-') == std::string::npos) {
        if (args[0].size() < 3) { /* args[0] == "-" || args[0] == "--" */
            xx::Flagger::Instance()->ShowFlagsInfo(std::cerr);
        } else {
            std::ofstream ofs("flg.log");
            if (ofs) xx::Flagger::Instance()->ShowFlagsInfo(ofs);
        }
        ::exit(0);
    }

    ::size_t beg = 0;
    if (!args.empty() && args[0].find_first_not_of('.') == std::string::npos) {
        xx::ErrToFile() = true;
        beg = 1;
    }

    std::string flg, val;
    for (::size_t i = beg; i < args.size(); ++i) {
        const std::string& arg = args[i];

        if (arg[0] != '-') {
            if (v != NULL) v->push_back(arg);  // non-flag element
            continue;
        }

        ::size_t bp = arg.find_first_not_of('-');
        ::size_t ep = arg.find('=');

        if (ep <= bp) {
            EXIT_ON_ERR("invalid parameter", arg);
        }

        std::string err;

        if (ep == std::string::npos) {
            flg = arg.substr(bp);
            if (!xx::Flagger::Instance()->SetBoolFlags(flg, err)) {
                EXIT_ON_ERR(err, arg);
            }

        } else {
            flg = arg.substr(bp, ep - bp);
            val = arg.substr(ep + 1);
            if (!xx::Flagger::Instance()->SetFlagValue(flg, val, err)) {
                EXIT_ON_ERR(err, arg);
            }
        }
    }
}

void init_ccflag(const std::string& args, std::vector<std::string>* v) {
    int argc;
    char** argv = xx::CommandLineToArgv(args, &argc);
    init_ccflag(argc, argv, v);

    ::free(*argv);
    ::free(argv);
}

// Ignore all errors in config file
void init_ccflag(int argc, char** argv, const std::string& config) {
    init_ccflag(argc, argv);

    if (config.empty()) return;

    std::ifstream ifs(config.c_str());
    if (!ifs) {
        xx::PrintErrMsg("failed to open config file", config);
        return;
    }

    std::string line, flg, val, err;
    while (std::getline(ifs, line)) {
        xx::TrimString(line);
        if (line.empty() || line[0] == '#') continue;

        ::size_t pos = line.find('#');
        if (pos != std::string::npos) {
            line.resize(pos);
        }

        pos = line.find('=');
        if (pos == std::string::npos) {
            xx::PrintErrMsg("config error, value not set for flag", line);
            continue;
        }

        flg = line.substr(0, pos);
        val = line.substr(pos + 1);

        xx::TrimString(flg);
        xx::TrimString(val);

        if (!xx::Flagger::Instance()->SetFlagValue(flg, val, err)) {
            xx::PrintErrMsg(err, line);
            continue;
        }
    }
}

bool SetFlagValue(const std::string& name, const std::string& value) {
    std::string err;
    bool ret = xx::Flagger::Instance()->SetFlagValue(name, value, err);

    if (!ret) {
        std::cerr << "SetFlagValue <" << name << ", " << value << "> failed: "
                  << err << std::endl;
    }

    return ret;
}

#undef EXIT_ON_ERR
}  // namespace ccflag
