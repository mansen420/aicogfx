#include <ctime>
#include <cstdint>

namespace aico
{
    //TODO migrate all definitions to .cpp
    inline uint64_t sys_time() noexcept 
    {
      timespec ts{}; clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
      return uint64_t(ts.tv_sec)*1000000000ull + uint64_t(ts.tv_nsec);
    }
    inline uint64_t start_ns = sys_time(); // captured at static init
    inline uint64_t aico_time() noexcept { return sys_time()-start_ns;}
}
