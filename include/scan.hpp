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
[[nodiscard]] std::expected<scan_result<Ts...>, scan_error>
aggregate_impl(const std::vector<std::string_view> &format_parts, const std::vector<std::string_view> &input_parts,
               std::index_sequence<Is...>) {

    std::tuple<remove_cvref_t<Ts>...> result{};
    std::expected<void, scan_error> status{};

    auto process_one = [&]<std::size_t I>() -> bool {
        using ParseT = std::tuple_element_t<I, std::tuple<Ts...>>;
        auto res = parse_value_with_format<ParseT>(input_parts[I], format_parts[I]);
        if (!res) {
            status = std::unexpected(std::move(res).error());
            return false;
        }
        std::get<I>(result) = std::move(*res);
        return true;
    };

    if (!(process_one.template operator()<Is>() && ...)) {
        return std::unexpected(std::move(status).error());
    }

    return scan_result<Ts...>{std::tuple<Ts...>{std::get<Is>(std::move(result))...}};
}

}  // namespace details

template <typename... Ts>
[[nodiscard]] std::expected<details::scan_result<Ts...>, details::scan_error> scan(std::string_view input,
                                                                                   std::string_view format) {
    static_assert(sizeof...(Ts) > 0, "scan: at least one type is required");

    if constexpr (!details::AllSupported<Ts...> || !details::NoReferences<Ts...>) {
        return std::unexpected(details::scan_error{"scan: unsupported type or reference type"});
    } else {
        auto sources = details::parse_sources<Ts...>(input, format);
        if (!sources) {
            return std::unexpected(std::move(sources).error());
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
}

}  // namespace stdx
