/// @copyright Copyright (c) 2025 - present KMX Systems. All rights reserved.
/// @file sensor/data/light_intensity.hpp
/// @brief Defines a class for representing light intensity values.
#pragma once
#ifndef PCH
    #include <kmx/sensor/data/base.hpp>
#endif

namespace kmx::sensor::data
{
    /// @brief Traits for a light intensity sensor.
    ///        Range: 0.0 lux to 65535.0 lux, Resolution: 1.0 lux, Unit: Lux
    struct light_intensity_traits:
        traits<std::uint16_t, // storage_type
               float,         // input_type
               0.0f,          // min_value (0.0 lux)
               65535.0f,      // max_value (65535.0 lux)
               1.0f,          // resolution (1.0 lux)
               unit::lux      // unit
               >
    {
    };

    /// @brief Represents a light intensity value.
    ///        By default, it is in an undefined (empty optional) state.
    class light_intensity: public base<light_intensity_traits>
    {
    public:
        // Define an alias for the specific base class instantiation
        using sensor_base_type = base<light_intensity_traits>;

        // Inherit constructors from the base class.
        using sensor_base_type::base;
    };

} // namespace sensor::data
