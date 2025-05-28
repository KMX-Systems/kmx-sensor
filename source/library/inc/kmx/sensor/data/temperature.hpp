/// @copyright Copyright (c) 2025 - present KMX Systems. All rights reserved.
/// @file sensor/data/temperature.hpp
/// @brief Defines a class for representing temperature values.
#pragma once
#ifndef PCH
    #include <kmx/sensor/data/base.hpp>
#endif

namespace kmx::sensor::data
{
    /// @brief Traits for a temperature sensor.
    ///        Range: -50.0°C to +50.0°C, Resolution: 0.1°C, Unit: Celsius
    struct temperature_traits:
        traits<std::int16_t, // storage_type
               float,        // input_type
               -50.0f,       // min_value (-50.0°C)
               50.0f,        // max_value (+50.0°C)
               0.1f,         // resolution (0.1°C)
               unit::celsius // unit
               >
    {
    };

    /// @brief Represents a temperature value.
    ///        By default, it is in an undefined (empty optional) state.
    class temperature: public base<temperature_traits>
    {
    public:
        // Define an alias for the specific base class instantiation
        using sensor_base_type = base<temperature_traits>;

        // Inherit constructors from the base class.
        using sensor_base_type::base;
    };

} // namespace sensor::data
