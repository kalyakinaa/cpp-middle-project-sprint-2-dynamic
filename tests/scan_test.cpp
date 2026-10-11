#include <gtest/gtest.h>
#include <string>
#include <string_view>

#include "scan.hpp"

TEST(ScanTest, SimpleTest) {
    auto result = stdx::scan<std::string>("number", "{}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), "number");
}

TEST(ScanTest, IntegerParsing) {
    auto result = stdx::scan<int>("13", "{}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), 13);
}

TEST(ScanTest, NegativeInteger) {
    auto result = stdx::scan<int>("-13", "{%d}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), -13);
}

TEST(ScanTest, UnsignedInteger) {
    auto result = stdx::scan<uint32_t>("4000000000", "{%u}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), 4000000000u);
}

TEST(ScanTest, FloatParsing) {
    auto result = stdx::scan<double>("3.14", "{%f}");

    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(std::get<0>(result->values()), 3.14);
}

TEST(ScanTest, MultipleTypes) {
    auto result = stdx::scan<int, double, std::string_view>("13 3.14 number", "{} {%f} {%s}");

    ASSERT_TRUE(result);

    auto [i, d, s] = result->values();

    EXPECT_EQ(i, 13);
    EXPECT_DOUBLE_EQ(d, 3.14);
    EXPECT_EQ(s, "number");
}

TEST(ScanTest, MixedFormat) {
    auto result = stdx::scan<int, std::string, double>("id=13 name=Asta score=3.14", "id={%d} name={%s} score={%f}");

    ASSERT_TRUE(result);

    auto [id, name, score] = result->values();

    EXPECT_EQ(id, 13);
    EXPECT_EQ(name, "Asta");
    EXPECT_DOUBLE_EQ(score, 3.14);
}

TEST(ScanTest, SignedUnsigned) {
    auto result = stdx::scan<int8_t, uint8_t>("-13 200", "{%d} {%u}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), -13);
    EXPECT_EQ(std::get<1>(result->values()), 200);
}

TEST(ScanTest, LongTypes) {
    auto result = stdx::scan<int64_t, uint64_t>("-7000000000000 13000000000000000000", "{%d} {%u}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), -7000000000000LL);
    EXPECT_EQ(std::get<1>(result->values()), 13000000000000000000ULL);
}

TEST(ScanTest, FloatAndIntMix) {
    auto result = stdx::scan<float, int>("3.14 13", "{%f} {%d}");

    ASSERT_TRUE(result);
    EXPECT_FLOAT_EQ(std::get<0>(result->values()), 3.14f);
    EXPECT_EQ(std::get<1>(result->values()), 13);
}

TEST(ScanTest, FormatMismatch) {
    auto result = stdx::scan<int>("number", "{}");

    EXPECT_FALSE(result);
}

TEST(ScanTest, WrongSpecifier) {
    auto result = stdx::scan<int>("13", "{%s}");

    EXPECT_FALSE(result);
}

TEST(ScanTest, PlaceholderCountMismatch) {
    auto result = stdx::scan<int>("13 14", "{}");

    EXPECT_FALSE(result);
}

TEST(ScanTest, NegativeUnsigned) {
    auto result = stdx::scan<uint32_t>("-13", "{%u}");

    EXPECT_FALSE(result);
}

TEST(ScanTest, UnsignedWithSignedSpec) {
    auto result = stdx::scan<uint32_t>("13", "{%d}");

    EXPECT_FALSE(result);
}

TEST(ScanTest, TextMismatch) {
    auto result = stdx::scan<int>("number 13", "word {}");

    EXPECT_FALSE(result);
}

TEST(ScanTest, IntegerOverflow) {
    auto result = stdx::scan<int8_t>("200", "{%d}");

    EXPECT_FALSE(result);
}

TEST(ScanTest, FloatIntoInt) {
    auto result = stdx::scan<int>("3.14", "{%f}");

    EXPECT_FALSE(result);
}

TEST(ScanTest, ConstexprTypes) {
    auto result = stdx::scan<const int, volatile double>("13 3.14", "{} {%f}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), 13);
    EXPECT_DOUBLE_EQ(std::get<1>(result->values()), 3.14);
}

TEST(ScanTest, EmptyInputForInteger) {
    auto result = stdx::scan<int>("", "{}");

    EXPECT_FALSE(result);
}

TEST(ScanTest, EmptyInputForString) {
    auto result = stdx::scan<std::string>("", "{}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), "");
}

TEST(ScanTest, WhitespaceOnlyInputForInteger) {
    auto result = stdx::scan<int>("   ", "{}");

    EXPECT_FALSE(result);
}

TEST(ScanTest, EmptyFormatNoPlaceholders) {
    auto result = stdx::scan<int>("13", "");

    EXPECT_FALSE(result);
}

TEST(ScanTest, LeadingWhitespaceInteger) {
    auto result = stdx::scan<int>("   13", "{}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), 13);
}

TEST(ScanTest, TrailingWhitespaceInteger) {
    auto result = stdx::scan<int>("13   ", "{}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), 13);
}

TEST(ScanTest, LeadingAndTrailingWhitespaceDouble) {
    auto result = stdx::scan<double>("   3.14   ", "{%f}");

    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(std::get<0>(result->values()), 3.14);
}

TEST(ScanTest, FloatWithExponent) {
    auto result = stdx::scan<double>("1e-3", "{%f}");

    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(std::get<0>(result->values()), 1e-3);
}

TEST(ScanTest, FloatNegative) {
    auto result = stdx::scan<double>("-3.14", "{%f}");

    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(std::get<0>(result->values()), -3.14);
}

TEST(ScanTest, StringWithSpacesTrimmed) {
    auto result = stdx::scan<std::string>("   number   ", "{%s}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), "number");
}

TEST(ScanTest, StringViewWithSpaces) {
    auto result = stdx::scan<std::string_view>("   number   ", "{%s}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), "number");
}

TEST(ScanTest, IntegerWithTrailingGarbage) {
    auto result = stdx::scan<int>("13number", "{%d}");

    EXPECT_FALSE(result);
}

TEST(ScanTest, IntegerWithLeadingPlus) {
    auto result = stdx::scan<int>("+13", "{%d}");

    EXPECT_FALSE(result);
}

TEST(ScanTest, DoubleWithTrailingGarbage) {
    auto result = stdx::scan<double>("3.14number", "{%f}");

    EXPECT_FALSE(result);
}

TEST(ScanTest, PlaceholderAtStart) {
    auto result = stdx::scan<int, int>("13 and 23", "{} and {}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), 13);
    EXPECT_EQ(std::get<1>(result->values()), 23);
}

TEST(ScanTest, ThreeIntegers) {
    auto result = stdx::scan<int, int, int>("13 23 33", "{} {} {}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), 13);
    EXPECT_EQ(std::get<1>(result->values()), 23);
    EXPECT_EQ(std::get<2>(result->values()), 33);
}

TEST(ScanTest, Int8MinValue) {
    auto result = stdx::scan<int8_t>("-128", "{%d}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), -128);
}

TEST(ScanTest, Int8MaxValue) {
    auto result = stdx::scan<int8_t>("127", "{%d}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), 127);
}

TEST(ScanTest, Uint8MaxValue) {
    auto result = stdx::scan<uint8_t>("255", "{%u}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), 255);
}

TEST(ScanTest, Int64MinValue) {
    auto result = stdx::scan<int64_t>("-9223372036854775808", "{%d}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), std::numeric_limits<int64_t>::min());
}

TEST(ScanTest, Uint64MaxValue) {
    auto result = stdx::scan<uint64_t>("18446744073709551615", "{%u}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), std::numeric_limits<uint64_t>::max());
}

TEST(ScanTest, InputWithNewline) {
    auto result = stdx::scan<int>("13\n", "{}");

    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<0>(result->values()), 13);
}
