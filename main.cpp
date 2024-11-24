#include "app_info.hpp"

#include <iostream>
#include <thread>

int main()
{
    std::cout << AppInfo::startup_time_str("yyyy-mm-dd hh:nn:ss.zzz")   << std::endl;

    for (int i = 0; i < 10; i++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout   << AppInfo::elapsed_time<std::chrono::seconds>() << " seconds elapsed from "
                    << AppInfo::startup_time_str("yyyy-mm-dd hh:nn:ss.zzz")   << std::endl;
    }

    std::cout << AppInfo::total_memory() << " bytes total memory." << std::endl;
}