#include <asset/detail/extension.hpp>

namespace vulture {

namespace detail {

StringView Extension(const String& path) {
  const char* extension{};
  for (auto& c : path) {
    if (c == '.') {
      extension = &c;
    }
  }
  return StringView{extension};
}

}  // namespace detail

}  // namespace vulture