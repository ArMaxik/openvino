// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <gtest/gtest.h>
#include <tuple>

#include "openvino/op/convert.hpp"
#include "conversion.hpp"

using namespace ov;
using ConversionTypes = ngraph::helpers::ConversionTypes;

namespace reference_tests {
namespace ConversionOpsRefTestDefinitions {
namespace {

INSTANTIATE_TEST_SUITE_P(
    smoke_Conversion_With_Hardcoded_Refs, ReferenceConversionLayerTest,
    ::testing::Values(
        // destination boolean
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {2, 3}, ov::element::u8, ov::element::boolean,
                      std::vector<uint8_t> {0, 12, 23, 0, std::numeric_limits<uint8_t>::lowest(), std::numeric_limits<uint8_t>::max()},
                      std::vector<char> {0, 1, 1, 0, 0, 1}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {2, 3}, ov::element::i32, ov::element::boolean,
                      std::vector<int32_t> {0, -12, 23, 0, std::numeric_limits<int32_t>::lowest(), std::numeric_limits<int32_t>::max()},
                      std::vector<char> {0, 1, 1, 0, 1, 1}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {3, 3}, ov::element::f32, ov::element::boolean,
                      std::vector<float> {0.f, 1.5745f, 0.12352f, 0.f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max(),
                                          std::numeric_limits<float>::min(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()},
                      std::vector<char> {0, 1, 1, 0, 1, 1, 1, 1, 1}),
        // destination bf16
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {1, 1, 3, 5}, ov::element::f32, ov::element::bf16,
                      std::vector<float> {0.5f, 1.5f, 0.5f, 2.5f, 1.5f, 0.5f, 3.5f, 2.5f, 0.5f, 0.5f, 2.5f, 0.5f, 0.5f, 0.5f, 1.5f},
                      std::vector<bfloat16> {0.5f, 1.5f, 0.5f, 2.5f, 1.5f, 0.5f, 3.5f, 2.5f, 0.5f, 0.5f, 2.5f, 0.5f, 0.5f, 0.5f, 1.5f}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {11}, ov::element::u8, ov::element::bf16,
                      std::vector<uint8_t> {0, 10, 15, 20, 43, 56, 78, 99, 102, 130, 142},
                      std::vector<bfloat16> {0, 10, 15, 20, 43, 56, 78, 99, 102, 130, 142}),

        // destination f16
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {1, 1, 3, 5}, ov::element::f32, ov::element::f16,
                      std::vector<float> {0.5f, 1.5f, 0.5f, 2.5f, 1.5f, 0.5f, 3.5f, 2.5f, 0.5f, 0.5f, 2.5f, 0.5f, 0.5f, 0.5f, 1.5f},
                      std::vector<float16> {0.5f, 1.5f, 0.5f, 2.5f, 1.5f, 0.5f, 3.5f, 2.5f, 0.5f, 0.5f, 2.5f, 0.5f, 0.5f, 0.5f, 1.5f}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {11}, ov::element::u8, ov::element::f16,
                      std::vector<uint8_t> {0, 10, 15, 20, 43, 56, 78, 99, 102, 130, 142}, std::vector<float16> {0, 10, 15, 20, 43, 56, 78, 99, 102, 130, 142}),

        // destination f32
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {2, 2}, ov::element::u1, ov::element::f32, std::vector<uint8_t> {0xA0},
                      std::vector<float> {1.0f, 0.0f, 1.0f, 0.0f}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {2, 2}, ov::element::u4, ov::element::f32, std::vector<uint8_t> {0xFB, 0x0A},
                      std::vector<float> {15.0f, 11.0f, 0.0f, 10.0f}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {2, 2}, ov::element::u8, ov::element::f32, std::vector<uint8_t> {255, 128, 32, 0},
                      std::vector<float> {255.0f, 128.0f, 32.0f, 0.0f}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {2, 2}, ov::element::u16, ov::element::f32,
                      std::vector<uint16_t> {64000, 32000, 128, 0}, std::vector<float> {64000.0f, 32000.0f, 128.0f, 0.0f}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {2, 2}, ov::element::u32, ov::element::f32,
                      std::vector<uint32_t> {4000000, 2000000, 128, 0}, std::vector<float> {4000000.0f, 2000000.0f, 128.0f, 0.0f}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {2, 2}, ov::element::u64, ov::element::f32,
                      std::vector<uint64_t> {4000000, 2000000, 128, 0}, std::vector<float> {4000000.0f, 2000000.0f, 128.0f, 0.0f}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {2, 2}, ov::element::i4, ov::element::f32, std::vector<uint8_t> {0xFE, 0xF2},
                      std::vector<float> {-1.0f, -2.0f, -1.0f, 2.0f}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {2, 2}, ov::element::i8, ov::element::f32, std::vector<int8_t> {-127, -0, 0, 127},
                      std::vector<float> {-127.0f, -0.0f, 0.0f, 127.0f}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {2, 2}, ov::element::i16, ov::element::f32,
                      std::vector<int16_t> {-32000, -0, 0, 32000}, std::vector<float> {-32000.0f, -0.0f, 0.0f, 32000.0f}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {2, 2}, ov::element::i32, ov::element::f32,
                      std::vector<int32_t> {-64000, -0, 0, 64000}, std::vector<float> {-64000.0f, -0.0f, 0.0f, 64000.0f}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {2, 2}, ov::element::i64, ov::element::f32,
                      std::vector<int64_t> {-64000, -0, 0, 64000}, std::vector<float> {-64000.0f, -0.0f, 0.0f, 64000.0f}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {1, 1, 3, 5}, ov::element::bf16, ov::element::f32,
                      std::vector<bfloat16> {0.5f, 1.5f, 0.5f, 2.5f, 1.5f, 0.5f, 3.5f, 2.5f, 0.5f, 0.5f, 2.5f, 0.5f, 0.5f, 0.5f, 1.5f},
                      std::vector<float> {0.5f, 1.5f, 0.5f, 2.5f, 1.5f, 0.5f, 3.5f, 2.5f, 0.5f, 0.5f, 2.5f, 0.5f, 0.5f, 0.5f, 1.5f}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {1, 1, 3, 5}, ov::element::f16, ov::element::f32,
                      std::vector<float16> {0.5f, 1.5f, 0.5f, 2.5f, 1.5f, 0.5f, 3.5f, 2.5f, 0.5f, 0.5f, 2.5f, 0.5f, 0.5f, 0.5f, 1.5f},
                      std::vector<float> {0.5f, 1.5f, 0.5f, 2.5f, 1.5f, 0.5f, 3.5f, 2.5f, 0.5f, 0.5f, 2.5f, 0.5f, 0.5f, 0.5f, 1.5f}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {1, 1, 3, 5}, ov::element::f32, ov::element::f32,
                      std::vector<float> {0.5f, 1.5f, 0.5f, 2.5f, 1.5f, 0.5f, 3.5f, 2.5f, 0.5f, 0.5f, 2.5f, 0.5f, 0.5f, 0.5f, 1.5f},
                      std::vector<float> {0.5f, 1.5f, 0.5f, 2.5f, 1.5f, 0.5f, 3.5f, 2.5f, 0.5f, 0.5f, 2.5f, 0.5f, 0.5f, 0.5f, 1.5f}),

        // destination i4
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u1, ov::element::i4, std::vector<uint8_t> {0xA0},
                      std::vector<uint8_t> {0x10, 0x10}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u4, ov::element::i4, std::vector<uint8_t> {0x12, 0x03},
                      std::vector<uint8_t> {0x12, 0x03}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u8, ov::element::i4, std::vector<uint8_t> {1, 2, 0, 3},
                      std::vector<uint8_t> {0x12, 0x03}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u16, ov::element::i4, std::vector<uint16_t> {1, 2, 0, 3},
                      std::vector<uint8_t> {0x12, 0x03}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u32, ov::element::i4, std::vector<uint32_t> {1, 2, 0, 3},
                      std::vector<uint8_t> {0x12, 0x03}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u64, ov::element::i4, std::vector<uint64_t> {1, 2, 0, 3},
                      std::vector<uint8_t> {0x12, 0x03}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i4, ov::element::i4, std::vector<uint8_t> {0xFE, 0x03},
                      std::vector<uint8_t> {0xFE, 0x03}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i8, ov::element::i4, std::vector<int8_t> {-1, -2, 2, 3},
                      std::vector<uint8_t> {0xFE, 0x23}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i16, ov::element::i4, std::vector<int16_t> {-1, -2, 2, 3},
                      std::vector<uint8_t> {0xFE, 0x23}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i32, ov::element::i4, std::vector<int32_t> {-1, -2, 2, 3},
                      std::vector<uint8_t> {0xFE, 0x23}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i64, ov::element::i4, std::vector<int64_t> {-1, -2, 2, 3},
                      std::vector<uint8_t> {0xFE, 0x23}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f16, ov::element::i4,
                      std::vector<ngraph::float16> {-1, -2, 0, 3}, std::vector<uint8_t> {0xFE, 0x03}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::bf16, ov::element::i4,
                      std::vector<ngraph::bfloat16> {-1, -2, 0, 3}, std::vector<uint8_t> {0xFE, 0x03}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f32, ov::element::i4, std::vector<float> {-1, -2, 2, 3},
                      std::vector<uint8_t> {0xFE, 0x23}, 4, 4),
        // destination i8
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::u1, ov::element::i8, std::vector<uint8_t> {0x81},
                      std::vector<int8_t> {1, 0, 0, 0, 0, 0, 0, 1}, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u4, ov::element::i8, std::vector<uint8_t> {0x21, 0x43},
                      std::vector<int8_t> {2, 1, 4, 3}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u8, ov::element::i8, std::vector<uint8_t> {1, 2, 0, 3},
                      std::vector<int8_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u16, ov::element::i8, std::vector<uint16_t> {1, 2, 0, 3},
                      std::vector<int8_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u32, ov::element::i8, std::vector<uint32_t> {1, 2, 0, 3},
                      std::vector<int8_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u64, ov::element::i8, std::vector<uint64_t> {1, 2, 0, 3},
                      std::vector<int8_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i4, ov::element::i8, std::vector<uint8_t> {0x21, 0x43},
                      std::vector<int8_t> {2, 1, 4, 3}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i8, ov::element::i8, std::vector<int8_t> {-1, -2, 2, 3},
                      std::vector<int8_t> {-1, -2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i16, ov::element::i8, std::vector<int16_t> {-1, -2, 2, 3},
                      std::vector<int8_t> {-1, -2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i32, ov::element::i8, std::vector<int32_t> {-1, -2, 2, 3},
                      std::vector<int8_t> {-1, -2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i64, ov::element::i8, std::vector<int64_t> {-1, -2, 2, 3},
                      std::vector<int8_t> {-1, -2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f16, ov::element::i8,
                      std::vector<ngraph::float16> {-1, -2, 0, 3}, std::vector<int8_t> {-1, -2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::bf16, ov::element::i8,
                      std::vector<ngraph::bfloat16> {-1, -2, 0, 3}, std::vector<int8_t> {-1, -2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f32, ov::element::i8, std::vector<float> {-1, -2, 2.2, 3.8},
                      std::vector<int8_t> {-1, -2, 2, 3}),
        // destination i16
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::u1, ov::element::i16, std::vector<uint8_t> {0x81},
                      std::vector<int16_t> {1, 0, 0, 0, 0, 0, 0, 1}, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u4, ov::element::i16, std::vector<uint8_t> {0x21, 0x43},
                      std::vector<int16_t> {2, 1, 4, 3}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u8, ov::element::i16, std::vector<uint8_t> {1, 2, 0, 3},
                      std::vector<int16_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u16, ov::element::i16, std::vector<uint16_t> {1, 2, 0, 3},
                      std::vector<int16_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u32, ov::element::i16, std::vector<uint32_t> {1, 2, 0, 3},
                      std::vector<int16_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u64, ov::element::i16, std::vector<uint64_t> {1, 2, 0, 3},
                      std::vector<int16_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i4, ov::element::i16, std::vector<uint8_t> {0x21, 0x43},
                      std::vector<int16_t> {2, 1, 4, 3}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i8, ov::element::i16, std::vector<int8_t> {-1, -2, 2, 3},
                      std::vector<int16_t> {-1, -2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i16, ov::element::i16, std::vector<int16_t> {-1, -2, 2, 3},
                      std::vector<int16_t> {-1, -2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i32, ov::element::i16, std::vector<int32_t> {-1, -2, 2, 3},
                      std::vector<int16_t> {-1, -2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i64, ov::element::i16, std::vector<int64_t> {-1, -2, 2, 3},
                      std::vector<int16_t> {-1, -2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f16, ov::element::i16,
                      std::vector<ngraph::float16> {-1, -2, 0, 3}, std::vector<int16_t> {-1, -2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::bf16, ov::element::i16,
                      std::vector<ngraph::bfloat16> {-1, -2, 0, 3}, std::vector<int16_t> {-1, -2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f32, ov::element::i16, std::vector<float> {-1, -2, 2.2, 3.8},
                      std::vector<int16_t> {-1, -2, 2, 3}),
        // destination i32
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::u1, ov::element::i32, std::vector<uint8_t> {0x81},
                      std::vector<int32_t> {1, 0, 0, 0, 0, 0, 0, 1}, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u4, ov::element::i32, std::vector<uint8_t> {0x21, 0x43},
                      std::vector<int32_t> {2, 1, 4, 3}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u8, ov::element::i32, std::vector<uint8_t> {1, 2, 0, 3},
                      std::vector<int32_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u16, ov::element::i32, std::vector<uint16_t> {1, 2, 0, 3},
                      std::vector<int32_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u32, ov::element::i32, std::vector<uint32_t> {1, 2, 0, 3},
                      std::vector<int32_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u64, ov::element::i32, std::vector<uint64_t> {1, 2, 0, 3},
                      std::vector<int32_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i4, ov::element::i32, std::vector<uint8_t> {0x21, 0x43},
                      std::vector<int32_t> {2, 1, 4, 3}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i8, ov::element::i32, std::vector<int8_t> {-1, -2, 2, 3},
                      std::vector<int32_t> {-1, -2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i16, ov::element::i32, std::vector<int16_t> {-1, -2, 2, 3},
                      std::vector<int32_t> {-1, -2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i32, ov::element::i32, std::vector<int32_t> {-1, -2, 2, 3},
                      std::vector<int32_t> {-1, -2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i64, ov::element::i32, std::vector<int64_t> {-1, -2, 2, 3},
                      std::vector<int32_t> {-1, -2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f16, ov::element::i32,
                      std::vector<ngraph::float16> {-1, -2, 0, 3}, std::vector<int32_t> {-1, -2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::bf16, ov::element::i32,
                      std::vector<ngraph::bfloat16> {-1, -2, 0, 3}, std::vector<int32_t> {-1, -2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f32, ov::element::i32, std::vector<float> {-1, -2, 2.2, 3.8},
                      std::vector<int32_t> {-1, -2, 2, 3}),
        // destination i64
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::u1, ov::element::i64, std::vector<uint8_t> {0x81},
                      std::vector<int64_t> {1, 0, 0, 0, 0, 0, 0, 1}, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u4, ov::element::i64, std::vector<uint8_t> {0x21, 0x43},
                      std::vector<int64_t> {2, 1, 4, 3}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u8, ov::element::i64, std::vector<uint8_t> {1, 2, 0, 3},
                      std::vector<int64_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u16, ov::element::i64, std::vector<uint16_t> {1, 2, 0, 3},
                      std::vector<int64_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u32, ov::element::i64, std::vector<uint32_t> {1, 2, 0, 3},
                      std::vector<int64_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u64, ov::element::i64, std::vector<uint64_t> {1, 2, 0, 3},
                      std::vector<int64_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i4, ov::element::i64, std::vector<uint8_t> {0x21, 0x43},
                      std::vector<int64_t> {2, 1, 4, 3}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i8, ov::element::i64, std::vector<int8_t> {-1, -2, 2, 3},
                      std::vector<int64_t> {-1, -2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i16, ov::element::i64, std::vector<int16_t> {-1, -2, 2, 3},
                      std::vector<int64_t> {-1, -2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i32, ov::element::i64, std::vector<int32_t> {-1, -2, 2, 3},
                      std::vector<int64_t> {-1, -2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i64, ov::element::i64, std::vector<int64_t> {-1, -2, 2, 3},
                      std::vector<int64_t> {-1, -2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f16, ov::element::i64,
                      std::vector<ngraph::float16> {-1, -2, 0, 3}, std::vector<int64_t> {-1, -2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::bf16, ov::element::i64,
                      std::vector<ngraph::bfloat16> {-1, -2, 0, 3}, std::vector<int64_t> {-1, -2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f32, ov::element::i64, std::vector<float> {-1, -2, 2.2, 3.8},
                      std::vector<int64_t> {-1, -2, 2, 3}),

        // destination u1
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::u1, ov::element::u1, std::vector<uint8_t> {0xA0},
                      std::vector<uint8_t> {0xA0}, 8, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::u4, ov::element::u1,
                      std::vector<uint8_t> {0x10, 0x01, 0x00, 0x00}, std::vector<uint8_t> {0x90}, 8, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::u8, ov::element::u1,
                      std::vector<uint8_t> {1, 0, 1, 0, 0, 0, 0, 1}, std::vector<uint8_t> {0xA1}, 8, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::u16, ov::element::u1,
                      std::vector<uint16_t> {1, 0, 1, 0, 0, 0, 0, 1}, std::vector<uint8_t> {0xA1}, 8, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::u32, ov::element::u1,
                      std::vector<uint32_t> {1, 0, 1, 0, 0, 0, 0, 1}, std::vector<uint8_t> {0xA1}, 8, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::u64, ov::element::u1,
                      std::vector<uint64_t> {1, 0, 1, 0, 0, 0, 0, 1}, std::vector<uint8_t> {0xA1}, 8, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::i4, ov::element::u1,
                      std::vector<uint8_t> {0x10, 0x01, 0x00, 0x00}, std::vector<uint8_t> {0x90}, 8, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::i8, ov::element::u1,
                      std::vector<int8_t> {1, 0, 1, 0, 0, 0, 0, 1}, std::vector<uint8_t> {0xA1}, 8, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::i16, ov::element::u1,
                      std::vector<int16_t> {1, 0, 1, 0, 0, 0, 0, 1}, std::vector<uint8_t> {0xA1}, 8, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::i32, ov::element::u1,
                      std::vector<int32_t> {1, 0, 1, 0, 0, 0, 0, 1}, std::vector<uint8_t> {0xA1}, 8, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::i64, ov::element::u1,
                      std::vector<int64_t> {1, 0, 1, 0, 0, 0, 0, 1}, std::vector<uint8_t> {0xA1}, 8, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::f16, ov::element::u1,
                      std::vector<ngraph::float16> {1, 0, 1, 0, 0, 0, 0, 1}, std::vector<uint8_t> {0xA1}, 8, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::bf16, ov::element::u1,
                      std::vector<ngraph::bfloat16> {1, 0, 1, 0, 0, 0, 0, 1}, std::vector<uint8_t> {0xA1}, 8, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::f32, ov::element::u1,
                      std::vector<float> {1, 0, 1, 0, 0, 0, 0, 1}, std::vector<uint8_t> {0xA1}, 8, 8),

        // destination u4
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u1, ov::element::u4, std::vector<uint8_t> {0xA0},
                      std::vector<uint8_t> {0x10, 0x10}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u4, ov::element::u4, std::vector<uint8_t> {0x12, 0x03},
                      std::vector<uint8_t> {0x12, 0x03}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u8, ov::element::u4, std::vector<uint8_t> {1, 2, 0, 3},
                      std::vector<uint8_t> {0x12, 0x03}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u16, ov::element::u4, std::vector<uint16_t> {1, 2, 0, 3},
                      std::vector<uint8_t> {0x12, 0x03}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u32, ov::element::u4, std::vector<uint32_t> {1, 2, 0, 3},
                      std::vector<uint8_t> {0x12, 0x03}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u64, ov::element::u4, std::vector<uint64_t> {1, 2, 0, 3},
                      std::vector<uint8_t> {0x12, 0x03}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i4, ov::element::u4, std::vector<uint8_t> {0xFE, 0x03},
                      std::vector<uint8_t> {0xFE, 0x03}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i8, ov::element::u4, std::vector<int8_t> {-1, -2, 2, 3},
                      std::vector<uint8_t> {0xFE, 0x23}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i16, ov::element::u4, std::vector<int16_t> {-1, -2, 2, 3},
                      std::vector<uint8_t> {0xFE, 0x23}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i32, ov::element::u4, std::vector<int32_t> {-1, -2, 2, 3},
                      std::vector<uint8_t> {0xFE, 0x23}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i64, ov::element::u4, std::vector<int64_t> {-1, -2, 2, 3},
                      std::vector<uint8_t> {0xFE, 0x23}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f16, ov::element::u4,
                      std::vector<ngraph::float16> {-1, -2, 0, 3}, std::vector<uint8_t> {0xFE, 0x03}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::bf16, ov::element::u4,
                      std::vector<ngraph::bfloat16> {-1, -2, 0, 3}, std::vector<uint8_t> {0xFE, 0x03}, 4, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f32, ov::element::u4, std::vector<float> {-1, -2, 2, 3},
                      std::vector<uint8_t> {0xFE, 0x23}, 4, 4),

        // destination u8
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::u1, ov::element::u8, std::vector<uint8_t> {0x81},
                      std::vector<uint8_t> {1, 0, 0, 0, 0, 0, 0, 1}, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u4, ov::element::u8, std::vector<uint8_t> {0x21, 0x43},
                      std::vector<uint8_t> {2, 1, 4, 3}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u8, ov::element::u8, std::vector<uint8_t> {1, 2, 0, 3},
                      std::vector<uint8_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u16, ov::element::u8, std::vector<uint16_t> {1, 2, 0, 3},
                      std::vector<uint8_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u32, ov::element::u8, std::vector<uint32_t> {1, 2, 0, 3},
                      std::vector<uint8_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u64, ov::element::u8, std::vector<uint64_t> {1, 2, 0, 3},
                      std::vector<uint8_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i4, ov::element::u8, std::vector<uint8_t> {0x21, 0x43},
                      std::vector<uint8_t> {2, 1, 4, 3}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i8, ov::element::u8, std::vector<int8_t> {1, 2, 2, 3},
                      std::vector<uint8_t> {1, 2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i16, ov::element::u8, std::vector<int16_t> {1, 2, 2, 3},
                      std::vector<uint8_t> {1, 2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i32, ov::element::u8, std::vector<int32_t> {1, 2, 2, 3},
                      std::vector<uint8_t> {1, 2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i64, ov::element::u8, std::vector<int64_t> {1, 2, 2, 3},
                      std::vector<uint8_t> {1, 2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f16, ov::element::u8, std::vector<ngraph::float16> {1, 2, 0, 3},
                      std::vector<uint8_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::bf16, ov::element::u8,
                      std::vector<ngraph::bfloat16> {1, 2, 0, 3}, std::vector<uint8_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f32, ov::element::u8, std::vector<float> {1, 2, 2.2, 3.8},
                      std::vector<uint8_t> {1, 2, 2, 3}),

        // destination u16
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::u1, ov::element::u16, std::vector<uint8_t> {0x81},
                      std::vector<uint16_t> {1, 0, 0, 0, 0, 0, 0, 1}, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u4, ov::element::u16, std::vector<uint8_t> {0x21, 0x43},
                      std::vector<uint16_t> {2, 1, 4, 3}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u8, ov::element::u16, std::vector<uint8_t> {1, 2, 0, 3},
                      std::vector<uint16_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u16, ov::element::u16, std::vector<uint16_t> {1, 2, 0, 3},
                      std::vector<uint16_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u32, ov::element::u16, std::vector<uint32_t> {1, 2, 0, 3},
                      std::vector<uint16_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u64, ov::element::u16, std::vector<uint64_t> {1, 2, 0, 3},
                      std::vector<uint16_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i4, ov::element::u16, std::vector<uint8_t> {0x21, 0x43},
                      std::vector<uint16_t> {2, 1, 4, 3}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i8, ov::element::u16, std::vector<int8_t> {1, 2, 2, 3},
                      std::vector<uint16_t> {1, 2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i16, ov::element::u16, std::vector<int16_t> {1, 2, 2, 3},
                      std::vector<uint16_t> {1, 2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i32, ov::element::u16, std::vector<int32_t> {1, 2, 2, 3},
                      std::vector<uint16_t> {1, 2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i64, ov::element::u16, std::vector<int64_t> {1, 2, 2, 3},
                      std::vector<uint16_t> {1, 2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f16, ov::element::u16, std::vector<ngraph::float16> {1, 2, 0, 3},
                      std::vector<uint16_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::bf16, ov::element::u16,
                      std::vector<ngraph::bfloat16> {1, 2, 0, 3}, std::vector<uint16_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f32, ov::element::u16, std::vector<float> {1, 2, 2.2, 3.8},
                      std::vector<uint16_t> {1, 2, 2, 3}),

        // destination u32
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::u1, ov::element::u32, std::vector<uint8_t> {0x81},
                      std::vector<uint32_t> {1, 0, 0, 0, 0, 0, 0, 1}, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u4, ov::element::u32, std::vector<uint8_t> {0x21, 0x43},
                      std::vector<uint32_t> {2, 1, 4, 3}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u8, ov::element::u32, std::vector<uint8_t> {1, 2, 0, 3},
                      std::vector<uint32_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u16, ov::element::u32, std::vector<uint16_t> {1, 2, 0, 3},
                      std::vector<uint32_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u32, ov::element::u32, std::vector<uint32_t> {1, 2, 0, 3},
                      std::vector<uint32_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u64, ov::element::u32, std::vector<uint64_t> {1, 2, 0, 3},
                      std::vector<uint32_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i4, ov::element::u32, std::vector<uint8_t> {0x21, 0x43},
                      std::vector<uint32_t> {2, 1, 4, 3}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i8, ov::element::u32, std::vector<int8_t> {1, 2, 2, 3},
                      std::vector<uint32_t> {1, 2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i16, ov::element::u32, std::vector<int16_t> {1, 2, 2, 3},
                      std::vector<uint32_t> {1, 2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i32, ov::element::u32, std::vector<int32_t> {1, 2, 2, 3},
                      std::vector<uint32_t> {1, 2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i64, ov::element::u32, std::vector<int64_t> {1, 2, 2, 3},
                      std::vector<uint32_t> {1, 2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f16, ov::element::u32, std::vector<ngraph::float16> {1, 2, 0, 3},
                      std::vector<uint32_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::bf16, ov::element::u32,
                      std::vector<ngraph::bfloat16> {1, 2, 0, 3}, std::vector<uint32_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f32, ov::element::u32, std::vector<float> {1, 2, 2.2, 3.8},
                      std::vector<uint32_t> {1, 2, 2, 3}),
        // destination u64
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {8}, ov::element::u1, ov::element::u64, std::vector<uint8_t> {0x81},
                      std::vector<uint64_t> {1, 0, 0, 0, 0, 0, 0, 1}, 8),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u4, ov::element::u64, std::vector<uint8_t> {0x21, 0x43},
                      std::vector<uint64_t> {2, 1, 4, 3}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u8, ov::element::u64, std::vector<uint8_t> {1, 2, 0, 3},
                      std::vector<uint64_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u16, ov::element::u64, std::vector<uint16_t> {1, 2, 0, 3},
                      std::vector<uint64_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u32, ov::element::u64, std::vector<uint32_t> {1, 2, 0, 3},
                      std::vector<uint64_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::u64, ov::element::u64, std::vector<uint64_t> {1, 2, 0, 3},
                      std::vector<uint64_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i4, ov::element::u64, std::vector<uint8_t> {0x21, 0x43},
                      std::vector<uint64_t> {2, 1, 4, 3}, 4),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i8, ov::element::u64, std::vector<int8_t> {1, 2, 2, 3},
                      std::vector<uint64_t> {1, 2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i16, ov::element::u64, std::vector<int16_t> {1, 2, 2, 3},
                      std::vector<uint64_t> {1, 2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i32, ov::element::u64, std::vector<int32_t> {1, 2, 2, 3},
                      std::vector<uint64_t> {1, 2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::i64, ov::element::u64, std::vector<int64_t> {1, 2, 2, 3},
                      std::vector<uint64_t> {1, 2, 2, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f16, ov::element::u64, std::vector<ngraph::float16> {1, 2, 0, 3},
                      std::vector<uint64_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::bf16, ov::element::u64,
                      std::vector<ngraph::bfloat16> {1, 2, 0, 3}, std::vector<uint64_t> {1, 2, 0, 3}),
        ConvertParams(ConversionTypes::CONVERT, ov::PartialShape {4}, ov::element::f32, ov::element::u64, std::vector<float> {1, 2, 2.2, 3.8},
                      std::vector<uint64_t> {1, 2, 2, 3})),
    ReferenceConversionLayerTest::getTestCaseName);
} // namespace
} // namespace ConversionOpsRefTestDefinitions
} // namespace reference_tests