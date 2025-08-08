#include "storage.h"

namespace aico
{
    inline storage<char> concat(const storage<char>& a, const storage<char>& b)
    {
        storage<char> result(a.size()+b.size());
        opres resA, resB;
        resA=a.copyinto(/*dst=*/result, /*n_elements=*/a.size());
        resB=b.copyinto(/*dst=*/result, /*n_elements=*/b.size(), 
            /*dst_startidx=*/a.size());
        if(resA!=opres::SUCCESS||resB!=opres::SUCCESS) return storage<char>();
        return result;
    }
    //TODO
    //I suppose I can add special 
    //if constexpr(std::is_same_v<T, char> into key constructors 
    //to make sure we allow for the '\0' at size() as a special 
    //invariant in storage<char>
    //well, that would corrupt storage<uint8_t>...
    inline storage<char> cstr(const storage<char>& str)
    {
        storage<char> result(str.size()+1);
        str.copyinto(result, str.size());
        result[str.size()]='\0';
        return result;
    }
}
