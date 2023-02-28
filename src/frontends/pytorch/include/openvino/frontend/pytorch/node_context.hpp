// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "openvino/frontend/exception.hpp"
#include "openvino/frontend/node_context.hpp"
#include "openvino/frontend/pytorch/decoder.hpp"

namespace ov {
namespace frontend {
namespace pytorch {

class TranslateSession;

typedef std::unordered_map<size_t, Output<Node>> TensorMap;

class NodeContext : public frontend::NodeContext {
public:
    NodeContext(std::shared_ptr<TorchDecoder> decoder,
                TensorMap* tensor_map,
                ParameterVector* external_parameters,
                const TensorMap& ext_tensor_map,
                TranslateSession* translate_session)
        : frontend::NodeContext(decoder->get_op_type()),
          m_decoder(decoder),
          m_tensor_map(tensor_map),
          m_ext_tensor_map(ext_tensor_map),
          m_external_parameters(external_parameters),
          m_translate_session(translate_session),
          m_decoder_inputs(decoder->inputs()),
          m_decoder_outputs(decoder->outputs()) {
        FRONT_END_GENERAL_CHECK(tensor_map != nullptr && external_parameters != nullptr &&
                                translate_session != nullptr);
    }

    // Do not search for input in tensor map; try to access it as a constant of specified type T and return its value
    template <typename T>
    T const_input(size_t index) const;

    size_t get_input_size() const override {
        return m_decoder_inputs.size();
    };

    // Search for input in tensor map and return an output port for already converted op
    // TODO: int due to base class uses it, but naturally it should be size_t for PT
    Output<Node> get_input(int index) const override {
        FRONT_END_GENERAL_CHECK(!m_decoder->input_is_none(index), "Input is none with index: ", index);
        auto input = m_decoder_inputs.at(index);
        FRONT_END_GENERAL_CHECK(m_tensor_map->count(input), "No tensor corresponding input: ", input, " exist.");
        return m_tensor_map->at(input);
    }

    // TODO: upstream to base class
    OutputVector inputs() const {
        OutputVector res;
        for (auto input : m_decoder_inputs) {
            FRONT_END_GENERAL_CHECK(m_tensor_map->count(input), "No tensor corresponding index: ", input, " exist.");
            res.push_back(m_tensor_map->at(input));
        }
        return res;
    }

    Any get_input_type(size_t index) const {
        return m_decoder->get_input_type(index);
    }

    bool input_is_none(size_t index) const {
        return m_decoder->input_is_none(index);
    }

    size_t get_output_size() const {
        return m_decoder_outputs.size();
    }

    std::vector<size_t> outputs() const {
        return m_decoder_outputs;
    }

    // Convert the resulting value of this node to ov Constant; works correctly only for nodes that produce
    // constant value, naturally for prim::Constant
    OutputVector as_constant() const;

    std::string get_schema() const {
        return m_decoder->get_schema();
    }

    std::shared_ptr<Node> mark_node(std::shared_ptr<Node> ov_node) const;

    // Call mark_node for each node from the vector
    void mark_nodes(std::vector<std::shared_ptr<Node>> ov_nodes) const {
        for (auto& ov_node : ov_nodes) {
            mark_node(ov_node);
        }
    }

    // Syntactic sugar around mark_node -- just calls it for corresponding node for the passed output port
    Output<Node> mark_output(Output<Node> ov_output) const {
        mark_node(ov_output.get_node_shared_ptr());
        return ov_output;
    }

    Any get_attribute_as_any(const std::string&) const override {
        throw std::runtime_error(
            "There is no any named attributes in PyTorch node, query by attribute name is not implemented");
    }

    void mutate_input(size_t index, Output<Node> ov_output);

    std::set<size_t> get_mutated_tensors() const {
        return m_mutated_tensors;
    }

    std::shared_ptr<TorchDecoder> get_decoder() const {
        return m_decoder;
    }

    TranslateSession* get_session() const {
        return m_translate_session;
    }

    void add_tensor_to_context(size_t index, Output<Node> ov_output);

    Output<Node> get_tensor_from_model(size_t index) const {
        if (m_tensor_map->find(index) != m_tensor_map->end()) {
            return m_tensor_map->at(index);
        } else {
            return Output<Node>();
        }
    }

    Output<Node> get_tensor_from_model_or_create_input(size_t index);
    Output<Node> get_input_from_visible_context(size_t index) const;
    std::shared_ptr<ov::Model> convert_subgraph(size_t index);

private:
    std::shared_ptr<TorchDecoder> m_decoder;
    std::set<size_t> m_mutated_tensors;
    TensorMap* m_tensor_map;
    const TensorMap& m_ext_tensor_map;
    ParameterVector* m_external_parameters;
    TranslateSession* m_translate_session;
    const std::vector<size_t> m_decoder_inputs;
    const std::vector<size_t> m_decoder_outputs;
};

using PytorchCreatorFunction = std::function<OutputVector(NodeContext&)>;

}  // namespace pytorch
}  // namespace frontend
}  // namespace ov