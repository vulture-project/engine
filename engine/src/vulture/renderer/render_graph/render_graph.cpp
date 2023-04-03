/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file render_graph.cpp
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

#include <vulture/renderer/render_graph/render_graph.hpp>

using namespace vulture;
using namespace vulture::rg;

/************************************************************************************************
 * Dynamic Texture Specification
 ************************************************************************************************/
DynamicTextureSpecification::DynamicTextureSpecification(const TextureSpecification& specification)
    : format(specification.format),
      type(specification.type),
      usage(specification.usage),
      cpu_readable(specification.cpu_readable),
      width(specification.width),
      height(specification.height),
      samples(specification.samples),
      mip_levels(specification.mip_levels) {}

/************************************************************************************************
 * Render Graph
 ************************************************************************************************/
Blackboard& RenderGraph::GetBlackboard() { return blackboard_; }

TextureVersionId RenderGraph::ImportTexture(const std::string_view name, SharedPtr<Texture> texture,
                                            TextureLayout final_layout) {
  assert(texture);
  return NewEntry(name, texture, DynamicTextureSpecification(texture->GetSpecification()), true, final_layout);
}

void RenderGraph::Setup() {
  for (auto& pass_node : pass_nodes_) {
    RenderGraphBuilder builder{*this, pass_node};

    cur_subgraph_idx_ = pass_node.subgraph_idx;
    pass_node.render_pass->Setup(builder, blackboard_, pass_node.render_pass_id);
    cur_subgraph_idx_ = -1;
  }
}

void RenderGraph::Compile(RenderDevice& device) {
  UpdateDependentTextureValues();
  RecreateTransientTextures(device);
  RecreateRenderPasses(device);
  RecreateFramebuffers(device);
}

void RenderGraph::Execute(RenderDevice& device, CommandBuffer& command_buffer) {
  if (textures_dirty_) {
    UpdateDependentTextureValues();
    RecreateTransientTextures(device);
    RecreateRenderPasses(device);
    RecreateFramebuffers(device);
  }

  for (uint32_t i = 0; i < built_passes_.size(); ++i) {
    const auto& pass_node  = pass_nodes_[i];
    const auto& built_pass = built_passes_[i];

    uint32_t width  = 0;
    uint32_t height = 0;
    if (!pass_node.color_attachment_usages.empty()) {
      const detail::TextureEntry& texture = GetTextureEntry(pass_node.color_attachment_usages[0].out);
      width  = texture.specification.width.Get();
      height = texture.specification.height.Get();
    } else if (pass_node.depth_stencil_usage.has_value()) {
      const detail::TextureEntry& texture = GetTextureEntry(pass_node.depth_stencil_usage->out);
      width  = texture.specification.width.Get();
      height = texture.specification.height.Get();
    } else {
      assert(!"No color or depth stencil attachments found!");
    }

    CommandBuffer::RenderPassBeginInfo render_pass_begin_info{};
    render_pass_begin_info.render_pass        = built_pass.pass_handle;
    render_pass_begin_info.framebuffer        = built_pass.framebuffer_handle;
    render_pass_begin_info.render_area        = RenderArea{0, 0, width, height};
    render_pass_begin_info.clear_values_count = built_pass.clear_values.size();
    render_pass_begin_info.clear_values       = built_pass.clear_values.data();

    command_buffer.RenderPassBegin(render_pass_begin_info);

    Viewport viewport{};
    viewport.x         = 0;
    viewport.y         = static_cast<float>(height);
    viewport.width     = static_cast<float>(width);
    viewport.height    = -static_cast<float>(height);
    viewport.min_depth = 0.0f;
    viewport.max_depth = 1.0f;
    // Viewport viewport{};
    // viewport.x         = 0;
    // viewport.y         = 0;
    // viewport.width     = static_cast<float>(width);
    // viewport.height    = static_cast<float>(height);
    // viewport.min_depth = 0.0f;
    // viewport.max_depth = 1.0f;
    command_buffer.CmdSetViewports(1, &viewport);

    pass_node.render_pass->Execute(command_buffer, blackboard_, pass_node.render_pass_id, built_pass.pass_handle);

    command_buffer.RenderPassEnd();
  }
}

void RenderGraph::ReimportTexture(TextureVersionId version, SharedPtr<Texture> texture) {
  assert(version != kInvalidTextureVersionId);
  assert(texture);

  const TextureSpecification& specification = texture->GetSpecification();

  detail::TextureEntry& entry = GetTextureEntry(version);
  entry.dirty |= entry.specification.format.Get()  != specification.format ||
                 entry.specification.type          != specification.type ||
                 entry.specification.usage         != specification.usage ||
                 entry.specification.cpu_readable  != specification.cpu_readable ||
                 entry.specification.width.Get()   != specification.width ||
                 entry.specification.height.Get()  != specification.height ||
                 entry.specification.samples.Get() != specification.samples ||
                 entry.specification.mip_levels    != specification.mip_levels;

  if (entry.dirty) {
    textures_dirty_ = true;
  }

  entry.specification = DynamicTextureSpecification{texture->GetSpecification()};
  entry.texture      = texture;
}

void RenderGraph::UpdateDependentTextureValues() {
  for (auto& entry : texture_entries_) {
    DynamicTextureSpecification& specification = entry.specification;

    if (specification.format.IsDependent()) {
      auto dependency_format = GetTextureEntry(specification.format.GetDependency()).specification.format;

      if (specification.format.Get() != dependency_format.Get()) {
        specification.format.UpdateDependentValue(dependency_format.Get());
        entry.dirty = true;
      }
    }

    if (specification.width.IsDependent()) {
      auto dependency_width = GetTextureEntry(specification.width.GetDependency()).specification.width;

      if (specification.width.Get() != dependency_width.Get()) {
        specification.width.UpdateDependentValue(dependency_width.Get());
        entry.dirty = true;
      }
    }

    if (specification.height.IsDependent()) {
      auto dependency_height = GetTextureEntry(specification.height.GetDependency()).specification.height;

      if (specification.height.Get() != dependency_height.Get()) {
        specification.height.UpdateDependentValue(dependency_height.Get());
        entry.dirty = true;
      }
    }

    if (specification.samples.IsDependent()) {
      auto dependency_samples = GetTextureEntry(specification.samples.GetDependency()).specification.samples;

      if (specification.samples.Get() != dependency_samples.Get()) {
        specification.samples.UpdateDependentValue(dependency_samples.Get());
        entry.dirty = true;
      }
    }

    if (entry.dirty) {
      textures_dirty_ = true;
    }
  }
}

void RenderGraph::RecreateTransientTextures(RenderDevice& device) {
  for (auto& entry : texture_entries_) {
    if (entry.dirty && !entry.imported) {
      TextureSpecification specification{};
      specification.format       = entry.specification.format.Get();
      specification.type         = entry.specification.type;
      specification.usage        = entry.specification.usage;
      specification.cpu_readable = entry.specification.cpu_readable;
      specification.width        = entry.specification.width.Get();
      specification.height       = entry.specification.height.Get();
      specification.samples      = entry.specification.samples.Get();
      specification.mip_levels   = entry.specification.mip_levels;

      if (entry.texture) {
        entry.texture->Recreate(specification);
      } else {
        entry.texture = CreateShared<Texture>(device, specification);
      }

      entry.dirty = false;
    }
  }

  textures_dirty_ = false;
}

void RenderGraph::RecreateRenderPasses(RenderDevice& device) {
  built_passes_.resize(pass_nodes_.size());
  
  for (uint32_t i = 0; i < pass_nodes_.size(); ++i) {
    const auto& pass_node  = pass_nodes_[i];
    auto&       built_pass = built_passes_[i];

    built_pass.description.subpasses.resize(1);

    SubpassDescription& subpass = built_pass.description.subpasses[0];
    subpass.bind_point = SubpassPipelineBindPoint::kGraphics;
    subpass.color_attachments.resize(pass_node.color_attachment_usages.size());
    subpass.resolve_attachments.resize(pass_node.resolve_attachment_usages.size());

    /* Attachments */
    uint32_t attachments_count = (pass_node.depth_stencil_usage.has_value() ? 1 : 0) +
                                 pass_node.color_attachment_usages.size() + pass_node.resolve_attachment_usages.size();

    built_pass.description.attachments.resize(attachments_count);
    built_pass.framebuffer_textures.resize(attachments_count);
    built_pass.clear_values.resize(attachments_count);

    uint32_t attachment = 0;
    if (pass_node.depth_stencil_usage.has_value()) {
      const detail::TextureEntry& entry = GetTextureEntry(pass_node.depth_stencil_usage->in);

      built_pass.framebuffer_textures[attachment] = entry.texture->GetHandle();
      built_pass.clear_values[attachment]         = pass_node.depth_stencil_usage->clear_value;

      TextureLayout initial_layout = TextureLayout::kUndefined;
      if (pass_node.depth_stencil_usage->load == AttachmentLoad::kLoad) {
        initial_layout = TextureLayout::kDepthStencilAttachment;
      }

      TextureLayout final_layout = TextureLayout::kUndefined;

      NextPassUsage next_pass_usage = GetNextPassUsage(i, pass_node.depth_stencil_usage->out);
      if (next_pass_usage.pass_idx != -1) {
        final_layout = next_pass_usage.layout;
      } else if (entry.final_layout != TextureLayout::kUndefined) {
        final_layout = entry.final_layout;
      } else {
        final_layout = TextureLayout::kGeneral;
      }

      AttachmentDescription attachment_description{};
      attachment_description.format         = entry.specification.format.Get();   // FIXME: dynamic
      attachment_description.samples        = entry.specification.samples.Get();  // FIXME: dynamic
      attachment_description.load_op        = pass_node.depth_stencil_usage->load;
      attachment_description.store_op       = pass_node.depth_stencil_usage->store;
      attachment_description.initial_layout = initial_layout;
      attachment_description.final_layout    = final_layout;
      built_pass.description.attachments[attachment] = attachment_description;

      subpass.depth_stencil_attachment = AttachmentReference{attachment, TextureLayout::kDepthStencilAttachment};

      ++attachment;
    }

    uint32_t color_attachment_idx = 0;
    for (const auto& color_attachment_usage : pass_node.color_attachment_usages) {
      const detail::TextureEntry& entry = GetTextureEntry(color_attachment_usage.in);

      built_pass.framebuffer_textures[attachment] = entry.texture->GetHandle();
      built_pass.clear_values[attachment]         = color_attachment_usage.clear_value;

      TextureLayout initial_layout = TextureLayout::kUndefined;
      if (color_attachment_usage.load == AttachmentLoad::kLoad) {
        initial_layout = TextureLayout::kColorAttachment;
      }

      TextureLayout final_layout = TextureLayout::kUndefined;

      NextPassUsage next_pass_usage = GetNextPassUsage(i, color_attachment_usage.out);
      if (next_pass_usage.pass_idx != -1) {
        final_layout = next_pass_usage.layout;
      } else if (entry.final_layout != TextureLayout::kUndefined) {
        final_layout = entry.final_layout;
      } else {
        final_layout = TextureLayout::kGeneral;
      }

      AttachmentDescription attachment_description{};
      attachment_description.format         = entry.specification.format.Get();   // FIXME: dynamic
      attachment_description.samples        = entry.specification.samples.Get();  // FIXME: dynamic
      attachment_description.load_op        = color_attachment_usage.load;
      attachment_description.store_op       = color_attachment_usage.store;
      attachment_description.initial_layout = initial_layout;
      attachment_description.final_layout    = final_layout;
      built_pass.description.attachments[attachment] = attachment_description;

      subpass.color_attachments[color_attachment_idx] =
          AttachmentReference{attachment, TextureLayout::kColorAttachment};

      ++color_attachment_idx;
      ++attachment;
    }

    uint32_t resolve_attachment_idx = 0;
    for (const auto& resolve_attachment_usage : pass_node.resolve_attachment_usages) {
      const detail::TextureEntry& entry = GetTextureEntry(resolve_attachment_usage.in);

      built_pass.framebuffer_textures[attachment] = entry.texture->GetHandle();
      built_pass.clear_values[attachment]         = resolve_attachment_usage.clear_value;

      TextureLayout initial_layout = TextureLayout::kUndefined;
      if (resolve_attachment_usage.load == AttachmentLoad::kLoad) {
        initial_layout = TextureLayout::kColorAttachment;
      }

      TextureLayout final_layout = TextureLayout::kUndefined;

      NextPassUsage next_pass_usage = GetNextPassUsage(i, resolve_attachment_usage.out);
      if (next_pass_usage.pass_idx != -1) {
        final_layout = next_pass_usage.layout;
      } else if (entry.final_layout != TextureLayout::kUndefined) {
        final_layout = entry.final_layout;
      } else {
        final_layout = TextureLayout::kGeneral;
      }

      AttachmentDescription attachment_description{};
      attachment_description.format         = entry.specification.format.Get();   // FIXME: dynamic
      attachment_description.samples        = entry.specification.samples.Get();  // FIXME: dynamic
      attachment_description.load_op        = resolve_attachment_usage.load;
      attachment_description.store_op       = resolve_attachment_usage.store;
      attachment_description.initial_layout = initial_layout;
      attachment_description.final_layout    = final_layout;
      built_pass.description.attachments[attachment] = attachment_description;

      subpass.resolve_attachments[resolve_attachment_idx] =
          AttachmentReference{attachment, TextureLayout::kColorAttachment};

      ++resolve_attachment_idx;
      ++attachment;
    }

    if (ValidRenderHandle(built_pass.pass_handle)) {
      device.DeleteRenderPass(built_pass.pass_handle);
    }

    built_pass.pass_handle = device.CreateRenderPass(built_pass.description);
  }
}

void RenderGraph::RecreateFramebuffers(RenderDevice& device) {
  for (auto& built_pass : built_passes_) {
    if (ValidRenderHandle(built_pass.framebuffer_handle)) {
      device.DeleteFramebuffer(built_pass.framebuffer_handle);
    }

    built_pass.framebuffer_handle = device.CreateFramebuffer(built_pass.framebuffer_textures, built_pass.pass_handle);
  }
}

TextureVersionId RenderGraph::NewEntry(const std::string_view name, SharedPtr<Texture> texture,
                                       const DynamicTextureSpecification& specification, bool imported,
                                       TextureLayout final_layout) {
  texture_entries_.emplace_back(name, texture, specification, imported, final_layout, 0, imported ? false : true);

  TextureNode& texture_node = AddTextureNode(texture_entries_.size() - 1, 0);
  texture_node.subgraph_idx = cur_subgraph_idx_;

  return texture_node.version_id;
}

TextureNode& RenderGraph::AddTextureNode(uint32_t actual_texture_idx, uint32_t texture_version) {
  TextureNode& texture_node       = texture_nodes_.emplace_back();
  texture_node.version_id         = texture_nodes_.size() - 1;
  texture_node.actual_texture_idx = actual_texture_idx;
  texture_node.version_num        = texture_version;

  ++texture_entries_[actual_texture_idx].ref_count;

  return texture_node;
}

rg::detail::TextureEntry& RenderGraph::GetTextureEntry(TextureVersionId id) {
  assert(id != kInvalidTextureVersionId);
  return texture_entries_[texture_nodes_[id].actual_texture_idx];
}

const rg::detail::TextureEntry& RenderGraph::GetTextureEntry(TextureVersionId id) const {
  assert(id != kInvalidTextureVersionId);
  return texture_entries_[texture_nodes_[id].actual_texture_idx];
}

RenderGraph::NextPassUsage RenderGraph::GetNextPassUsage(uint32_t pass_idx, TextureVersionId id) {
  for (uint32_t cur_pass_idx = pass_idx + 1; cur_pass_idx < pass_nodes_.size(); ++cur_pass_idx) {
    const auto& pass_node = pass_nodes_[cur_pass_idx];

    if (pass_node.depth_stencil_usage.has_value() && pass_node.depth_stencil_usage->in == id) {
      return NextPassUsage{static_cast<int32_t>(cur_pass_idx), TextureLayout::kDepthStencilAttachment};
    }

    for (const auto& color_attachment_usage : pass_node.color_attachment_usages) {
      if (color_attachment_usage.in == id) {
        return NextPassUsage{static_cast<int32_t>(cur_pass_idx), TextureLayout::kColorAttachment};
      }
    }

    for (const auto& resolve_attachment_usage : pass_node.resolve_attachment_usages) {
      if (resolve_attachment_usage.in == id) {
        return NextPassUsage{static_cast<int32_t>(cur_pass_idx), TextureLayout::kColorAttachment};
      }
    }

    for (const auto& sampler_id : pass_node.sampled_texture_ids) {
      if (sampler_id == id) {
        return NextPassUsage{static_cast<int32_t>(cur_pass_idx), TextureLayout::kShaderReadOnly};
      }
    }
  }

  return NextPassUsage{-1, TextureLayout::kUndefined};
}

void RenderGraph::BeginSubgraph(const std::string_view name) {
  subgraph_names_.emplace_back(name);
  cur_subgraph_idx_ = subgraph_names_.size() - 1;
}

void RenderGraph::EndSubgraph() {
  cur_subgraph_idx_ = -1;
}

std::ostream& ExportGraphvizWriteColor(std::ostream& os, ClearValue clear_value, bool is_depth) {
  if (is_depth) {
    fmt::print(os, "clear_depth = {0}", clear_value.depth_stencil.depth);
  } else {
    fmt::print(os, "clear_rgba = ({0}, {1}, {2}, {3})",
               clear_value.color.rgba_float32[0],
               clear_value.color.rgba_float32[1],
               clear_value.color.rgba_float32[2],
               clear_value.color.rgba_float32[3]);
  }

  return os;
};

void RenderGraph::ExportGraphviz(std::ostream& os) const {
  fmt::print(os, "digraph {{\n"
                 "graph [style=invis, rankdir=\"LR\" ordering=out, splines=spline]\n"
                 "node [shape=record, fontname=\"helvetica\", fontsize=14, margin=\"0.2,0.15\"]\n\n");

  /* Subgraphs */
  for (int32_t subgraph_idx = 0; subgraph_idx < static_cast<int32_t>(subgraph_names_.size()); ++subgraph_idx) {
    ExportGraphvizSubgraph(os, subgraph_idx);
  }

  ExportGraphvizSubgraph(os, -1);

  /* Connections */
  for (uint32_t pass_idx = 0; pass_idx < pass_nodes_.size(); ++pass_idx) {
    const PassNode& pass_node = pass_nodes_[pass_idx];

    /* Writes */
    if (pass_node.depth_stencil_usage.has_value()) {
      fmt::print(os,
                 "P{0} -> T{1} ["
                 "label=\"Depth Stencil, store_op = {2}\""
                 "fontcolor=orangered color=orangered"
                 "]\n",
                 pass_idx,
                 pass_node.depth_stencil_usage->out,
                 AttachmentStoreToStr(pass_node.depth_stencil_usage->store));
    }

    uint32_t attachment_idx = 0;
    for (const auto& color_attachment_usage : pass_node.color_attachment_usages) {
      fmt::print(os,
                 "P{0} -> T{1} ["
                 "label=\"Color [{2}], store_op = {3}\""
                 "fontcolor=orangered color=orangered"
                 "]\n",
                 pass_idx,
                 color_attachment_usage.out,
                 attachment_idx++,
                 AttachmentStoreToStr(color_attachment_usage.store));
    }

    attachment_idx = 0;
    for (const auto& resolve_attachment_usage : pass_node.resolve_attachment_usages) {
      fmt::print(os,
                 "P{0} -> T{1} ["
                 "label=\"Resolve [{2}], store_op = {3}\""
                 "fontcolor=orangered color=orangered"
                 "]\n",
                 pass_idx,
                 resolve_attachment_usage.out,
                 attachment_idx++,
                 AttachmentStoreToStr(resolve_attachment_usage.store));
    }

    // TODO: (tralf-strues) rewrite the remaining std::ostream usage to fmt

    /* Reads */
    if (pass_node.depth_stencil_usage.has_value()) {
      os << "T" << pass_node.depth_stencil_usage->in << " -> "
         << "P" << pass_idx << " [label=\""
         << "load_op = " << AttachmentLoadToStr(pass_node.depth_stencil_usage->load);
      
      if (pass_node.depth_stencil_usage->load == AttachmentLoad::kClear) {
        os << ", ";
        ExportGraphvizWriteColor(os, pass_node.depth_stencil_usage->clear_value, true);
      }

      os << "\" "
         << "fontcolor="
         << "gray"
         << " color="
         << "gray"
         << " style="
         << "dashed"
         << "]" << std::endl;
    }

    for (const auto& color_attachment_usage : pass_node.color_attachment_usages) {
      os << "T" << color_attachment_usage.in << " -> "
         << "P" << pass_idx << " [label=\""
         << "load_op = " << AttachmentLoadToStr(color_attachment_usage.load);
      
      if (color_attachment_usage.load == AttachmentLoad::kClear) {
        os << ", ";
        ExportGraphvizWriteColor(os, color_attachment_usage.clear_value, false);
      }

      os << "\" "
         << "fontcolor="
         << "gray"
         << " color="
         << "gray"
         << " style="
         << "dashed"
         << "]" << std::endl;
    }

    for (const auto& resolve_attachment_usage : pass_node.resolve_attachment_usages) {
      os << "T" << resolve_attachment_usage.in << " -> "
         << "P" << pass_idx << " [label=\""
         << "load_op = " << AttachmentLoadToStr(resolve_attachment_usage.load);
      
      if (resolve_attachment_usage.load == AttachmentLoad::kClear) {
        os << ", ";
        ExportGraphvizWriteColor(os, resolve_attachment_usage.clear_value, false);
      }

      os << "\" "
         << "fontcolor="
         << "gray"
         << " color="
         << "gray"
         << " style="
         << "dashed"
         << "]" << std::endl;
    }

    for (const auto& sampled_texture_id : pass_node.sampled_texture_ids) {
      os << "T" << sampled_texture_id << " -> "
         << "P" << pass_idx << " [label=\""
         << "Sample"
         << "\" "
         << "fontcolor="
         << "olivedrab3"
         << " color="
         << "olivedrab3"
         << "]" << std::endl;
    }
  }
  os << std::endl;

  /* Invisible edges between render passes */
  for (int32_t i = 1, prev_first_subgraph_node_ = 0; i < static_cast<int32_t>(pass_nodes_.size()); ++i) {
    if (pass_nodes_[i - 1].subgraph_idx == pass_nodes_[i].subgraph_idx) {
      os << "P" << i - 1 << " -> P" << i << " [style=invis]" << std::endl;
    } else {
      os << "P" << prev_first_subgraph_node_ << " -> P" << i << " [style=invis]" << std::endl;

      prev_first_subgraph_node_ = i;
    }
  }

  os << "}" << std::endl;
}

void RenderGraph::ExportGraphvizSubgraph(std::ostream& os, int32_t subgraph_idx) const {
  if (subgraph_idx != -1) {
    os << "subgraph cluster_" << subgraph_idx << " {" << std::endl
      << "label=<<B>" << subgraph_names_[subgraph_idx] << "</B>>;" << std::endl
      << "margin=\"36,108\";" << std::endl
      << "style=dashed;" << std::endl
      << "bgcolor=gray98;" << std::endl
      << "fontsize=36;" << std::endl
      << "fontname=\"helvetica\";" << std::endl
      << "color=black;" << std::endl;
  }

  /* Pass Nodes */
  for (uint32_t pass_idx = 0; pass_idx < pass_nodes_.size(); ++pass_idx) {
    const PassNode& pass_node = pass_nodes_[pass_idx];

    if (pass_node.subgraph_idx == subgraph_idx) {
      os << "P" << pass_idx << " [label=<{[" << pass_idx << "] <B>" << pass_node.name
        << "  </B>"
        // << "  </B><BR/>"
        // << "ID: "
        // << std::hex
        // << pass_node.render_pass_id
        // << std::dec
        << "}> style=\"filled\", fillcolor="
        << "goldenrod1"
        << ", fontsize=" << 28 << "]" << std::endl;
    }
  }
  os << std::endl;

  /* Texture Nodes */
  for (const auto& texture_node : texture_nodes_) {
    const detail::TextureEntry& entry = GetTextureEntry(texture_node.version_id);

    if (texture_node.subgraph_idx == subgraph_idx) {
      os << "T" << texture_node.version_id << " [label=<{ {<B>" << entry.name << "</B>"
         << " <FONT>(v." << texture_node.version_num << ")</FONT>"
         << "<BR/><BR/>"
         << entry.specification.width.Get() << "x" << entry.specification.height.Get();

      if (entry.specification.width.IsDependent()) {
        os << " (<U>" << GetTextureEntry(entry.specification.width.GetDependency()).name << "</U>)";
      }

      os << "<BR/> Samples: " << entry.specification.samples.Get();

      if (entry.specification.samples.IsDependent()) {
        os << " (<U>" << GetTextureEntry(entry.specification.samples.GetDependency()).name << "</U>)";
      }

      os << "<BR/><BR/>" << DataFormatToStr(entry.specification.format.Get());

      if (entry.specification.format.IsDependent()) {
        os << " (<U>" << GetTextureEntry(entry.specification.format.GetDependency()).name << "</U>)";
      }

      os << "} | {"
         << "Handle: " << entry.texture->GetHandle() << "<BR/><BR/>"
         << "Id: " << texture_node.actual_texture_idx << "<BR/>"
         << "Refs : " << entry.ref_count
         << (entry.imported ? "<BR/><BR/><B>[Imported]</B>" : "")
         << "} }> style=\"rounded,filled\", fillcolor="
         << (entry.imported ? "indianred1" : "lightsteelblue") << "]" << std::endl;
    }
  }

  if (subgraph_idx != -1) {
    os << "}" << std::endl;
  }

  os << std::endl;
}

/************************************************************************************************
 * Render Graph Builder
 ************************************************************************************************/
RenderGraphBuilder::RenderGraphBuilder(RenderGraph& graph, PassNode& pass_node)
    : graph_(graph), pass_node_(pass_node) {}

TextureVersionId RenderGraphBuilder::CreateTexture(const std::string_view name,
                                                   const DynamicTextureSpecification& specification,
                                                   TextureLayout final_layout) {
  return graph_.NewEntry(name, nullptr, specification, false, final_layout);
}

TextureVersionId RenderGraphBuilder::SetDepthStencil(TextureVersionId texture_version_id,
                                                     AttachmentLoad load,
                                                     AttachmentStore store,
                                                     ClearValue clear_value) {
  assert(texture_version_id != kInvalidTextureVersionId);

  TextureNode& in_texture_node = graph_.texture_nodes_[texture_version_id];
  TextureNode& out_texture_node =
      graph_.AddTextureNode(in_texture_node.actual_texture_idx, in_texture_node.version_num + 1);
  out_texture_node.subgraph_idx = graph_.cur_subgraph_idx_;

  pass_node_.depth_stencil_usage = {texture_version_id, out_texture_node.version_id, load, store, clear_value};

  return out_texture_node.version_id;
}

TextureVersionId RenderGraphBuilder::AddColorAttachment(TextureVersionId texture_version_id,
                                                        AttachmentLoad load,
                                                        AttachmentStore store,
                                                        ClearValue clear_value) {
  assert(texture_version_id != kInvalidTextureVersionId);

  TextureNode& in_texture_node = graph_.texture_nodes_[texture_version_id];
  TextureNode& out_texture_node =
      graph_.AddTextureNode(in_texture_node.actual_texture_idx, in_texture_node.version_num + 1);
  out_texture_node.subgraph_idx = graph_.cur_subgraph_idx_;

  pass_node_.color_attachment_usages.push_back(
      {texture_version_id, out_texture_node.version_id, load, store, clear_value});

  return out_texture_node.version_id;
}

TextureVersionId RenderGraphBuilder::AddResolveAttachment(TextureVersionId texture_version_id,
                                                          AttachmentLoad load,
                                                          AttachmentStore store,
                                                          ClearValue clear_value) {
  assert(texture_version_id != kInvalidTextureVersionId);

  TextureNode& in_texture_node = graph_.texture_nodes_[texture_version_id];
  TextureNode& out_texture_node =
      graph_.AddTextureNode(in_texture_node.actual_texture_idx, in_texture_node.version_num + 1);
  out_texture_node.subgraph_idx = graph_.cur_subgraph_idx_;

  pass_node_.resolve_attachment_usages.push_back(
      {texture_version_id, out_texture_node.version_id, load, store, clear_value});

  return out_texture_node.version_id;
}

void RenderGraphBuilder::AddSampledTexture(TextureVersionId texture_version_id) {
  assert(texture_version_id != kInvalidTextureVersionId);

  pass_node_.sampled_texture_ids.push_back(texture_version_id);
}