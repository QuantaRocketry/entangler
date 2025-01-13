#pragma once

#include <Arduino.h>
#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>

#include <Debug.hpp>
#include <map>
#include <string>
#include <unordered_map>

class Event {
 private:
  SemaphoreHandle_t mtx;
  std::unordered_map<std::string, std::vector<QueueHandle_t>> topics;

 public:
  Event();
  bool create_topic(std::string topic, UBaseType_t uxItemSize);
  void publish(std::string topic, const void* pvItemToQueue);
  bool subscribe(std::string topic, QueueHandle_t queueHandle);
};

Event::Event() {
  this->mtx = xSemaphoreCreateMutex();
  this->topics = std::unordered_map<std::string, std::vector<QueueHandle_t>>();
}

/// @brief Creates a topic with no subscribers.
/// @param topic The topic name.
/// @param uxItemSize The size of the items that will be published.
/// @return
bool Event::create_topic(std::string topic, UBaseType_t uxItemSize) {
  xSemaphoreTake(this->mtx, portMAX_DELAY);

  if (this->topics.find(topic) != this->topics.end()) {
    DBG("Tried to create already existing topic: " + String(topic.c_str()));
    xSemaphoreGive(this->mtx);
    return false;
  }

  this->topics[topic] = std::vector<QueueHandle_t>();
  xSemaphoreGive(this->mtx);

  return true;
}

/// @brief Publishes item to all subscriber queues that have been registered.
/// Skips queue if full.
/// @param topic The topic to publish to.
/// @param pvItemToQueue A pointer to the object to write.
void Event::publish(std::string topic, const void* pvItemToQueue) {
  xSemaphoreTake(this->mtx, portMAX_DELAY);

  auto v = this->topics[topic];
  for (auto q : v) {
    xQueueSendToBack(q, pvItemToQueue, 0);
  }

  xSemaphoreGive(this->mtx);
};

/// @brief Adds the queue to the topic.
/// Must create the queue before subscribing to the topic.
/// @param topic The topic to subscribe to.
/// @param queue The queue to add to the topic.
/// @return True if the topic exists yet.
bool Event::subscribe(std::string topic, QueueHandle_t queueHandle) {
  xSemaphoreTake(this->mtx, portMAX_DELAY);

  if (this->topics.find(topic) == this->topics.end()) {
    DBG("No topic found: " + String(topic.c_str()));
    xSemaphoreGive(this->mtx);
    return false;
  }

  this->topics[topic].push_back(queueHandle);
  xSemaphoreGive(this->mtx);
  return true;
};

namespace GLOBAL {
Event EventHandler;
}
