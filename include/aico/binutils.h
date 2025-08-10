#include <cstdint>
#include <cstddef>
#include <type_traits>

namespace aico
{
    template<typename Res_t, typename Lo_t, typename Hi_t>
        requires(
            std::is_integral_v<Res_t>&&
            std::is_integral_v<Lo_t>&&
            std::is_integral_v<Hi_t>&&
            sizeof(Res_t)==sizeof(Lo_t)+sizeof(Hi_t)&&
            std::is_unsigned_v<Res_t>&&
            std::is_unsigned_v<Lo_t>&&
            std::is_unsigned_v<Hi_t>)
    [[nodiscard]]inline constexpr Res_t
    concat(Lo_t lo, Hi_t hi)noexcept
    {
        constexpr size_t Nlobits=sizeof(Lo_t)*8;
        constexpr Res_t 
            lomask=(Res_t(1)<<Nlobits)-1,//-->0x[0..00]F..FF
            himask=~lomask;//-->0xF..FF[0..00]

        return Res_t(((Res_t(hi)<<Nlobits)&himask)|(Res_t(lo)&lomask));
    }
    template<typename From_t, typename Extract_t>
        requires(
                std::is_unsigned_v<From_t>&&
                std::is_unsigned_v<Extract_t>&&
                sizeof(Extract_t)<sizeof(From_t))
    [[nodiscard]]inline constexpr Extract_t
    lo(From_t x)noexcept
    {
        constexpr size_t Nbits=sizeof(Extract_t)*8;
        constexpr From_t mask=(From_t(1)<<Nbits)-1;//-->low Nbits set
        return (Extract_t)(x&mask);
    }
    template<typename From_t, typename Extract_t>
        requires(
                std::is_unsigned_v<From_t>&&
                std::is_unsigned_v<Extract_t>&&
                sizeof(Extract_t)<sizeof(From_t))
    [[nodiscard]]inline constexpr Extract_t
    hi(From_t x)noexcept
    {
        constexpr size_t shift=sizeof(From_t)*8-sizeof(Extract_t)*8;
        return Extract_t(x>>shift);
    }
    
    template<typename From_t>
    [[nodiscard]]inline constexpr uint8_t
    lo8(From_t x)noexcept{return lo<From_t, uint8_t>(x);}
    template<typename From_t>
    [[nodiscard]]inline constexpr uint16_t
    lo16(From_t x)noexcept{return lo<From_t, uint16_t>(x);}
    template<typename From_t>
    [[nodiscard]]inline constexpr uint32_t
    lo32(From_t x)noexcept{return lo<From_t, uint32_t>(x);}
     
    template<typename From_t>
    [[nodiscard]]inline constexpr uint8_t
    hi8(From_t x)noexcept{return hi<From_t, uint8_t>(x);}
    template<typename From_t>
    [[nodiscard]]inline constexpr uint16_t
    hi16(From_t x)noexcept{return hi<From_t, uint16_t>(x);}
    template<typename From_t>
    [[nodiscard]]inline constexpr uint32_t
    hi32(From_t x)noexcept{return hi<From_t, uint32_t>(x);}   
    
    [[nodiscard]]inline constexpr uint64_t
    concat(uint32_t lo, uint32_t hi)noexcept
    {
        return concat<uint64_t>(lo, hi);
    }
    [[nodiscard]]inline constexpr uint32_t
    concat(uint16_t lo, uint16_t hi)noexcept
    {
        return concat<uint32_t>(lo, hi);
    }
    [[nodiscard]]inline constexpr uint16_t
    concat(uint8_t lo, uint8_t hi)noexcept
    {
        return concat<uint16_t>(lo, hi);
    }
};
