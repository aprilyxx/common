#pragma once

#include <stdint.h>
#include <string>
#include <vector>

namespace ccflag {
namespace std = ::std;

/*
 * parse command line flags from <argc, argv>.
 * non-flag elements will be put into the vector, if v != NULL.
 */
void init_ccflag(int argc, char** argv, std::vector<std::string>* v = NULL);

/*
 * parse command line flags from a string.
 * non-flag elements will be put into the vector, if v != NULL.
 *
 * <usage>  init_ccflag("-i=23 -s=\"hello world\" -t=hello_world");
 */
void init_ccflag(const std::string& args, std::vector<std::string>* v = NULL);

/*
 * parse command line flags from <argc, argv> first, and then from config file.
 */
void init_ccflag(int argc, char** argv, const std::string& config);

/*
 * set value of a flag by name, return false if flag not found or value invalid.
 *
 * <usage>  SetFlagValue("boo", "true"); // --boo=true.
 */
bool SetFlagValue(const std::string& name, const std::string& value);

namespace xx {
namespace std = ::std;
using std::string;

typedef ::int32_t int32;
typedef ::int64_t int64;
typedef ::uint32_t uint32;
typedef ::uint64_t uint64;

enum {
    TYPE_bool,
    TYPE_int32,
    TYPE_int64,
    TYPE_uint32,
    TYPE_uint64,
    TYPE_string,
    TYPE_double
};

struct FlagSaver {
    FlagSaver(const char* type_str, const char* name, const char* value,
              const char* help, const char* file, void* addr, int type);
};
}  // namespace xx
}  // namespace ccflag

#define DECLARE_CCFLAG(type, name) \
    namespace ccflag { \
    namespace zz { \
        using namespace ::ccflag::xx; \
        extern type FLG_##name; \
    } \
    } \
    using ccflag::zz::FLG_##name

#define DEC_bool(name)    DECLARE_CCFLAG(bool, name)
#define DEC_int32(name)   DECLARE_CCFLAG(int32, name)
#define DEC_int64(name)   DECLARE_CCFLAG(int64, name)
#define DEC_uint32(name)  DECLARE_CCFLAG(uint32, name)
#define DEC_uint64(name)  DECLARE_CCFLAG(uint64, name)
#define DEC_string(name)  DECLARE_CCFLAG(string, name)
#define DEC_double(name)  DECLARE_CCFLAG(double, name)

#define DEFINE_CCFLAG(type, name, value, help) \
    namespace ccflag { \
    namespace zz { \
        using namespace ::ccflag::xx; \
        type FLG_##name = value; \
        static ::ccflag::xx::FlagSaver kFs_##name( \
            #type, #name, #value, help, __FILE__, &FLG_##name, \
            ::ccflag::xx::TYPE_##type \
        ); \
    } \
    } \
    using ccflag::zz::FLG_##name

#define DEF_bool(name, value, help)    DEFINE_CCFLAG(bool, name, value, help)
#define DEF_int32(name, value, help)   DEFINE_CCFLAG(int32, name, value, help)
#define DEF_int64(name, value, help)   DEFINE_CCFLAG(int64, name, value, help)
#define DEF_uint32(name, value, help)  DEFINE_CCFLAG(uint32, name, value, help)
#define DEF_uint64(name, value, help)  DEFINE_CCFLAG(uint64, name, value, help)
#define DEF_string(name, value, help)  DEFINE_CCFLAG(string, name, value, help)
#define DEF_double(name, value, help)  DEFINE_CCFLAG(double, name, value, help)
