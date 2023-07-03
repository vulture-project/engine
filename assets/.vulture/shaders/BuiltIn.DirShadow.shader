name: "BuiltIn.DirShadow"
target_render_pass: "Cascaded Shadow Map Pass"

descriptor_sets: [Frame, View]

vert_shader: ["assets/.vulture/shaders/BuiltIn.DirShadow.vert", "assets/.vulture/shaders/BuiltIn.DirShadow.vert.spv"]
frag_shader: ["assets/.vulture/shaders/BuiltIn.DirShadow.frag", "assets/.vulture/shaders/BuiltIn.DirShadow.frag.spv"]

# Vertex Format
vertex_format: Vertex3D           # default: Vertex3D
topology: TriangleList            # default: TriangleList

# Rasterization
cull: FrontOnly                   # default: BackOnly
front_face: CounterClockwise      # default: CounterClockwise
polygon_mode: Fill                # default: Fill

# Depth Test
depth_test_enable: true           # default: true
depth_write_enable: true          # default: true
depth_compare: Less               # default: Less

# Blend (alpha-blending by default, but disabled)
blend_enable: false               # default: false

blend_src_color_factor: SrcAlpha  # default: SrcAlpha
blend_dst_color_factor: DstAlpha  # default: DstAlpha
blend_color_operation: Add        # default: Add

blend_src_alpha_factor: SrcAlpha  # default: One
blend_dst_alpha_factor: DstAlpha  # default: Zero
blend_alpha_operation: Add        # default: Add