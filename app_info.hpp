#ifndef APP_INFO_HPP
#define APP_INFO_HPP

#include <chrono>
#include <string>
#include <cstdint>
#include <mutex>
#include <sstream>
#include <stdexcept>

#ifdef __unix__
#include <unistd.h>
#include <limits.h>
#else
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#else
#include <windows.h>
#endif
#include <fileapi.h>
#endif

class AppInfo
{

using time_point = std::chrono::system_clock::time_point;

public:

    time_point  startup_time;
    std::string app_path;

    static const AppInfo& get_instance()
    {
        static const AppInfo app_info;
        return app_info;
    }

    static constexpr bool is_little_endian()
    {
        int32_t i = 0x12345678;
        return (*(char*)&i == 0x78);
    }

    static constexpr bool is_big_endian()
    {
        int32_t i = 0x12345678;
        return (*(char*)&i == 0x12);
    }

    static constexpr bool is_unix()
    {
#ifdef __unix__
        return true;
#endif
        return false;
    }

    static constexpr bool is_windows()
    {
#if defined(_WIN32) || defined(_WIN64)
        return true;
#endif
        return false;
    }

    static uint32_t total_memory()
    {
#ifdef __unix__
        return sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE);
#else
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        GlobalMemoryStatusEx(&status);
        return status.ullTotalPhys;
#endif
    }

    template<typename T = std::chrono::milliseconds>
    static int elapsed_time(const time_point& tp = std::chrono::system_clock::now())
    {
        return std::chrono::duration_cast<T>(tp - get_instance().startup_time).count();
    }

    static std::string startup_time_str(const std::string& format)
    {
        static std::recursive_mutex mtx;
        auto dump = [](std::string& str, const char* fmt, const std::string& rep)
        {
            std::string::size_type pos = 0;
            int fmt_size = std::string(fmt).size();
            while(true)
            {
                pos = str.find(fmt, pos);
                if(pos == std::string::npos)
                    break;
                str = str.substr(0, pos) + rep + str.substr(pos + fmt_size);
                pos = pos + rep.size();
            }
        };

        auto fill = [](const std::string& str, const std::size_t& digit)
        {
            std::string result(str);
            int remain = digit > str.size() ? digit - str.size() : 0;
            while(remain--)
                result = '0' + result;
            return result;
        };

        auto tp_value   = get_instance().startup_time;
        auto tt_value   = std::chrono::system_clock::to_time_t(tp_value);
        mtx.lock();
        auto tm_ptr     = std::gmtime(&tt_value);
        auto tm_value   = *tm_ptr;
        mtx.unlock();
        int usec_epoch  = std::chrono::duration_cast<std::chrono::microseconds>(tp_value.time_since_epoch()).count();
        int sec_epoch   = 1000000 * std::chrono::duration_cast<std::chrono::seconds>(tp_value.time_since_epoch()).count();
        int usec        = usec_epoch - sec_epoch;

        std::string result = format;
        dump(result,   "yyyy", std::to_string(tm_value.tm_year + 1900));
        dump(result,     "mm", fill(std::to_string(tm_value.tm_mon + 1), 2));
        dump(result,     "dd", fill(std::to_string(tm_value.tm_mday), 2));
        dump(result,     "hh", fill(std::to_string(tm_value.tm_hour), 2));
        dump(result,     "nn", fill(std::to_string(tm_value.tm_min), 2));
        dump(result,     "ss", fill(std::to_string(tm_value.tm_sec), 2));
        dump(result, "zzzzzz", fill(std::to_string(usec), 6));
        dump(result,    "zzz", fill(std::to_string(usec / 1000), 3));
        return result;
    }

private:

    explicit AppInfo()
        : startup_time(std::chrono::system_clock::now())
        , app_path(check_app_path())
    {}

    static std::string  check_app_path()
    {
#ifdef __unix__
        char tmp[PATH_MAX];
        if (readlink("/proc/self/exe", tmp, sizeof(tmp)-1) == -1)
        {
            std::stringstream ss;
            ss << "Internal error in " << __func__ << " : " << errno;
            throw std::runtime_error(ss.str());
        }
        return std::string(tmp);
#else
        char tmp[MAX_PATH];
        GetModuleFileNameA(NULL, tmp, MAX_PATH);
        return std::string(tmp);
#endif
    }
    
};

#endif // APP_INFO_HPP