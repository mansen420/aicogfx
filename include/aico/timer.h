#include <chrono>

namespace aico 
{
    //GPT wrote a better version of my micro_timer
    template<class D, class Clock = std::chrono::steady_clock>
    class timer 
    {
        static_assert(std::chrono::treat_as_floating_point<typename D::rep>::value ||
                      std::is_integral_v<typename D::rep>,
                      "D must be a std::chrono::duration");

    public:
        using duration   = D;
        using clock      = Clock;
        using time_point = typename Clock::time_point;

        timer() : _last(clock::now()), _acc(duration::zero()) {}

        // time accumulated since construction/reset (doesn't modify state)
        [[nodiscard]] duration time_since_start() const 
        {
            const auto now = clock::now();
            return _acc + std::chrono::duration_cast<duration>(now - _last);
        }

        // advance the timer and return the delta since last tick
        [[nodiscard]] duration tick() 
        {
            const auto now = clock::now();
            const auto dt  = std::chrono::duration_cast<duration>(now - _last);
            _last = now;
            _acc += dt;
            return dt;
        }

        // reset accumulated time to zero and restart
        void reset() 
        {
            _acc  = duration::zero();
            _last = clock::now();
        }

    private:
        time_point _last{clock::now()};
        duration   _acc{};
    };

    // Handy aliases
    using nano_timer  = timer<std::chrono::nanoseconds>;
    using micro_timer = timer<std::chrono::microseconds>;
    using milli_timer = timer<std::chrono::milliseconds>;

} // namespace aico

