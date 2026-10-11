#pragma once

#include <cstdint>
#include <string>
#include <tuple>
#include <type_traits>

namespace stdx::details {

struct scan_error {
    std::string message;
};

template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename... Ts>
class scan_result {
public:
    using storage_type = std::tuple<remove_cvref_t<Ts>...>;

    explicit scan_result(storage_type values) : values_(std::move(values)) {}

    const storage_type &values() const noexcept { return values_; }
    storage_type &values() noexcept { return values_; }

private:
    storage_type values_;
};

template <typename T>
concept SupportedType =
    std::same_as<remove_cvref_t<T>, int8_t> || std::same_as<remove_cvref_t<T>, int16_t> ||
    std::same_as<remove_cvref_t<T>, int32_t> || std::same_as<remove_cvref_t<T>, int64_t> ||
    std::same_as<remove_cvref_t<T>, uint8_t> || std::same_as<remove_cvref_t<T>, uint16_t> ||
    std::same_as<remove_cvref_t<T>, uint32_t> || std::same_as<remove_cvref_t<T>, uint64_t> ||
    std::same_as<remove_cvref_t<T>, float> || std::same_as<remove_cvref_t<T>, double> ||
    std::same_as<remove_cvref_t<T>, std::string_view> || std::same_as<remove_cvref_t<T>, std::string>;

template <typename T>
concept NotReference = !std::is_reference_v<T>;

template <typename... Ts>
concept AllSupported = (SupportedType<Ts> && ...);

template <typename... Ts>
concept NoReferences = (NotReference<Ts> && ...);

}  // namespace stdx::details
