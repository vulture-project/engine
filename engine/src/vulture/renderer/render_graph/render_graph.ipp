/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file render_graph.ipp
 * @date 2023-03-18
 * 
 * The MIT License (MIT)
 * Copyright (c) 2022 Nikita Mochalov
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/************************************************************************************************
 * Dynamic Texture Specification
 ************************************************************************************************/
namespace detail {

template <typename T>
DependentTextureValue<T>::DependentTextureValue(T independent_value) : independent_value_(independent_value) {}

template <typename T>
T DependentTextureValue<T>::Get() const {
  return IsDependent() ? dependent_value_ : independent_value_;
}

template <typename T>
bool DependentTextureValue<T>::IsDependent() const {
  return dependency_ != kInvalidTextureVersionId;
}

template <typename T>
void DependentTextureValue<T>::SetIndependentValue(T independent_value) {
  dependency_        = kInvalidTextureVersionId;
  independent_value_ = independent_value;
}

template <typename T>
void DependentTextureValue<T>::SetDependency(TextureVersionId dependency) {
  dependency_ = dependency;
}

template <typename T>
TextureVersionId DependentTextureValue<T>::GetDependency() const {
  return dependency_;
}

template <typename T>
void DependentTextureValue<T>::UpdateDependentValue(T dependent_value) {
  dependent_value_ = dependent_value;
}

}  // namespace detail

/************************************************************************************************
 * Render Graph
 ************************************************************************************************/
template <typename OutputDataT, typename RenderPassT, typename... RenderPassArgs>
OutputDataT& RenderGraph::AddPass(const std::string_view name, RenderPassArgs... args) {
  OutputDataT& output_data = blackboard_.Add<OutputDataT>();
  AddPass<RenderPassT>(name, std::forward<RenderPassArgs>(args)...);
  return output_data;
}

template <typename RenderPassT, typename... RenderPassArgs>
void RenderGraph::AddPass(const std::string_view name, RenderPassArgs... args) {
  PassNode& pass_node = pass_nodes_.emplace_back(name, new RenderPassT(std::forward<RenderPassArgs>(args)...));
  pass_node.subgraph_idx = cur_subgraph_idx_;
  pass_node.render_pass_id = GeneratePassIdFromString(pass_node.name);
}
