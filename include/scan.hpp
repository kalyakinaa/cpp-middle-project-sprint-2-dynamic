#pragma once

#include <expected>
#include <string_view>
#include <tuple>
#include <utility>

#include "parse.hpp"
#include "types.hpp"

namespace stdx {

namespace details {

template <typename... Ts, std::size_t... Is>
std::expected<scan_result<Ts...>, scan_error> aggregate_impl(const std::vector<std::string_view> &format_parts,
                                                             const std::vector<std::string_view> &input_parts,
                                                             std::index_sequence<Is...>) {

    std::tuple<remove_cvref_t<Ts>...> result{};
    std::expected<void, scan_error> status{};

    auto process_one = [&]<std::size_t I>() {
        if (!status)
            return;

        using ParseT = std::tuple_element_t<I, std::tuple<Ts...>>;
        auto res = parse_value_with_format<ParseT>(input_parts[I], format_parts[I]);
        if (!res) {
            status = std::unexpected(res.error());
        } else {
            std::get<I>(result) = std::move(*res);
        }
    };

    (process_one.template operator()<Is>(), ...);

    if (!status) {
        return std::unexpected(status.error());
    }
    return scan_result<Ts...>{std::move(result)};
}

}  // namespace details

template <typename... Ts>
std::expected<details::scan_result<Ts...>, details::scan_error> scan(std::string_view input, std::string_view format) {
    static_assert(sizeof...(Ts) > 0, "scan: at least one type is required");
    static_assert(details::AllSupported<Ts...>, "scan: unsupported type. Allowed: int8_t..int64_t, "
                                                "uint8_t..uint64_t, float, double, std::string, std::string_view");
    static_assert(details::NoReferences<Ts...>, "scan: reference types are not allowed");

    auto sources = details::parse_sources<Ts...>(input, format);
    if (!sources) {
        return std::unexpected(sources.error());
    }

    auto &[format_parts, input_parts] = *sources;

    if (format_parts.size() != sizeof...(Ts)) {
        return std::unexpected(details::scan_error{"Number of placeholders does not match number of types"});
    }
    if (input_parts.size() != sizeof...(Ts)) {
        return std::unexpected(details::scan_error{"Number of input parts does not match number of types"});
    }

    return details::aggregate_impl<Ts...>(format_parts, input_parts, std::index_sequence_for<Ts...>{});
}

}  // namespace stdx
