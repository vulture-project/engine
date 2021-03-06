/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file component_holder.hpp
 * @date 2022-05-10
 *
 * The MIT License (MIT)
 * Copyright (c) vulture-project
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

#include "ecs/generator.hpp"

#include <cstdint>
#include <utility>

namespace vulture {

using ComponentTypeId = uint64_t;
using ComponentTypeIdGenerator = StaticIdGenerator<ComponentTypeId>;

class IComponentHolder {
 public:
  virtual ~IComponentHolder() = default;
};

template <typename ComponentT>
class ComponentHolderBase : public IComponentHolder {
 public:
  template <typename... Args>
  ComponentHolderBase(Args&&... args);

  ~ComponentHolderBase() override = default;

 protected:
  ComponentT component_;
};

template <typename ComponentT>
class ComponentHolder : public ComponentHolderBase<ComponentT> {
 public:
  template <typename... Args>
  ComponentHolder(Args&&... args);

  ComponentT* Get();

  static ComponentTypeId GetTypeId();

 private:
  static ComponentTypeId type_id_;
};

}  // namespace vulture

#include "ecs/component_holder.ipp"