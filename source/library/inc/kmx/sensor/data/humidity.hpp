/// @copyright Copyright (c) 2025 - present KMX Systems. All rights reserved.
/// @file sensor/data/humidity.hpp
/// @brief Defines a class for representing humidity values.
#pragma once
#ifndef PCH
    #include <kmx/sensor/data/base.hpp>
#endif

namespace kmx::sensor::data
{
    /// @brief Traits for a humidity sensor.
    ///        Range: 0.0% to 100.0%, Resolution: 0.5%, Unit: Percent
    struct humidity_traits:
        traits<std::uint8_t, // storage_type
               float,        // input_type
               0.0f,         // min_value (0.0%)
               100.0f,       // max_value (100.0%)
               0.5f,         // resolution (0.5%)
               unit::percent // unit
               >
    {
    };

    /// @brief Represents a humidity value.
    ///        By default, it is in an undefined (empty optional) state.
    class humidity: public base<humidity_traits>
    {
    public:
        // Define an alias for the specific base class instantiation
        using sensor_base_type = base<humidity_traits>;

        // Inherit constructors from the base class.
        using sensor_base_type::base;
    };

} // namespace sensor::data
