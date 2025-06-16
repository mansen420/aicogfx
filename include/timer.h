#include <chrono>
namespace aico
{
    class micro_timer
    {
    public:
        [[nodiscard]] std::chrono::microseconds time_since_start() const
        {
            return acc + std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - lastUpdate);
        }
        [[nodiscard]] std::chrono::microseconds clock()
        {
            auto temp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - lastUpdate);
            lastUpdate = std::chrono::high_resolution_clock::now();
            acc += temp;
            return temp;
        }
    private:
        std::chrono::high_resolution_clock::time_point lastUpdate = std::chrono::high_resolution_clock::now();
        std::chrono::microseconds acc{};
    };
}
