/**
 * @author Viktor Baranov (github.com/baranov-V-V)
 * @file view.ipp
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

namespace vulture {

template <typename ComponentT>
View<ComponentT>::Iterator::Iterator(EntityMapIterator iterator, EntityRegistry& registry)
    : iterator_{iterator},
      registry_{registry} {
}

template <typename ComponentT>
typename View<ComponentT>::Iterator& View<ComponentT>::Iterator::operator++() {
  ++iterator_;
  return *this;
}

template <typename ComponentT>
typename View<ComponentT>::Iterator View<ComponentT>::Iterator::operator++(int unused) {
  Iterator temp{*this};
  ++(*this);
  return temp;
}

template <typename ComponentT>
ComponentT* View<ComponentT>::Iterator::GetComponent() const {
  ComponentHolder<ComponentT>* holder = reinterpret_cast<ComponentHolder<ComponentT>*>(iterator_->second);
  assert(holder != nullptr);
  return holder->Get();
}

template <typename ComponentT>
EntityHandle View<ComponentT>::Iterator::GetEntityHandle() const {
  return EntityHandle{iterator_->first, registry_};
}

template <typename ComponentT>
View<ComponentT>::View(EntityMap& entity_map, EntityRegistry& registry) 
    : entity_map_{entity_map},
      registry_{registry} {
}

template <typename ComponentT>
typename View<ComponentT>::Iterator View<ComponentT>::Begin() {
  return Iterator{entity_map_.begin(), registry_}; 
}

template <typename ComponentT>
typename View<ComponentT>::Iterator View<ComponentT>::End() {
  return Iterator{entity_map_.end(), registry_};
}

} // namespace vulture 