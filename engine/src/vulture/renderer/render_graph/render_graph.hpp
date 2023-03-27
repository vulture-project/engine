/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file render_graph.hpp
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

#pragma once

#include <ostream>
#include <string>
#include <vector>
#include <vulture/renderer/graphics_api/render_device.hpp>
#include <vulture/renderer/render_graph/blackboard.hpp>
#include <vulture/renderer/render_graph/render_graph_node.hpp>
#include <vulture/renderer/render_graph/render_pass.hpp>
#include <vulture/renderer/texture.hpp>

namespace vulture {
namespace rg {

/************************************************************************************************
 * Dynamic Texture Specification
 ************************************************************************************************/
namespace detail {

template <typename T>
class DependentTextureValue {
 public:
  DependentTextureValue() = default;
  DependentTextureValue(T independent_value);

  T Get() const;
  bool IsDependent() const;

  void SetIndependentValue(T independent_value);

  void SetDependency(TextureVersionId dependency);
  TextureVersionId GetDependency() const;

  void UpdateDependentValue(T dependent_value);  ///< Used by RenderGraph

 private:
  T independent_value_{};

  T dependent_value_{};  ///< Set and updated by render graph
  TextureVersionId dependency_{kInvalidTextureVersionId};
};

}  // namespace detail

struct DynamicTextureSpecification {
  detail::DependentTextureValue<DataFormat> format{DataFormat::kInvalid};
  TextureType                               type{TextureType::kTexture2D};
  TextureUsageFlags                         usage{kTextureUsageBitNone};
  bool                                      cpu_readable{false};

  detail::DependentTextureValue<uint32_t>   width{0};
  detail::DependentTextureValue<uint32_t>   height{0};
  detail::DependentTextureValue<uint32_t>   samples{1};                        
  uint32_t                                  mip_levels{1};

  DynamicTextureSpecification() = default;
  explicit DynamicTextureSpecification(const TextureSpecification& specification);
};

/************************************************************************************************
 * Render Graph
 ************************************************************************************************/
namespace detail {

struct TextureEntry {
  std::string                 name{"unnamed"};
  SharedPtr<Texture>          texture{nullptr};
  DynamicTextureSpecification  specification{};
  bool                        imported{false};
  TextureLayout               final_layout{TextureLayout::kUndefined};  ///< After the last use
  uint32_t                    ref_count{0};
  bool                        dirty{true};

  TextureEntry(const std::string_view name, SharedPtr<Texture> texture,
               const DynamicTextureSpecification& specification, bool imported, TextureLayout final_layout,
               uint32_t ref_count = 0, bool dirty = true)
      : name(name),
        texture(texture),
        specification(specification),
        imported(imported),
        final_layout(final_layout),
        ref_count(ref_count),
        dirty(dirty) {}
};

struct BuiltPass {
  RenderPassHandle           pass_handle{kInvalidRenderResourceHandle};
  RenderPassDescription      description{};

  FramebufferHandle          framebuffer_handle{kInvalidRenderResourceHandle};
  std::vector<TextureHandle> framebuffer_textures;

  std::vector<ClearValue>    clear_values{};
};

}  // namespace detail

class RenderGraph {
 public:
  RenderGraph() = default;

  RenderGraph(const RenderGraph& render_graph) = delete;
  RenderGraph& operator=(const RenderGraph& render_graph) = delete;

  /* Setup phase */
  void BeginSubgraph(const std::string_view name);
  void EndSubgraph();

  template <typename OutputDataT, typename RenderPassT, typename... RenderPassArgs>
  OutputDataT& AddPass(const std::string_view name, RenderPassArgs... args);

  template <typename RenderPassT, typename... RenderPassArgs>
  void AddPass(const std::string_view name, RenderPassArgs... args);

  TextureVersionId ImportTexture(const std::string_view name, SharedPtr<Texture> texture,
                                 const TextureSpecification& specification, TextureLayout final_layout);

  Blackboard& GetBlackboard();

  void Setup();

  /* Compile phase */
  void Compile(RenderDevice& device);

  /* Execute phase */
  void Execute(RenderDevice& device, CommandBuffer& command_buffer);

  /* Update phase */
  void ReimportTexture(TextureVersionId version_id, SharedPtr<Texture> texture,
                       const TextureSpecification& specification);

  /* Other */
  void Destroy(RenderDevice& device);
  void ExportGraphviz(std::ostream& os) const;

 private:
  void UpdateDependentTextureValues();
  void RecreateTransientTextures(RenderDevice& device);
  void RecreateRenderPasses(RenderDevice& device);
  void RecreateFramebuffers(RenderDevice& device);

  TextureVersionId NewEntry(const std::string_view name, SharedPtr<Texture> texture,
                            const DynamicTextureSpecification& specification, bool imported,
                            TextureLayout final_layout);

  TextureNode& AddTextureNode(uint32_t actual_texture_idx, uint32_t texture_version);

  detail::TextureEntry& GetTextureEntry(TextureVersionId id);
  const detail::TextureEntry& GetTextureEntry(TextureVersionId id) const;

  struct NextPassUsage {
    int32_t       pass_idx{-1};
    TextureLayout layout{TextureLayout::kUndefined};
  };

  NextPassUsage GetNextPassUsage(uint32_t pass_idx, TextureVersionId id);

  void ExportGraphvizSubgraph(std::ostream& os, int32_t subgraph_idx) const;

 private:
  Blackboard                        blackboard_;
  std::vector<PassNode>             pass_nodes_;
  std::vector<detail::BuiltPass>    built_passes_;

  std::vector<TextureNode>          texture_nodes_;    ///< Texture versions, indexed by TextureVersionId
  std::vector<detail::TextureEntry> texture_entries_;  ///< Actual Texture resources
  bool                              textures_dirty_{true};

  std::vector<std::string>          subgraph_names_;
  int32_t                           cur_subgraph_idx_{-1};

  friend class RenderGraphBuilder;
};

/************************************************************************************************
 * Render Graph Builder
 ************************************************************************************************/
class RenderGraphBuilder {
 public:
  RenderGraphBuilder(RenderGraph& graph, PassNode& pass_node);

  TextureVersionId CreateTexture(const std::string_view name, const DynamicTextureSpecification& specification,
                                 TextureLayout final_layout = TextureLayout::kUndefined);

  TextureVersionId SetDepthStencil(TextureVersionId texture,
                                   AttachmentLoad load = AttachmentLoad::kLoad,
                                   AttachmentStore store = AttachmentStore::kStore,
                                   ClearValue clear_value = ClearValue{1.0f, 0});

  TextureVersionId AddColorAttachment(TextureVersionId texture,
                                      AttachmentLoad load = AttachmentLoad::kLoad,
                                      AttachmentStore store = AttachmentStore::kStore,
                                      ClearValue clear_value = ClearValue{0, 0, 0, 0});

  TextureVersionId AddResolveAttachment(TextureVersionId texture,
                                        AttachmentLoad load = AttachmentLoad::kDontCare,
                                        AttachmentStore store = AttachmentStore::kStore,
                                        ClearValue clear_value = ClearValue{0, 0, 0, 0});

  void AddSampledTexture(TextureVersionId texture);

 private:
  RenderGraph& graph_;
  PassNode&    pass_node_;
};

#include <vulture/renderer/render_graph/render_graph.ipp>

}  // namespace rg
}  // namespace vulture
