/**
 * @file event_system.hpp
 * @author Vasiliy Solostovskiy (vasezs@inbox.ru)
 * @brief
 * @version 0.1
 * @date 2022-05-16
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <cassert>
#include <cstdlib>
#include <functional>
#include <unordered_map>
#include <vector>
#include <tuple>

#include "core/logger.hpp"

namespace vulture {

using EventId = size_t;

struct EventIdGenerator {
  static EventId GetId() {
    static EventId id{0};
    return id++;
  }
};

template <typename T>
struct EventIdHolder {
  static EventId GetId() {
    const static EventId id = EventIdGenerator::GetId();
    return id;
  }
};

class BaseSink {
 public:
  virtual ~BaseSink() {};
};

template <typename EventT>
class Sink final : public BaseSink {
  using function_type = void(const EventT&);
 public:
  template <auto F, typename U>
  void Connect(U& instance) {
    callback_.emplace_back(reinterpret_cast<void*>(F), reinterpret_cast<void*>(&instance), std::bind(F, &instance, std::placeholders::_1));
    // callback_.emplace_back(reinterpret_cast<void*>(F), //FIXME: bind does not work
    //                        std::bind(F, instance, std::placeholders::_1));
  }

  template <auto F>
  void Connect() {
    callback_.emplace_back(reinterpret_cast<void*>(F), nullptr, std::bind(F, std::placeholders::_1));
    // callback_.emplace_back(reinterpret_cast<void*>(F), //FIXME:
    //                        std::bind(F, std::placeholders::_1));
  }

  //FIXME:
  template <auto F, typename U>
  void Disconnect(U& instance) {
    for (auto it = callback_.begin(); it != callback_.end(); ++it) {
      if (std::get<0>(*it) == reinterpret_cast<void*>(F) && std::get<1>(*it) == reinterpret_cast<void*>(&instance)) {
        callback_.erase(it);
        break;
      }
    }
  }

  template <auto F>
  void Disconnect() {
    for (auto it = callback_.begin(); it != callback_.end(); ++it) {
      if (std::get<0>(*it) == reinterpret_cast<void*>(F) && std::get<1>(*it) == nullptr) {
        callback_.erase(it);
        break;
      }
    }
  }

  void Publish(const EventT& event) {
    for (size_t i = 0; i < callback_.size(); ++i) {
      std::get<2>(callback_[i])(event);
    }
  }

 private:
  // std pair just for now, then we will write our own std::function with
  // operator ==
  std::vector<std::tuple<void*, void*, std::function<function_type>>> callback_;
  // std::vector<std::pair<void*, std::function<void(const EventT&)>>> callback_;
};

class Dispatcher {
 public:
  template <typename EventType>
  Sink<EventType>& Assure(EventId id) {
    BaseSink* ans_ptr = nullptr;
    if (dispatchers.count(id) == 0) {
      ans_ptr = new Sink<EventType>{};
      dispatchers[id] = ans_ptr;
    } else {
      ans_ptr = dispatchers[id];
    }

    assert(ans_ptr != nullptr);
    return *static_cast<Sink<EventType>*>(ans_ptr);
  }

  template <typename EventType>
  Sink<EventType>& GetSink() {
    return Assure<EventType>(EventIdHolder<EventType>::GetId());
  }

  template <typename EventType, typename... Args>
  void Trigger(Args&&... args) {
    EventType e{std::forward<Args>(args)...};
    Assure<EventType>(EventIdHolder<EventType>::GetId()).Publish(e);
  }

  ~Dispatcher() {
    for (size_t i = 0; i < dispatchers.size(); ++i) {
      delete dispatchers[i];
    }
  }

 private:
  std::unordered_map<EventId, BaseSink*> dispatchers;
};

}  // namespace vulture
