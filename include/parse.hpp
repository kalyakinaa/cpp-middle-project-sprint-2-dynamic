#pragma once

#include <cctype>
#include <charconv>
#include <expected>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

#include "types.hpp"

namespace stdx::details {

[[nodiscard]] constexpr std::string_view trim(std::string_view s) noexcept {
    std::size_t b = 0;
    std::size_t e = s.size();

    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) {
        ++b;
    }
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) {
        --e;
    }
    return s.substr(b, e - b);
}

template <StringLike T>
[[nodiscard]] std::expected<T, scan_error> parse_value(std::string_view input, char spec) {
    if (spec != 's') {
        return std::unexpected(scan_error{"String type requires format specifier 's'"});
    }
    if constexpr (std::is_same_v<remove_cvref_t<T>, std::string>) {
        return std::string(input);
    } else {
        return input;
    }
}

template <SignedInteger T>
[[nodiscard]] std::expected<T, scan_error> parse_value(std::string_view input, char spec) {
    if (spec != 'd') {
        return std::unexpected(scan_error{"Signed integer type requires 'd' specifier"});
    }
    using CleanT = remove_cvref_t<T>;

    long long wide{};
    auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), wide);
    if (ec != std::errc{} || ptr != input.data() + input.size()) {
        return std::unexpected(scan_error{"Failed to parse integer"});
    }
    if (wide < static_cast<long long>(std::numeric_limits<CleanT>::min()) ||
        wide > static_cast<long long>(std::numeric_limits<CleanT>::max())) {
        return std::unexpected(scan_error{"Integer out of range"});
    }
    return static_cast<CleanT>(wide);
}

template <UnsignedInteger T>
[[nodiscard]] std::expected<T, scan_error> parse_value(std::string_view input, char spec) {
    if (spec != 'u') {
        return std::unexpected(scan_error{"Unsigned integer type requires 'u' specifier"});
    }
    using CleanT = remove_cvref_t<T>;

    unsigned long long wide{};
    auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), wide);
    if (ec != std::errc{} || ptr != input.data() + input.size()) {
        return std::unexpected(scan_error{"Failed to parse integer"});
    }
    if (wide > static_cast<unsigned long long>(std::numeric_limits<CleanT>::max())) {
        return std::unexpected(scan_error{"Integer out of range"});
    }
    return static_cast<CleanT>(wide);
}

template <FloatingPoint T>
[[nodiscard]] std::expected<T, scan_error> parse_value(std::string_view input, char spec) {
    if (spec != 'f') {
        return std::unexpected(scan_error{"Floating point type requires 'f' specifier"});
    }
    using CleanT = remove_cvref_t<T>;

    CleanT value{};
    auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), value);
    if (ec != std::errc{} || ptr != input.data() + input.size()) {
        return std::unexpected(scan_error{"Failed to parse floating point"});
    }
    return value;
}

template <typename T>
[[nodiscard]] std::expected<T, scan_error> parse_value_with_format(std::string_view input, std::string_view fmt) {
    using CleanT = remove_cvref_t<T>;

    if constexpr (!SupportedType<T>) {
        return std::unexpected(scan_error{"Unsupported type"});
    } else {
        input = trim(input);

        if (!fmt.empty() && fmt[0] == '%') {
            fmt = fmt.substr(1);
        }

        char spec = '\0';
        if (fmt.empty()) {
            if constexpr (StringLike<CleanT>) {
                spec = 's';
            } else if constexpr (FloatingPoint<CleanT>) {
                spec = 'f';
            } else if constexpr (SignedInteger<CleanT>) {
                spec = 'd';
            } else if constexpr (UnsignedInteger<CleanT>) {
                spec = 'u';
            }
        } else {
            if (fmt.size() != 1) {
                return std::unexpected(scan_error{"Invalid format specifier"});
            }
            spec = fmt[0];
        }

        if (spec != 'd' && spec != 'u' && spec != 'f' && spec != 's') {
            return std::unexpected(scan_error{"Unknown format specifier"});
        }

        return parse_value<T>(input, spec);
    }
}

template <typename... Ts>
[[nodiscard]] std::expected<std::pair<std::vector<std::string_view>, std::vector<std::string_view>>, scan_error>
parse_sources(std::string_view input, std::string_view format) {
    std::vector<std::string_view> format_parts;
    std::vector<std::string_view> input_parts;
    size_t start = 0;
    while (true) {
        size_t open = format.find('{', start);
        if (open == std::string_view::npos) {
            break;
        }
        size_t close = format.find('}', open);
        if (close == std::string_view::npos) {
            break;
        }

        if (open > start) {
            std::string_view between = format.substr(start, open - start);
            auto pos = input.find(between);
            if (input.size() < between.size() || pos == std::string_view::npos) {
                return std::unexpected(scan_error{"Unformatted text in input and format string are different"});
            }
            if (start != 0) {
                input_parts.emplace_back(input.substr(0, pos));
            }

            input = input.substr(pos + between.size());
        }

        format_parts.push_back(format.substr(open + 1, close - open - 1));
        start = close + 1;
    }

    if (start < format.size()) {
        std::string_view remaining_format = format.substr(start);
        auto pos = input.find(remaining_format);
        if (input.size() < remaining_format.size() || pos == std::string_view::npos) {
            return std::unexpected(scan_error{"Unformatted text in input and format string are different"});
        }
        input_parts.emplace_back(input.substr(0, pos));
        input = input.substr(pos + remaining_format.size());
    } else {
        input_parts.emplace_back(input);
    }
    return std::pair{format_parts, input_parts};
}

}  // namespace stdx::details
