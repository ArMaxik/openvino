// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "openvino/op/op.hpp"

namespace ov {
namespace op {
namespace v3 {
/// \brief ROIAlign operation.
///
/// \ingroup ov_ops_cpp_api
class OPENVINO_API ROIAlign : public Op {
public:
    OPENVINO_OP("ROIAlign", "opset3", op::Op);
    enum class PoolingMode { AVG, MAX };

    ROIAlign() = default;
    /// \brief Constructs a ROIAlign node matching the ONNX ROIAlign specification
    ///
    /// \param input           Input feature map {N, C, H, W}
    /// \param rois            Regions of interest to pool over
    /// \param batch_indices   Indices of images in the batch matching
    ///                        the number or ROIs
    /// \param pooled_h        Height of the ROI output features
    /// \param pooled_w        Width of the ROI output features
    /// \param sampling_ratio  Number of sampling points used to compute
    ///                        an output element
    /// \param spatial_scale   Spatial scale factor used to translate ROI coordinates
    /// \param mode            Method of pooling - 'avg' or 'max'
    ROIAlign(const Output<Node>& input,
             const Output<Node>& rois,
             const Output<Node>& batch_indices,
             const int pooled_h,
             const int pooled_w,
             const int sampling_ratio,
             const float spatial_scale,
             const std::string& mode);

    ROIAlign(const Output<Node>& input,
             const Output<Node>& rois,
             const Output<Node>& batch_indices,
             const int pooled_h,
             const int pooled_w,
             const int sampling_ratio,
             const float spatial_scale,
             const PoolingMode mode);

    void validate_and_infer_types() override;
    bool visit_attributes(AttributeVisitor& visitor) override;
    std::shared_ptr<Node> clone_with_new_inputs(const OutputVector& new_args) const override;

    int get_pooled_h() const {
        return m_pooled_h;
    }

    void set_pooled_h(const int h) {
        m_pooled_h = h;
    }

    int get_pooled_w() const {
        return m_pooled_w;
    }
    void set_pooled_w(const int w) {
        m_pooled_w = w;
    }

    int get_sampling_ratio() const {
        return m_sampling_ratio;
    }

    void set_sampling_ratio(const int ratio) {
        m_sampling_ratio = ratio;
    }

    float get_spatial_scale() const {
        return m_spatial_scale;
    }

    void set_spatial_scale(const float scale) {
        m_spatial_scale = scale;
    }

    PoolingMode get_mode() const {
        return m_mode;
    }

    void set_mode(const PoolingMode mode) {
        m_mode = mode;
    }

    bool evaluate(TensorVector& outputs, const TensorVector& inputs) const override;
    bool has_evaluate() const override;

private:
    PoolingMode mode_from_string(const std::string& mode) const;

private:
    int m_pooled_h;
    int m_pooled_w;
    int m_sampling_ratio;
    float m_spatial_scale;
    PoolingMode m_mode;
};
}  // namespace v3

namespace v9 {
class OPENVINO_API ROIAlign : public Op {
public:
    OPENVINO_OP("ROIAlign", "opset9");
    enum class PoolingMode { AVG, MAX };
    enum class AlignedMode { ASYMMETRIC, HALF_PIXEL_FOR_NN, HALF_PIXEL };

    ROIAlign() = default;
    /// \brief Constructs a ROIAlign operation.
    ///
    /// \param input           Input feature map {N, C, H, W}
    /// \param rois            Regions of interest to pool over
    /// \param batch_indices   Indices of images in the batch matching
    ///                        the number or ROIs
    /// \param pooled_h        Height of the ROI output features
    /// \param pooled_w        Width of the ROI output features
    /// \param sampling_ratio  Number of sampling points used to compute
    ///                        an output element
    /// \param spatial_scale   Spatial scale factor used to translate ROI coordinates
    /// \param mode            Method of pooling - 'avg' or 'max'
    /// \param aligned_mode    Method of coordinates alignment - 'asymmetric', 'half_pixel_for_nn' or 'half_pixel'
    ROIAlign(const Output<Node>& input,
             const Output<Node>& rois,
             const Output<Node>& batch_indices,
             const int pooled_h,
             const int pooled_w,
             const int sampling_ratio,
             const float spatial_scale,
             const PoolingMode mode,
             const AlignedMode aligned_mode = AlignedMode::ASYMMETRIC);

    void validate_and_infer_types() override;
    bool visit_attributes(AttributeVisitor& visitor) override;
    std::shared_ptr<Node> clone_with_new_inputs(const OutputVector& new_args) const override;

    int get_pooled_h() const {
        return m_pooled_h;
    }

    void set_pooled_h(const int h) {
        m_pooled_h = h;
    }

    int get_pooled_w() const {
        return m_pooled_w;
    }
    void set_pooled_w(const int w) {
        m_pooled_w = w;
    }

    int get_sampling_ratio() const {
        return m_sampling_ratio;
    }

    void set_sampling_ratio(const int ratio) {
        m_sampling_ratio = ratio;
    }

    float get_spatial_scale() const {
        return m_spatial_scale;
    }

    void set_spatial_scale(const float scale) {
        m_spatial_scale = scale;
    }

    PoolingMode get_mode() const {
        return m_mode;
    }

    void set_mode(const PoolingMode mode) {
        m_mode = mode;
    }

    AlignedMode get_aligned_mode() const {
        return m_aligned_mode;
    }

    void set_aligned_mode(AlignedMode mode) {
        m_aligned_mode = mode;
    }

private:
    PoolingMode mode_from_string(const std::string& mode) const;
    int m_pooled_h;
    int m_pooled_w;
    int m_sampling_ratio;
    float m_spatial_scale;
    PoolingMode m_mode;
    AlignedMode m_aligned_mode;
};
}  // namespace v9
}  // namespace op

std::ostream& operator<<(std::ostream& s, const op::v3::ROIAlign::PoolingMode& mode);

template <>
class OPENVINO_API AttributeAdapter<op::v3::ROIAlign::PoolingMode>
    : public EnumAttributeAdapterBase<op::v3::ROIAlign::PoolingMode> {
public:
    AttributeAdapter(op::v3::ROIAlign::PoolingMode& value)
        : EnumAttributeAdapterBase<op::v3::ROIAlign::PoolingMode>(value) {}

    OPENVINO_RTTI("AttributeAdapter<ov::op::v3::ROIAlign::PoolingMode>");
};

std::ostream& operator<<(std::ostream& s, const op::v9::ROIAlign::PoolingMode& mode);

template <>
class OPENVINO_API AttributeAdapter<op::v9::ROIAlign::PoolingMode>
    : public EnumAttributeAdapterBase<op::v9::ROIAlign::PoolingMode> {
public:
    AttributeAdapter(op::v9::ROIAlign::PoolingMode& value)
        : EnumAttributeAdapterBase<op::v9::ROIAlign::PoolingMode>(value) {}

    OPENVINO_RTTI("AttributeAdapter<ov::op::v9::ROIAlign::PoolingMode>");
};

std::ostream& operator<<(std::ostream& s, const op::v9::ROIAlign::AlignedMode& mode);

template <>
class OPENVINO_API AttributeAdapter<op::v9::ROIAlign::AlignedMode>
    : public EnumAttributeAdapterBase<op::v9::ROIAlign::AlignedMode> {
public:
    AttributeAdapter(op::v9::ROIAlign::AlignedMode& value)
        : EnumAttributeAdapterBase<op::v9::ROIAlign::AlignedMode>(value) {}

    OPENVINO_RTTI("AttributeAdapter<ov::op::v9::ROIAlign::AlignedMode>");
};

}  // namespace ov
