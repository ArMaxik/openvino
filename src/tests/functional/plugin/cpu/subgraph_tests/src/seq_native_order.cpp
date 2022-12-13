// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "shared_test_classes/base/ov_subgraph.hpp"
#include "ngraph_functions/builders.hpp"
#include "test_utils/cpu_test_utils.hpp"
#include "transformations/op_conversions/bidirectional_sequences_decomposition.hpp"
#include "transformations/op_conversions/convert_sequences_to_tensor_iterator.hpp"

using namespace CPUTestUtils;
using namespace ov::test;

namespace SubgraphTestsDefinitions {

enum class SEQ_TYPE {
    GRU,
    LSTM,
    RNN
};

using TargetShapeParams = std::tuple<size_t,   // batch_size
                                     size_t>;  // seq_length

using InputShapeParams = std::tuple<std::vector<ov::Dimension>,       // bounds for batch_size and seq_length
                                    std::vector<TargetShapeParams>>;  // target batch_size and seq_length

using SeqParams = std::tuple<SEQ_TYPE,                            // node type
                             size_t,                              // hidden_size
                             size_t,                              // input_size
                             InputShapeParams,                    // input shapes
                             std::vector<std::string>,            // Activations
                             float,                               // Clip
                             bool,                                // Linear_before_reset
                             ov::op::RecurrentSequenceDirection,  // Direction
                             ElementType>;                        // Network precision

class SequenceCPUTest : public testing::WithParamInterface<SeqParams>, virtual public ov::test::SubgraphBaseTest, public CPUTestsBase {
public:
    static std::string getTestCaseName(const testing::TestParamInfo<SeqParams> &obj) {
        SEQ_TYPE seqType;
        size_t hidden_size, input_size;
        InputShapeParams inShapeParams;
        std::vector<std::string> activations;
        float clip;
        bool linearBeforeReset;
        ov::op::RecurrentSequenceDirection direction;
        ElementType netPrecision;

        std::tie(seqType, hidden_size, input_size, inShapeParams, activations, clip, linearBeforeReset, direction, netPrecision) = obj.param;

        std::vector<ov::Dimension> bounds;
        std::vector<TargetShapeParams> targetShapes;
        std::tie(bounds, targetShapes) = inShapeParams;

        std::ostringstream result;

        if (seqType == SEQ_TYPE::GRU) {
            result << "GRU_";
        } else if (seqType == SEQ_TYPE::LSTM) {
            result << "LSTM_";
        } else if (seqType == SEQ_TYPE::RNN) {
            result << "RNN_";
        } else {
            IE_THROW() << "Unsupported seq type";
        }
        result << "hidden_size=" << hidden_size << "_input_size=" << input_size << "_";
        result << "batch_size_dyn=" << bounds[0] << "_seq_length_dyn=" << bounds[1] << "_";
        for (const auto &ts : targetShapes) {
            size_t bs, sl;
            std::tie(bs, sl) = ts;
            result << "(bs=" << bs << "_sl=" << sl << ")_";
        }

        result << "activations=" << CommonTestUtils::vec2str(activations)  << "_";
        result << "clip=" << clip << "_";
        result << "linear=" << linearBeforeReset << "_";
        result << "direction=" << direction << "_";
        result << "netPrec=" << netPrecision;

        return result.str();
    }

protected:
    void SetUp() override {
        const size_t batch_size_pos = 0;
        const size_t seq_length_pos = 1;

        SEQ_TYPE seqType;
        size_t hidden_size, input_size;
        InputShapeParams inShapeParams;
        std::vector<std::string> activations;
        float clip;
        bool linearBeforeReset;
        ov::op::RecurrentSequenceDirection direction;
        ElementType netPrecision;

        std::tie(seqType, hidden_size, input_size, inShapeParams, activations, clip, linearBeforeReset, direction, netPrecision) = this->GetParam();

        std::vector<ov::Dimension> bounds;
        std::vector<TargetShapeParams> targetShapes;
        std::tie(bounds, targetShapes) = inShapeParams;

        targetDevice = CommonTestUtils::DEVICE_CPU;

        seqLengthInIdx = (seqType == SEQ_TYPE::LSTM ? 3 : 2);

        const size_t numDirections = direction == ov::op::RecurrentSequenceDirection::BIDIRECTIONAL ? 2 : 1;

        // dynamic shapes
        ov::PartialShape X_shape(std::vector<ov::Dimension>{bounds[seq_length_pos], bounds[batch_size_pos], ov::Dimension(input_size)});
        inputDynamicShapes.push_back(X_shape);
        ov::PartialShape second_in_shape(std::vector<ov::Dimension>{bounds[batch_size_pos], ov::Dimension(numDirections),
                                                                            ov::Dimension(hidden_size)});
        inputDynamicShapes.push_back(second_in_shape);
        if (seqType == SEQ_TYPE::LSTM) {
            inputDynamicShapes.push_back(second_in_shape);
        }
        ov::PartialShape seq_len_shape(std::vector<ov::Dimension>{bounds[batch_size_pos]});
        inputDynamicShapes.push_back(seq_len_shape);

        auto hidden_size_weight = hidden_size;
        if (seqType == SEQ_TYPE::GRU) {
            hidden_size_weight *= 3;
        } else if (seqType == SEQ_TYPE::LSTM) {
            hidden_size_weight *= 4;
        }

        std::vector<ov::Shape> weightShape;
        ov::Shape W_shape(std::vector<size_t>{numDirections, hidden_size_weight, input_size});
        weightShape.push_back(W_shape);
        ov::Shape R_shape(std::vector<size_t>{numDirections, hidden_size_weight, hidden_size});
        weightShape.push_back(R_shape);
        ov::Shape B_shape;
        if (seqType == SEQ_TYPE::GRU) {
            B_shape = std::vector<size_t>{numDirections, (linearBeforeReset ? (4 * hidden_size) : (3 * hidden_size))};
        } else {
            B_shape = std::vector<size_t>{numDirections, hidden_size_weight};
        }
        weightShape.push_back(B_shape);

        // target shape
        for (const auto &ts : targetShapes) {
            std::vector<ov::Shape> currTS;

            size_t bs, sl;
            std::tie(bs, sl) = ts;

            currTS.emplace_back(std::vector<size_t>{sl, bs, input_size});
            currTS.emplace_back(std::vector<size_t>{bs, numDirections, hidden_size});
            if (seqType == SEQ_TYPE::LSTM) {
                currTS.emplace_back(std::vector<size_t>{bs, numDirections, hidden_size});
            }
            currTS.emplace_back(std::vector<size_t>{bs});
            targetStaticShapes.push_back(currTS);
        }

        // funciton creation
        std::vector<ov::element::Type> types(inputDynamicShapes.size(), netPrecision);
        types.back() = ElementType::i64;
        auto params = ngraph::builder::makeDynamicParams(types, inputDynamicShapes);

        std::vector<int64_t> order_ref_before = {1, 0, 2};
        const auto order_before = std::make_shared<ov::op::v0::Constant>(ov::element::i64,
                                                                         ov::Shape({order_ref_before.size()}),
                                                                         order_ref_before);
        const auto transpose_before = std::make_shared<ov::op::v1::Transpose>(params[0], order_before);

        ov::OutputVector inputs;
        inputs.push_back(transpose_before);
        for (size_t i = 1; i < params.size(); i++) {
            inputs.push_back(params[i]);
        }

        std::shared_ptr<ov::Node> seq_node;
        if (seqType == SEQ_TYPE::GRU) {
            seq_node = ngraph::builder::makeGRU(inputs,
                                                weightShape,
                                                hidden_size,
                                                activations,
                                                {},
                                                {},
                                                clip,
                                                linearBeforeReset,
                                                true,
                                                direction,
                                                ngraph::helpers::SequenceTestsMode::PURE_SEQ_RAND_SEQ_LEN_PARAM);
        } else if (seqType == SEQ_TYPE::LSTM) {
            seq_node = ngraph::builder::makeLSTM(inputs,
                                                 weightShape,
                                                 hidden_size,
                                                 activations,
                                                 {},
                                                 {},
                                                 clip,
                                                 true,
                                                 direction,
                                                 ngraph::helpers::SequenceTestsMode::PURE_SEQ_RAND_SEQ_LEN_PARAM);
        } else if (seqType == SEQ_TYPE::RNN) {
            seq_node = ngraph::builder::makeRNN(inputs,
                                                weightShape,
                                                hidden_size,
                                                activations,
                                                {},
                                                {},
                                                clip,
                                                true,
                                                direction,
                                                ngraph::helpers::SequenceTestsMode::PURE_SEQ_RAND_SEQ_LEN_PARAM);
        } else {
            IE_THROW() << "Unsupported seq type";
        }

        std::vector<int64_t> order_ref_after = {2, 1, 0, 3};
        const auto order_after = std::make_shared<ov::op::v0::Constant>(ov::element::i64,
                                                                         ov::Shape({order_ref_after.size()}),
                                                                         order_ref_after);
        const auto transpose_after = std::make_shared<ov::op::v1::Transpose>(seq_node->output(0), order_after);

        ov::OutputVector results;
        results.push_back(transpose_after->output(0));

        for (size_t i = 1; i < seq_node->get_output_size(); i++) {
            results.push_back(seq_node->output(i));
        }
        function = std::make_shared<ov::Model>(results, params, "SequenceCPUTest");
    }

    void generate_inputs(const std::vector<ov::Shape>& targetInputStaticShapes) override {
        SubgraphBaseTest::generate_inputs(targetInputStaticShapes);

        const size_t batchSize = targetInputStaticShapes[0][1];
        const int64_t maxSeqLen = targetInputStaticShapes[0][0];

        const auto& funcInputs = function->inputs();
        const auto& seqLenInput = inputs.find(funcInputs[seqLengthInIdx].get_node_shared_ptr());
        if (seqLenInput == inputs.end())
            throw std::runtime_error("Could not find Sequence length input.");

        auto lenData = seqLenInput->second.data<ov::element_type_traits<ElementType::i64>::value_type>();
        std::fill(lenData, lenData + batchSize, maxSeqLen);
    }

private:
    size_t seqLengthInIdx = 2;
};

TEST_P(SequenceCPUTest, CompareWithRefs) {
    SKIP_IF_CURRENT_TEST_IS_DISABLED()

    run();
    CheckNumberOfNodesWithType(compiledModel, "RNNSeq", 1);
    CheckNumberOfNodesWithType(compiledModel, "Transpose", 0);
}

const std::vector<SEQ_TYPE> nodeType = {
    SEQ_TYPE::GRU, SEQ_TYPE::LSTM, SEQ_TYPE::RNN
};

const std::vector<size_t> hiddenSizes = {
    1, 10
};

const std::vector<size_t> inputSizes = {
    1, 10
};

const std::vector<InputShapeParams> inShapeParams = {
    InputShapeParams{std::vector<ov::Dimension>{-1, -1}, std::vector<TargetShapeParams>{TargetShapeParams{3, 8},
                                                                                        TargetShapeParams{10, 2}}},
    InputShapeParams{std::vector<ov::Dimension>{{1, 15}, {1, 15}}, std::vector<TargetShapeParams>{TargetShapeParams{3, 8},
                                                                                                  TargetShapeParams{10, 2}}}
};

std::vector<std::vector<std::string>> activations = {
    {"sigmoid", "tanh", "tanh"}
};

std::vector<float> clip{0.f};

std::vector<ov::op::RecurrentSequenceDirection> direction = {ov::op::RecurrentSequenceDirection::FORWARD};

std::vector<bool> linearBeforeReset = {true, false};

std::vector<ElementType> netPrecisions = { ElementType::f32 };

INSTANTIATE_TEST_SUITE_P(smoke_SequenceCPUTest, SequenceCPUTest,
            ::testing::Combine(::testing::ValuesIn(nodeType),
                               ::testing::ValuesIn(hiddenSizes),
                               ::testing::ValuesIn(inputSizes),
                               ::testing::ValuesIn(inShapeParams),
                               ::testing::ValuesIn(activations),
                               ::testing::ValuesIn(clip),
                               ::testing::ValuesIn(linearBeforeReset),
                               ::testing::ValuesIn(direction),
                               ::testing::ValuesIn(netPrecisions)),
            SequenceCPUTest::getTestCaseName);

} // namespace SubgraphTestsDefinitions