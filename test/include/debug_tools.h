#ifndef __DEBUG_TOOLS_H__
#define __DEBUG_TOOLS_H__

#include <stdexcept>
#include <sstream>

#ifdef __GNUC__
    #define FUNCTION_INFO __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
    #define FUNCTION_INFO __FUNCSIG__
#else
    #define FUNCTION_INFO __func__
#endif

#define MY_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::ostringstream oss; \
            oss << "[" << FUNCTION_INFO << " " << __LINE__  << "] "  << message; \
            throw std::runtime_error(oss.str()); \
        } \
    } while (0)

#endif // __DEBUG_TOOLS_H__