#include <asset/detail/asset_loader_registrar.hpp>

#include <asset/loaders/glsl_loader.hpp>
#include <asset/loaders/jpeg_loader.hpp>
#include <asset/loaders/jpg_loader.hpp>
#include <asset/loaders/obj_loader.hpp>
#include <asset/loaders/png_loader.hpp>
#include <asset/loaders/tga_loader.hpp>

namespace vulture {

namespace detail {

static AssetLoaderRegistrar<GLSLLoader> kGLSLLoaderRegistrar{};

static AssetLoaderRegistrar<JPEGLoader> kJPEGLoaderRegistrar{};

static AssetLoaderRegistrar<JPGLoader> kJPGLoaderRegistrar{};

static AssetLoaderRegistrar<OBJLoader> kOBJLoaderRegistrar{};

static AssetLoaderRegistrar<PNGLoader> kPNGLoaderRegistrar{};

static AssetLoaderRegistrar<TGALoader> kTGALoaderRegistrar{};

}  // namespace detail

}  // namespace vulture