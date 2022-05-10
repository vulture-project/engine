/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file view.hpp
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

#include "ecs/entity_handle.hpp"
#include "ecs/registry.hpp"

#include <unordered_map>

namespace vulture {

template <typename ComponentT>
class View {
 public:
  class Iterator {
   private:
    using EntityMapIterator = typename std::unordered_map<EntityId, IComponentHolder*>::iterator;

   public:
    Iterator(EntityMapIterator iterator, Registry& registry);

    Iterator& operator++();
    Iterator operator++(int unused);

    ComponentT* GetComponent() const;
    EntityHandle GetEntityHandle() const;

   public:
    friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
      return lhs.iterator_ == rhs.iterator_;
    }

    friend bool operator!=(const Iterator& lhs, const Iterator& rhs) {
      return lhs.iterator_ != rhs.iterator_;
    }

   private:
    EntityMapIterator iterator_;
    Registry& registry_;
  };

 public:
  View(EntityMap& entity_map, Registry& registry);

  Iterator Begin();

  Iterator End();

 private:
  EntityMap& entity_map_;
  Registry& registry_;
};

} // namespace vulture

#include "ecs/view.ipp"