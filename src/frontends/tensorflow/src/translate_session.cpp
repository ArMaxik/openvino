// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "translate_session.hpp"

#include "input_model.hpp"
#include "openvino/op/util/framework_node.hpp"
#include "openvino/opsets/opset10.hpp"
#include "openvino/opsets/opset8.hpp"
#include "tf_framework_node.hpp"
#include "utils.hpp"

using namespace ov::frontend::tensorflow;

namespace {
template <typename T>
std::vector<T> reorder_ops_by_names(const std::vector<std::string>& names, const std::vector<T>& ops) {
    if (names.empty()) {
        // in case unspecified names, return the initial order of operations
        return ops;
    }
    FRONT_END_GENERAL_CHECK(names.size() == ops.size(),
                            "[TensorFlow Frontend] Internal error: cannot perform reordering of operations. The number "
                            "of names mismatches the number of operations.");
    std::vector<T> resulted_ops(ops.size(), nullptr);
    for (const auto& op : ops) {
        const auto& op_name = op->get_friendly_name();
        auto iter = std::find(names.begin(), names.end(), op_name);
        FRONT_END_GENERAL_CHECK(iter != names.end(),
                                "[TensorFlow Frontend] Internal error: cannot perform reordering of operations. The "
                                "requested name is not found among operations.");
        auto ind = std::distance(names.begin(), iter);
        FRONT_END_GENERAL_CHECK(resulted_ops[ind] == nullptr,
                                "[TensorFlow Frontend] Internal error: incorrect reordering of operations. "
                                "Found two operations that are mapped to the same name.");
        resulted_ops[ind] = op;
    }
    return resulted_ops;
};

/// \brief Adds known input names from Saved Model file format
/// \param[in] node Node which should be updated
/// \param[in] saved_model_names Map of names from saved model
/// \returns True if node was updated, false otherwise
static bool apply_saved_model_names(std::shared_ptr<ov::Node> node,
                                    const std::shared_ptr<std::map<std::string, std::string>>& saved_model_names) {
    if (std::dynamic_pointer_cast<ov::opset8::Parameter>(node)) {
        for (size_t i = 0; i < node->get_output_size(); ++i) {
            const auto& node_names = node->get_output_tensor(i).get_names();
            for (const auto& name : node_names) {
                const auto& saved_model_name = saved_model_names->find(name);
                if (saved_model_name != saved_model_names->end()) {
                    set_node_name(saved_model_name->second, node);
                    return true;
                }
            }
        }
    } else if (std::dynamic_pointer_cast<ov::opset10::Result>(node)) {
        for (size_t i = 0; i < node->get_input_size(); ++i) {
            const auto& node_names = node->get_input_tensor(i).get_names();
            for (const auto& name : node_names) {
                const auto& saved_model_name = saved_model_names->find(name);
                if (saved_model_name != saved_model_names->end()) {
                    node->set_friendly_name(saved_model_name->second);
                    node->get_input_tensor(i).add_names({saved_model_name->second});
                    return true;
                }
            }
        }
    }
    return false;
}

// it creates framework node and saves exception message in the node attribute
ov::OutputVector create_fw_node_with_exception(const std::shared_ptr<DecoderBase>& decoder,
                                               const ov::OutputVector& inputs,
                                               size_t num_outputs,
                                               const std::string& operation_name,
                                               const std::string& exception_message) {
    ov::op::util::FrameworkNodeAttrs attrs;
    attrs[FrameworkNode::failed_conversion_key] = exception_message;
    auto fw_node = std::make_shared<FrameworkNode>(decoder, inputs, num_outputs);
    fw_node->set_attrs(attrs);
    set_node_name(operation_name, fw_node);
    return fw_node->outputs();
}

size_t get_flat_index_by_name_and_id(const ov::frontend::NamedOutputVector& outputs,
                                     const std::string& name,
                                     size_t idx) {
    // Assume that if at least one output port has name, then all the ports should have names
    if (!outputs.empty() && !outputs.front().name.empty()) {
        // Producer has names in ports
        auto it = std::find_if(outputs.begin(), outputs.end(), [&](const ov::frontend::NamedOutput& x) {
            return name == x.name;
        });
        FRONT_END_GENERAL_CHECK(outputs.end() - it > ptrdiff_t(idx),
                                "There is no output port specified by name and index");
        FRONT_END_GENERAL_CHECK(it[idx].name == name,
                                "There is no output port with specified index in a group with specified name");
        return it - outputs.begin() + idx;
    } else {
        // There are no named ports in the producer node, so reference by name wouldn't work
        return idx;
    }
}
}  // namespace

TranslateSession::TranslateSession(const ov::frontend::InputModel::Ptr& input_model,
                                   const std::shared_ptr<TranslatorDictionaryType>& translator_map,
                                   const std::string& model_name)
    : m_input_model(input_model),
      m_translator_map(translator_map),
      m_model_name(model_name),
      m_ov_model(nullptr),
      m_cached_body_models(std::make_shared<CachedBodyModelsType>()) {}

std::shared_ptr<ov::Model> TranslateSession::get_converted_model() {
    if (m_ov_model) {
        return m_ov_model;
    }
    translate_graph(m_input_model, m_ov_model);
    return m_ov_model;
}

void TranslateSession::inject_body_model(std::shared_ptr<ov::Model> body_model,
                                         const std::string& operation_type,
                                         const ov::OutputVector& ov_inputs,
                                         ov::OutputVector& ov_outputs) {
    ov_outputs.clear();
    auto body_parameters = body_model->get_parameters();
    FRONT_END_GENERAL_CHECK(body_parameters.size() == ov_inputs.size(),
                            "[TensorFlow Error] Internal error or incorrect input models: number of "
                            "inputs and arguments to the function " +
                                operation_type + " do not match.");
    for (size_t param_ind = 0; param_ind < body_parameters.size(); ++param_ind) {
        auto orig_type = body_parameters[param_ind]->get_element_type();
        body_parameters[param_ind]->output(0).replace(ov_inputs[param_ind]);
        if (auto ext_parameter = as_type_ptr<ov::opset8::Parameter>(ov_inputs[param_ind].get_node_shared_ptr())) {
            // save type of a Parameter as converted in the body
            // this is important if the external conversion extension is applied to body graph node
            // with setting its own type
            if (orig_type != element::dynamic) {
                ext_parameter->set_element_type(orig_type);
            }
        }
    }
    for (const auto& result_node : body_model->get_results()) {
        ov_outputs.push_back(result_node->input_value(0));
    }
}

void TranslateSession::translate_graph(const ov::frontend::InputModel::Ptr& input_model,
                                       std::shared_ptr<ov::Model>& ov_model) {
    OpMap ng_op_map;
    ov::ParameterVector params;
    ov::ResultVector results;
    const auto& model_tf = std::dynamic_pointer_cast<InputModel>(input_model);
    FRONT_END_GENERAL_CHECK(model_tf, "nullptr for InputModel is given for translation into OV Model");
    const auto& operation_places = model_tf->get_op_places();
    const auto& model_inputs = model_tf->get_inputs();
    const auto& model_outputs = model_tf->get_outputs();
    const auto& model_frozen_inputs = model_tf->get_tensor_values();
    const auto& saved_model_inputs = model_tf->get_saved_model_input_names();
    const auto& saved_model_outputs = model_tf->get_saved_model_output_names();

    // fill ng_op_map with Constant outputs for frozen inputs
    for (const auto& frozen_input : model_frozen_inputs) {
        const auto& frozen_input_name = frozen_input.first;
        const auto& frozen_input_value = frozen_input.second;
        FRONT_END_GENERAL_CHECK(ng_op_map.count(frozen_input_name) == 0,
                                "Input with frozen value has been already met: " + frozen_input_name);
        ng_op_map[frozen_input_name] = {frozen_input_value};
    }
    // create parameter nodes for all tensor places corresponding to inputs
    for (const auto& input_place : model_inputs) {
        FRONT_END_GENERAL_CHECK(input_place->get_names().size() == 1, "Input place must have one name.");
        auto input_name = input_place->get_names()[0];
        if (ng_op_map.count(input_name)) {
            // probably this input is frozen
            continue;
        }
        const auto& input_tensor_place = std::dynamic_pointer_cast<TensorPlace>(input_place);
        auto input_shape = input_tensor_place->get_partial_shape();
        auto input_type = input_tensor_place->get_element_type();

        // in case of cutting graph, types of custom inputs can be dynamic,
        // according to MO help, fp32 is used by default in such cases
        if (input_type == element::dynamic) {
            input_type = element::f32;
        }

        auto param = std::make_shared<ov::opset8::Parameter>(input_type, input_shape);
        set_node_name(input_name, param);
        params.push_back(param);
        ng_op_map[input_name] = {NamedOutput(param)};
    }

    // create the OV ops from TensorFlow ops
    for (const auto& operation_place : operation_places) {
        auto operation_decoder = operation_place->get_decoder();
        auto operation_name = operation_place->get_names()[0];
        // output for parameter nodes has been already generated
        if (ng_op_map.count(operation_name)) {
            continue;
        }

        // prepare a list of OV node inputs for each node
        ov::OutputVector ov_inputs;
        size_t operation_input_size = operation_decoder->get_input_size();

        if (operation_decoder->get_op_type() == "NextIteration") {
            // we expect no inputs for NextIteration because we break-up the cycle in InputModel
            operation_input_size = 0;
        }
        for (size_t input_port_idx = 0; input_port_idx < operation_input_size; ++input_port_idx) {
            // TODO: Implement more general approach. Skipping Constants that have input edges
            if (operation_decoder->get_op_type() == "Const") {
                break;
            }
            std::string producer_name;
            size_t producer_port_idx;
            try {
                std::string producer_port_name;
                operation_decoder->get_input_node(input_port_idx, producer_name, producer_port_name, producer_port_idx);
                if (!producer_port_name.empty()) {
                    producer_port_idx =
                        get_flat_index_by_name_and_id(ng_op_map[producer_name], producer_port_name, producer_port_idx);
                }
            } catch (const std::exception&) {
                FRONT_END_THROW("[ ERROR ] Exception happened when preparing input " + std::to_string(input_port_idx) +
                                " for op '" + operation_decoder->get_op_name() + "', expected input name: '" +
                                producer_name + "', expected input port index: " + std::to_string(producer_port_idx) +
                                '\n');
            }

            // skip conditional edges that must be resolved before operation translation
            // now we can meet them because we still work with TensorFlow protobuf
            if (is_conditional_edge(producer_name)) {
                continue;
            }

            // TODO: re-implement the logic below once Place graph structure is implemented
            // Using Place graph structure (OpPlace, In/OutPortPlace places and their connections) can give
            // names of ports and operations that can be used for further check about existence in ng_op_map

            // check if output vector for places have been already defined and the order of this check is important
            // it moves from places corresponding to input port of the current operation node to output port of original
            // producers
            if (ng_op_map.count(std::to_string(input_port_idx) + ":" + operation_name)) {
                const auto& input_outputs_vector = ng_op_map.at(std::to_string(input_port_idx) + ":" + operation_name);
                FRONT_END_GENERAL_CHECK(input_outputs_vector.size() == 1,
                                        "Input created with pruning must have one output");
                ov_inputs.push_back(input_outputs_vector.at(0).port);
            } else if (ng_op_map.count(producer_name + ":" + std::to_string(producer_port_idx))) {
                const auto& input_outputs_vector =
                    ng_op_map.at(producer_name + ":" + std::to_string(producer_port_idx));
                FRONT_END_GENERAL_CHECK(input_outputs_vector.size() == 1,
                                        "Input created with pruning must have one output");
                ov_inputs.push_back(input_outputs_vector.at(0).port);
            } else if (ng_op_map.count(producer_name)) {
                const auto& input_outputs_vector = ng_op_map.at(producer_name);
                if (input_outputs_vector.size() <= producer_port_idx) {
                    auto producer_node = input_outputs_vector[0].port.get_node_shared_ptr();
                    if (std::dynamic_pointer_cast<FrameworkNode>(producer_node)) {
                        // FrameworkNode node does not know in advance how many output ports will be used
                        // so we can increase number of outputs by demand
                        producer_node->set_output_type(producer_port_idx, element::dynamic, PartialShape::dynamic());
                        // update output vector in node map
                        ng_op_map[producer_name] = named_from_indexed(producer_node->outputs());
                    }
                }
                FRONT_END_GENERAL_CHECK(input_outputs_vector.size() > producer_port_idx,
                                        "Input created with pruning must have one output");
                ov_inputs.push_back(input_outputs_vector.at(producer_port_idx).port);
            } else {
                FRONT_END_GENERAL_CHECK(false,
                                        "No input is found for node \"" + operation_name + "\" by port " +
                                            std::to_string(producer_port_idx));
            }
        }

        // generate OV node output vector for the current operation node
        NamedOutputVector ov_outputs;
        auto operation_type = operation_decoder->get_op_type();
        if (m_translator_map->count(operation_type)) {
            try {
                auto translator = m_translator_map->at(operation_decoder->get_op_type());
                NodeContext node_context(operation_decoder, ov_inputs, this);
                ov_outputs = translator(node_context);
            } catch (const std::exception& ex) {
                // save the root-cause of the translation failure
                const auto fw_outs = create_fw_node_with_exception(operation_decoder,
                                                                   ov_inputs,
                                                                   operation_place->get_output_ports().size(),
                                                                   operation_name,
                                                                   ex.what());
                ov_outputs = named_from_indexed(fw_outs);
            } catch (...) {
                // save unknown exception type
                const auto fw_outs = create_fw_node_with_exception(operation_decoder,
                                                                   ov_inputs,
                                                                   operation_place->get_output_ports().size(),
                                                                   operation_name,
                                                                   "Unknown exception type");
                ov_outputs = named_from_indexed(fw_outs);
            }
        } else if (auto body_ov_model = get_body_ov_model(operation_type, ov_inputs)) {
            OutputVector indexed_ov_outputs;
            inject_body_model(body_ov_model, operation_type, ov_inputs, indexed_ov_outputs);

            // set output tensor names
            for (size_t idx = 0; idx < indexed_ov_outputs.size(); ++idx) {
                indexed_ov_outputs[idx].get_tensor().set_names({operation_name + ":" + std::to_string(idx)});
            }
            ov_outputs = named_from_indexed(indexed_ov_outputs);
        } else {
            // continue translation by replacing with FrameworkNode
            // for example, it helps auto-pruning to be triggered on later nodes
            auto fw_node = std::make_shared<FrameworkNode>(operation_decoder,
                                                           ov_inputs,
                                                           operation_place->get_output_ports().size());
            set_node_name(operation_name, fw_node);
            ov_outputs = named_from_indexed(fw_node->outputs());
        }

        // register OV node outputs in the map for new operation node
        for (const auto& output : ov_outputs) {
            if (auto result = as_type_ptr<ov::opset10::Result>(output.port.get_node_shared_ptr())) {
                // do not add RetVal type operation to ng_op_map
                results.push_back(result);
            } else {
                auto param = as_type_ptr<ov::opset8::Parameter>(output.port.get_node_shared_ptr());
                // avoid duplicating Parameter nodes if they are already in the Parameters vector
                if (param && std::find(params.begin(), params.end(), param) == params.end()) {
                    params.push_back(param);
                }
                ng_op_map[operation_name].push_back(output);
            }
        }
    }

    // create Result nodes for all model outputs
    if (results.empty()) {
        for (const auto& model_output : model_outputs) {
            auto model_output_tensor_place = std::dynamic_pointer_cast<TensorPlace>(model_output);
            auto model_output_name = model_output_tensor_place->get_names()[0];
            std::string operation_name;
            std::string port_type;
            size_t port_index;
            ov::frontend::tensorflow::extract_operation_name_and_port(model_output_name,
                                                                      operation_name,
                                                                      port_index,
                                                                      port_type);

            if (port_type == "none") {
                for (const auto& node_output : indexed_from_named(ng_op_map[operation_name])) {
                    auto result_node = std::make_shared<ov::opset8::Result>(node_output);
                    // to be aligned with Legacy Frontend we set a name along with output port index
                    // though, the Result name is not used in the OV API 2.0 but it is checked in MO args tests
                    result_node->set_friendly_name(model_output_name + ":0");
                    results.push_back(result_node);
                }
            } else if (port_type == "out") {
                const auto& node_outputs = indexed_from_named(ng_op_map[operation_name]);
                FRONT_END_GENERAL_CHECK(node_outputs.size() > port_index,
                                        "Output port with index " + std::to_string(port_index) + " of " +
                                            operation_name + "node specified as custom output does not exist");
                auto result_node = std::make_shared<ov::opset8::Result>(node_outputs[port_index]);
                result_node->set_friendly_name(model_output_name);
                results.push_back(result_node);
            } else if (port_type == "in") {
                // TODO: avoid this traversing by having a map for OpPlace objects, for example
                std::shared_ptr<OpPlace> operation_place = nullptr;
                for (const auto& op_place : operation_places) {
                    FRONT_END_GENERAL_CHECK(!op_place->get_names().empty(), "No names for OpPlace found.");
                    if (op_place->get_names()[0] == operation_name) {
                        operation_place = op_place;
                    }
                }
                FRONT_END_GENERAL_CHECK(operation_place, "There is no operation place with a name: " + operation_name);
                auto operation_decoder = operation_place->get_decoder();

                // get to know a producer node and by which its output port data is generated
                std::string producer_name;
                std::string producer_port_name;
                size_t producer_port_idx;
                try {
                    operation_decoder->get_input_node(port_index, producer_name, producer_port_name, producer_port_idx);
                    if (!producer_port_name.empty()) {
                        producer_port_idx = get_flat_index_by_name_and_id(ng_op_map[producer_name],
                                                                          producer_port_name,
                                                                          producer_port_idx);
                    }
                } catch (const std::exception&) {
                    FRONT_END_THROW("[ ERROR ] Exception happened when preparing input " + std::to_string(port_index) +
                                    " for op '" + operation_decoder->get_op_name() + "', expected input name: '" +
                                    producer_name +
                                    "', expected input port index: " + std::to_string(producer_port_idx) + '\n');
                }

                // add Result node for this producer output port
                const auto& node_outputs = indexed_from_named(ng_op_map[producer_name]);
                FRONT_END_GENERAL_CHECK(node_outputs.size() > producer_port_idx,
                                        "Output port with index " + std::to_string(producer_port_idx) + " of " +
                                            producer_name + "node specified as custom output does not exist");
                auto result_node = std::make_shared<ov::opset8::Result>(node_outputs[producer_port_idx]);
                // to be aligned with Legacy Frontend we set a name of the output tensor name
                // of the producer to the Result node
                // though, the Result name is not used in the OV API 2.0 but it is checked in MO args tests
                result_node->set_friendly_name(producer_name + ":" + std::to_string(producer_port_idx));
                results.push_back(result_node);
            }
        }
    }

    // TODO: it may be redundant step since models_output is filled in InputModel constructor
    // find all terminal nodes in OV graph to complete list of results
    if (results.empty()) {
        for (const auto& node_output_vector : ng_op_map) {
            for (size_t output_ind = 0; output_ind < node_output_vector.second.size(); ++output_ind) {
                auto output = node_output_vector.second[output_ind].port;
                if (output.get_target_inputs().empty() &&
                    !std::dynamic_pointer_cast<ov::opset8::Result>(output.get_node_shared_ptr())) {
                    auto model_output_name =
                        output.get_node_shared_ptr()->get_friendly_name() + ":" + std::to_string(output_ind);
                    auto result_node = std::make_shared<ov::opset8::Result>(output);
                    result_node->set_friendly_name(model_output_name);
                    results.push_back(result_node);
                }
            }
        }
    }

    if (saved_model_inputs.get() && saved_model_inputs->size() > 0) {
        for (auto param : params) {
            // If parameter isn't found in a map of known inputs - mark them as unused
            // and try to remove it in the normalize step later
            if (!apply_saved_model_names(param, saved_model_inputs)) {
                param->get_output_tensor(0).add_names({"saved_model_unused"});
            }
        }
    }

    if (saved_model_outputs.get() && saved_model_outputs->size() > 0) {
        for (auto result : results) {
            // If parameter isn't found in a map of known outputs - mark them as unused
            // and try to remove it in the normalize step later
            if (!apply_saved_model_names(result, saved_model_outputs)) {
                result->get_input_tensor(0).add_names({"saved_model_unused"});
            }
        }
    }

    // reorder Parameter and Result nodes according to the requested order
    // of input and output names from the original model
    // during translation and topologically sorting this order could be lost
    auto input_names = model_tf->get_input_names();
    auto output_names = model_tf->get_output_names();
    ov::ParameterVector ordered_params = reorder_ops_by_names(input_names, params);
    ov::ResultVector ordered_results = reorder_ops_by_names(output_names, results);

    ov_model = std::make_shared<ov::Model>(ordered_results, ordered_params, m_model_name);
}

std::shared_ptr<ov::Model> TranslateSession::get_body_ov_model(const std::string& body_graph_name,
                                                               const ov::OutputVector& ov_inputs,
                                                               bool clear_names) {
    std::shared_ptr<ov::Model> body_model = nullptr;
    auto input_model = std::dynamic_pointer_cast<InputModel>(m_input_model);
    std::vector<ov::PartialShape> input_shapes;
    input_shapes.reserve(ov_inputs.size());
    std::vector<ov::element::Type> input_types;
    input_types.reserve(ov_inputs.size());
    for (const auto& ov_input : ov_inputs) {
        input_shapes.push_back(ov_input.get_partial_shape());
        input_types.push_back(ov_input.get_element_type());
    }
    CachedBodyModelSignature body_model_signature{body_graph_name, input_shapes, input_types};

    if (m_cached_body_models->count(body_model_signature)) {
        // check if such body graph has been converted before
        // re-use it from the cache for further injection

        // create new instance of the required body model
        // since it will be modified by injection
        auto cached_body_model = m_cached_body_models->at(body_model_signature);
        body_model = cached_body_model->clone();
    } else if (auto body_input_model = input_model->get_body_input_model(body_graph_name)) {
        // set input shapes and types for InputModel of the body graph
        // it allows to get more optimized model after the conversion,
        // for example, to get less sub-graphs with ShapeOf and Convert operations
        auto inputs = body_input_model->get_inputs();
        size_t num_inputs = inputs.size();
        FRONT_END_GENERAL_CHECK(num_inputs == ov_inputs.size(),
                                "[TensorFlow Frontend] internal error: a number of external  and internal inputs for a "
                                "body graph mismatch");
        for (size_t input_ind = 0; input_ind < num_inputs; ++input_ind) {
            auto input_place = inputs[input_ind];
            if (input_types[input_ind].is_static()) {
                body_input_model->set_element_type(input_place, input_types[input_ind]);
            }
            if (input_shapes[input_ind].rank().is_static()) {
                body_input_model->set_partial_shape(input_place, input_shapes[input_ind]);
            }
        }

        // try to find a function by name in the model library
        translate_graph(body_input_model, body_model);
        // save new instance of body_model in the cache of body models
        // before its injection into the parent graph

        // before caching, erase tensor names from the body graph
        // otherwise, it can lead tensor names conflicts
        if (clear_names) {
            for (const auto& op : body_model->get_ordered_ops()) {
                for (size_t ind = 0; ind < op->get_output_size(); ++ind) {
                    op->get_output_tensor(ind).set_names({});
                }
            }
        }

        auto cached_body_model = body_model->clone();
        update_cached_body_models(body_model_signature, cached_body_model);
    }
    return body_model;
}
