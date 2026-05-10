#pragma once

#include <cstddef>
#include <cmath>
#include <algorithm>
#include <vector>

namespace ArrayStats
{
    template <typename T>
    struct Stats
    {
        T min;
        T max;
        float mean;
        float median;
        float q1;      // First quartile
        float q3;      // Third quartile
        float std_dev; // Standard deviation
    };

    /**
     * @brief Return basic statistics (min, max, mean, median, quartiles, std deviation) of an array of values
     * @param array Input array of values
     * @param size Size of the input array
     * @return Stats struct containing the calculated statistics. If input is invalid (nullptr or size 0), all fields will be set to 0.
     * @note This function uses Welford's algorithm
     */
    template <typename T>
    Stats<T> GetStats(const T* array, std::size_t size)
    {
        if (array == nullptr || size == 0)
        {
            return Stats<T>{};
        }

        Stats<T> stats{};

        T current_min = array[0];
        T current_max = array[0];
        float mean = 0.0f;
        float M2 = 0.0f;

        for (std::size_t i = 0; i < size; ++i)
        {
            T val = array[i];
            if (val < current_min) current_min = val;
            if (val > current_max) current_max = val;

            float delta = static_cast<float>(val) - mean;
            mean += delta / static_cast<float>(i + 1);
            float delta2 = static_cast<float>(val) - mean;
            M2 += delta * delta2;
        }

        stats.min = current_min;
        stats.max = current_max;
        stats.mean = mean;
        
        if (size > 1)
        {
            stats.std_dev = std::sqrtf(M2 / static_cast<float>(size));
        } 
        else
        {
            stats.std_dev = 0.0f;
        }
        
        std::vector<T> sorted_array(array, array + size);
        std::sort(sorted_array.begin(), sorted_array.end());

        auto get_percentile = [&sorted_array, size](float p) -> float {
            float idx = p * static_cast<float>(size - 1);
            std::size_t lhs = static_cast<std::size_t>(std::floor(idx));
            std::size_t rhs = static_cast<std::size_t>(std::ceil(idx));
            if (lhs == rhs) {
                return static_cast<float>(sorted_array[lhs]);
            }
            float weight = idx - static_cast<float>(lhs);
            return (1.0f - weight) * static_cast<float>(sorted_array[lhs]) + weight * static_cast<float>(sorted_array[rhs]);
        };

        stats.median = get_percentile(0.5f);
        stats.q1 = get_percentile(0.25f);
        stats.q3 = get_percentile(0.75f);

        return stats;
    }
}