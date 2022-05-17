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

#include <cassert>
#include <cstdlib>
#include <functional>
#include <unordered_map>
#include <vector>

namespace vulture {

using EventId = size_t;

template <typename T>
struct EventIdGenerator {
  static EventId GetId() {
    static EventId id{0};
    return id++;
  }
};

template <typename T>
struct EventIdHolder {
  static EventId GetId() {
    const static EventId id = EventIdGenerator<T>::GetId();
    return id;
  }
};

class BaseSink {
 public:
  virtual ~BaseSink(){};
};

template <typename T>
class Sink final : public BaseSink {
 public:
  template <auto F, typename U>
  void Connect(U& instance) {
    callback_.emplace_back(std::bind(F, instance, std::placeholders::_1));
  }
  template <auto F>
  void Connect() {
    callback_.emplace_back(std::bind(F, std::placeholders::_1));
  }

  template <auto F, typename U>
  void Connect() {
    callback_.emplace_back(std::bind(F, std::placeholders::_1));
  }

  void Publish(T& event) {
    for (size_t i = 0; i < callback_.size(); ++i) {
      callback_[i](event);
    }
  }

 private:
  std::vector<std::function<void(T&)>> callback_;
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

  template <typename EventType>
  void Trigger(EventType& event) {
    Assure<EventType>(EventIdHolder<EventType>::GetId()).Publish(event);
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