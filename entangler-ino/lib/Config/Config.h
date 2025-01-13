#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FreeRTOS.h>
#include <LittleFS.h>
#include <VFS.h>
#include <semphr.h>

#include <Debug.hpp>

struct config_t {
  uint32_t bandwidth = 125e3;
  uint32_t frequency = 915e6;

  /// @brief Serializes the config_t into a string
  /// @return The serialized string
  String serialize() {
    JsonDocument doc;
    doc["bandwidth"] = this->bandwidth;
    doc["frequency"] = this->frequency;

    String json;
    serializeJson(doc, json);
    return json;
  }

  /// @brief Deserializes a json string into this config_t
  /// @param json The json string to deserialize from
  void deserialize(String json) {
    JsonDocument doc;
    deserializeJson(doc, json);
    this->bandwidth = doc["bandwidth"];
    this->frequency = doc["frequency"];
  }
};

class Config {
 private:
  bool initialised = false;
  SemaphoreHandle_t mtx;
  config_t cfg;

 public:
  Config();
  ~Config();

  /// @brief Initializes the config manager.
  /// Pulls the in-storage config and stores it in-memory
  /// @return If the intilization was successful
  bool begin();

  /// @brief Populates the config var with the in-memory config_t
  /// @param config The variable that will be populated
  /// @return Whether updating config was successful or not
  bool getConfig(config_t* cfg);

  /// @brief Sets the config in-storage and updates the in-memory config_t
  /// @param config The config to update to
  /// @return Whether the config was updated in-storage
  bool setConfig(config_t cfg);
};

Config::Config() {};
Config::~Config() {};

bool Config::begin() {
  this->mtx = xSemaphoreCreateMutex();
  xSemaphoreTake(this->mtx, portMAX_DELAY);

  if (!LittleFS.begin()) {
    DBG("Failed to mount lfs, try reformatting.");
    xSemaphoreGive(this->mtx);
    return false;
  };

  if (!LittleFS.exists("config.json")) {
    String s = this->cfg.serialize();
    File f = LittleFS.open("config.json", "a");
    f.print(s);
  } else {
    File f = LittleFS.open("config.json", "r");
    this->cfg.deserialize(f.readString());
  }

  this->initialised = true;
  xSemaphoreGive(this->mtx);
  return true;
};

bool Config::getConfig(config_t* config) {
  xSemaphoreTake(this->mtx, portMAX_DELAY);

  if (!this->initialised) {
    xSemaphoreGive(this->mtx);
    return false;
  };

  *config = this->cfg;

  xSemaphoreGive(this->mtx);
  return true;
};

bool Config::setConfig(config_t config) {
  xSemaphoreTake(this->mtx, portMAX_DELAY);

  if (!LittleFS.exists("config.json")) {
    xSemaphoreGive(this->mtx);
    return false;
  };

  this->cfg = config;

  File f = LittleFS.open("config.json", "w");
  f.print(this->cfg.serialize());

  xSemaphoreGive(this->mtx);
  return true;
};

namespace GLOBAL {
Config ConfigManager;
};