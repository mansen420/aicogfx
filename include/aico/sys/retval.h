#include <type_traits>
#include <utility>
namespace aico
{   
    //TODO this should be a class, not a struct
    template<typename T, typename E>
    struct retval
    {
    private:
        struct _unexpected_t{};
        template<typename...Args>
        requires requires{E(std::declval<Args>()...);} //requires constructible
        explicit retval(_unexpected_t, Args&&...args)
        {
            new (&err) E(std::forward<Args>(args)...);
            bad=true;
        }
    public:
        union
        {
            T val;
            E err;
        };
        bool bad;

        template<typename...Args>
        requires requires{T(std::declval<Args>()...);} //requires constructible
        retval(Args&&...args)
        {
            new (&val) T(std::forward<Args>(args)...);
            bad=false;
        }
        template<typename...Args>
        static retval failure(Args&&... args) 
        {
            return retval(_unexpected_t{}, std::forward<Args>(args)...);
        }

        retval()=delete;

        retval(const retval& other)noexcept
        requires(std::is_trivially_copyable_v<T>&&std::is_trivially_copyable_v<E>)
            =default;
        
        retval(const retval& other)
        noexcept(std::is_nothrow_copy_constructible_v<T>&&
                 std::is_nothrow_copy_constructible_v<E>)
        requires(std::is_copy_constructible_v<T>&&std::is_copy_constructible_v<E>)
        {
            bad=other.bad;
            if(!bad)//TODO catch throws here
                new (&val) T(other.val);
            else
                new (&err) E(other.err);
        }
        retval& operator=(const retval& other)noexcept
        requires(std::is_trivially_copy_assignable_v<T>&&
                 std::is_trivially_copy_assignable_v<E>)
            =default;
        
        retval& operator=(const retval& other)
        noexcept(std::is_nothrow_copy_assignable_v<T>&&
                 std::is_nothrow_copy_assignable_v<E>&&noexcept(_destroy()))
        requires(std::is_copy_assignable_v<T>&&std::is_copy_assignable_v<E>)
        {
            if(this==&other) return *this;
            if(!this->bad)//-->T active
                if(!other.bad)
                    this->val=other.val;
                else
                {
                    _destroy();//destroy T
                    new (&err) E(other.err);//copy constrct E
                }
            else//-->E active
                if(!other.bad)
                {
                    _destroy();//destroy E
                    new (&val) T(other.val);
                }
                else
                    this->err=other.err;
            this->bad=other.bad;
            return *this;
        }
            
        retval(retval&& other)
        requires(std::is_trivially_move_constructible_v<T>&&
                 std::is_trivially_move_constructible_v<E>)
        =default;
        
        retval(retval&& other)
        noexcept(std::is_nothrow_move_constructible_v<T>&&
                 std::is_nothrow_move_constructible_v<E>)
        requires(std::is_move_constructible_v<T>&&std::is_move_constructible_v<E>)
        {
            bad=other.bad;
            if(!bad)
                new (&val) T(std::move(other.val));
            else
                new (&err) E(std::move(other.err));
        }
        
        retval& operator=(retval&& other)noexcept
        requires(std::is_trivially_move_assignable_v<T>&&
                 std::is_trivially_move_assignable_v<E>)
            =default;
        
        retval& operator=(retval&& other)
        noexcept(std::is_nothrow_move_assignable_v<T>&&
                 std::is_nothrow_move_assignable_v<E>&&noexcept(_destroy()))
        requires(std::is_move_assignable_v<T>&&std::is_move_assignable_v<E>)
        {
            if(this==&other) return *this;
            if(!this->bad)//-->T active
                if(!other.bad)
                    this->val=std::move(other.val);
                else
                {
                    _destroy();//destroy T
                    new (&err) E(std::move(other.err));//move constrct E
                }
            else//-->E active
                if(!other.bad)
                {
                    _destroy();//destroy E
                    new (&val) T(std::move(other.val));
                }
                else
                    this->err=std::move(other.err);
            this->bad=other.bad;
            return *this;
        }

        //value opertors assume !bad

        inline T& operator*()noexcept{return val;}
        inline const T& operator*()const noexcept{return val;}
        
        inline T* operator->()noexcept { return &val; }
        inline const T* operator->()const noexcept { return &val; }
        
        explicit inline operator T*()noexcept{return &val;}
        explicit inline operator const T*()const noexcept{return &val;}

        explicit inline operator bool()const noexcept{return !bad;}

        ~retval()
        {
            _destroy();
        }
    private:
        void _destroy()
            noexcept(std::is_nothrow_destructible_v<T>&&std::is_nothrow_destructible_v<E>)
        {
            if constexpr(!std::is_trivially_destructible_v<T>)if(!bad) 
            {
                val.~T();
                return;
            }
            if constexpr(!std::is_trivially_destructible_v<E>)if(bad)
                err.~E();
        }
    };
};
