#include "aico/core/storage.h"
#include <cstddef>

namespace aico
{
    template <size_t size=DYNAMIC>
    class string
    {
    private:
        const storage<char, size> _data;
    public:
        string (const char (&lit) [size]) requires(size!=DYNAMIC): 
            _data(){}
    };
};
