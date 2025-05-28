/// @copyright Copyright (c) 2025 - present KMX Systems. All rights reserved.
/// @file sensor/data/base.hpp
/// @brief Defines an enumeration for sensor units and a conversion function to
/// string_view.
#pragma once
#ifndef PCH
    #include <algorithm>
    #include <array>
    #include <cmath>
    #include <concepts>
    #include <cstddef>
    #include <cstdint>
    #include <limits>
    #include <optional>
    #include <string_view>
#endif

namespace kmx
{
    /// @brief Enumeration of supported sensor units.
    enum class unit : std::uint8_t
    {
        celsius,          ///< Temperature in degrees Celsius (°C)
        percent,          ///< Relative humidity or other percentage (%)
        lux,              ///< Illuminance in lux (lx)
        pascal,           ///< Pressure in Pascals (Pa)
        volt,             ///< Electrical potential in Volts (V)
        ampere,           ///< Electrical current in Amperes (A)
        meter_per_second, ///< Speed in meters per second (m/s)
    };

    /// @brief Provides a string_view representation for a unit.
    /// @param unit The unit enum value.
    /// @return A std::string_view representing the unit, or "unknown" if the unit
    /// is out of bounds.
    [[nodiscard]] constexpr std::string_view text_of(const unit unit) noexcept
    {
        // The order must match the unit enum definition.
        static constexpr std::array<std::string_view,
                                    static_cast<std::size_t>(unit::meter_per_second) + 1u>
            items = {
                "°C",  // celsius
                "%",   // percent
                "lx",  // lux
                "Pa",  // pascal
                "V",   // volt
                "A",   // ampere
                "m/s", // meter_per_second
            };

        const auto index = static_cast<std::size_t>(unit);
        if (index < items.size())
            return items[index];

        return {};
    }
}

namespace kmx::sensor::data
{
    /// @brief A template struct to define the characteristics of a sensor type.
    /// @details This struct is used as a template parameter for the `sensor::data::base` class
    ///          to provide compile-time configuration for sensor properties such as storage type,
    ///          input type, valid range, resolution, and unit.
    /// @tparam _Storage The underlying integer type used to store the scaled sensor value (e.g., std::int16_t).
    ///           Must satisfy `std::integral`.
    /// @tparam _Input The floating-point type used for the physical sensor value (e.g., float, double).
    ///           Must satisfy `std::floating_point`.
    /// @tparam param_min_value The minimum representable physical value for the sensor.
    /// @tparam param_max_value The maximum representable physical value for the sensor.
    /// @tparam param_resolution The resolution (smallest representable change) of the physical sensor value.
    /// @tparam param_unit The physical unit of the sensor's value, from the `unit` enum.
    template <std::integral _Storage, std::floating_point _Input, _Input param_min_value, _Input param_max_value, _Input param_resolution,
              kmx::unit param_unit>
    struct traits
    {
        /// @brief The underlying integer type for storing the scaled sensor value.
        using storage_type = _Storage;
        /// @brief The floating-point type for representing the sensor's physical value.
        using input_type = _Input;
        /// @brief The unit type for representing the sensor's units.
        using unit_type = kmx::unit;

        /// @brief The minimum representable physical value for this sensor type.
        static constexpr input_type min_value = param_min_value;
        /// @brief The maximum representable physical value for this sensor type.
        static constexpr input_type max_value = param_max_value;
        /// @brief The resolution (smallest representable change) of the sensor's physical value.
        static constexpr input_type resolution = param_resolution;
        /// @brief The physical unit of the sensor's value.
        static constexpr unit_type unit = param_unit;

        // Compile-time assertions for trait validity
        static_assert(resolution > static_cast<input_type>(0.0), "Resolution must be positive.");
        static_assert(min_value <= max_value, "Min value must be less than or equal to max value.");
    };

    /// @brief A template base class for representing and managing scaled sensor values.
    ///        Sensor characteristics are provided via a Traits template parameter.
    ///        The internal scaled value is stored in a std::optional.
    /// @tparam traits A struct satisfying the traits concept, defining storage_type, input_type,
    ///                min_value, max_value, resolution, and unit.
    template <typename traits>
    class base
    {
    public:
        using storage_type = typename traits::storage_type;
        using input_type = typename traits::input_type;
        using traits_type = traits; // Expose the traits type

        /// @brief Default constructor. Initializes the sensor value to an undefined state (empty optional).
        constexpr base() noexcept: scaled_value_ {} {} // Initializes to std::nullopt

        /// @brief Constructor to initialize with a specific value.
        /// @param initial_value The initial value to set. The value will be clamped and quantized.
        ///                      The sensor will have a defined value after this constructor.
        constexpr explicit base(const input_type initial_value) noexcept:
            scaled_value_ {convert_to_scaled(std::clamp(initial_value, traits_type::min_value, traits_type::max_value))}
        {
        }

        /// @brief Checks if the sensor currently holds a defined value.
        /// @return True if the sensor has a value, false otherwise.
        [[nodiscard]] constexpr explicit operator bool() const noexcept { return scaled_value_.has_value(); }

        /// @brief Gets the sensor value as a floating-point number, if defined.
        /// @return An std::optional containing the current value in its unit if defined,
        ///         otherwise an empty optional.
        [[nodiscard]] constexpr std::optional<input_type> value() const noexcept
        {
            if (!scaled_value_) // Uses operator bool()
                return {};      // Returns empty optional (std::nullopt)
            return convert_to_physical(scaled_value_.value());
        }

        /// @brief Sets the sensor value from a floating-point number, making it defined.
        ///        The value will be clamped to the sensor's valid range and quantized to its resolution.
        /// @param new_value The new value to set, in its unit.
        /// @return True if the new_value was within the sensor's min/max range (no significant clamping needed),
        ///         false if clamping occurred. The value is always updated to the (potentially clamped) new_value.
        [[nodiscard]] constexpr bool set_value(const input_type new_value) noexcept
        {
            const input_type clamped_value = std::clamp(new_value, traits_type::min_value, traits_type::max_value);
            scaled_value_ = convert_to_scaled(clamped_value);

            const input_type difference = std::abs(new_value - clamped_value);
            constexpr input_type epsilon_limit = std::numeric_limits<input_type>::epsilon() * static_cast<input_type>(100.0);
            return difference < epsilon_limit;
        }

        /// @brief Gets the raw scaled integer value used for storage/transmission, if defined.
        /// @return An std::optional containing the underlying scaled integer representation if defined,
        ///         otherwise an empty optional.
        [[nodiscard]] constexpr std::optional<storage_type> raw_scaled_value() const noexcept { return scaled_value_; }

        /// @brief Sets the sensor value from a raw scaled integer, making it defined if valid.
        ///        The raw value will be validated against the possible range of scaled values for this sensor type.
        /// @param raw_val The raw scaled integer value.
        /// @return True if the raw value is valid for this sensor type and was set, false otherwise.
        ///         If false, the sensor's defined state remains unchanged.
        [[nodiscard]] constexpr bool set_raw_scaled_value(const storage_type raw_val) noexcept
        {
            if ((raw_val < static_min_scaled_value) || (raw_val > static_max_scaled_value))
                return false;

            scaled_value_ = raw_val;
            return true;
        }

        /// @brief Clears the sensor value, making it undefined.
        constexpr void clear() noexcept { scaled_value_.reset(); }

        /// @brief Gets the minimum representable value for this sensor type.
        /// @return The minimum value.
        [[nodiscard]] static constexpr input_type min_value() noexcept { return traits_type::min_value; }

        /// @brief Gets the maximum representable value for this sensor type.
        /// @return The maximum value.
        [[nodiscard]] static constexpr input_type max_value() noexcept { return traits_type::max_value; }

        /// @brief Gets the resolution (smallest representable change) of the value.
        /// @return The resolution.
        [[nodiscard]] static constexpr input_type resolution() noexcept { return traits_type::resolution; }

        /// @brief Gets the unit of measurement for this sensor type.
        /// @return The unit enum value.
        [[nodiscard]] static constexpr unit unit() noexcept { return traits_type::unit; }

        /// @brief Gets the string representation of the unit of measurement for this sensor type.
        /// @return A std::string_view representing the unit.
        [[nodiscard]] static constexpr std::string_view unit_string() noexcept { return to_string_view(traits_type::unit); }

        /// @brief Gets the minimum storable scaled integer value.
        [[nodiscard]] static constexpr storage_type min_scaled_storage_value() noexcept { return static_min_scaled_value; }

        /// @brief Gets the maximum storable scaled integer value.
        [[nodiscard]] static constexpr storage_type max_scaled_storage_value() noexcept { return static_max_scaled_value; }

    protected:
        /// @brief The internally stored scaled sensor value.
        /// @details This optional member holds the sensor's value in its scaled, integer representation.
        ///          If the optional is empty (std::nullopt), the sensor's value is considered undefined.
        ///          Otherwise, it contains the current scaled value.
        std::optional<storage_type> scaled_value_;

        /// @brief The numerator used in calculating the scaling factor. Typically 1.0.
        /// @details This constant, along with `traits_type::resolution`, determines the `scale_factor`.
        ///          It's defined as a separate constant for potential future flexibility or clarity in the scaling formula.
        static constexpr input_type scale_numerator = static_cast<input_type>(1.0);

        /// @brief The scaling factor used to convert between physical and scaled values.
        /// @details Calculated as `scale_numerator / traits_type::resolution`.
        ///          Multiplying a physical value by this factor yields the unrounded scaled value.
        ///          Dividing a scaled value (cast to input_type) by this factor yields the physical value.
        static constexpr input_type scale_factor = scale_numerator / traits_type::resolution;

        /// @brief Converts a physical value (which should already be clamped) to its scaled integer representation.
        /// @details This static constexpr method performs the scaling by multiplying with `scale_factor`,
        ///          rounds the result to the nearest integer, and then casts it to `storage_type`.
        ///          It assumes the input `val` is already within the sensor's valid physical range
        ///          (i.e., between `traits_type::min_value` and `traits_type::max_value`).
        /// @param val The physical value to convert (must be pre-clamped).
        /// @return The corresponding scaled integer value.
        [[nodiscard]] static constexpr storage_type convert_to_scaled(const input_type val) noexcept
        {
            const input_type scaled_float = val * scale_factor;
            return static_cast<storage_type>(std::round(scaled_float));
        }

        /// @brief Converts a scaled integer value back to its physical floating-point representation.
        /// @details This static constexpr method performs the conversion by casting the scaled `val`
        ///          to `input_type` and then dividing by `scale_factor`.
        /// @param val The scaled integer value.
        /// @return The corresponding physical value.
        [[nodiscard]] static constexpr input_type convert_to_physical(const storage_type val) noexcept
        {
            const auto scaled_val_as_input = static_cast<input_type>(val);
            return scaled_val_as_input / scale_factor;
        }

        /// @brief The minimum valid scaled integer value, pre-calculated at compile time.
        /// @details This is derived by converting `traits_type::min_value` using `convert_to_scaled`.
        ///          It's used for validating raw scaled input.
        static constexpr storage_type static_min_scaled_value = convert_to_scaled(traits_type::min_value);

        /// @brief The maximum valid scaled integer value, pre-calculated at compile time.
        /// @details This is derived by converting `traits_type::max_value` using `convert_to_scaled`.
        ///          It's used for validating raw scaled input.
        static constexpr storage_type static_max_scaled_value = convert_to_scaled(traits_type::max_value);
    };

} // namespace sensor::data
