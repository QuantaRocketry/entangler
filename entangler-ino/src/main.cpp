#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <Config.h>
#include <Event.h>
#include <FreeRTOS.h>
#include <LittleFS.h>
#include <LoRa.h>
#include <SPI.h>

#include <Debug.hpp>

#define RFM95_CS 16
#define RFM95_INT 21
#define RFM95_RST 17

#define RF95_FREQ 915E6

void testTask(void *pvParams) {
  QueueHandle_t q = xQueueCreate(10, sizeof(uint8_t));
  DBG(GLOBAL::EventHandler.subscribe("test", q) ? "true" : "false");
  for (;;) {
    uint8_t buf = 0;
    xQueueReceive(q, &buf, portMAX_DELAY);
  }
}

void state_visualiser(void *pvParams) {
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, HIGH);

  Adafruit_NeoPixel pixel(1, NEOPIXEL, NEO_GRB + NEO_KHZ800);
  pixel.begin();

  for (uint16_t i = 0; true; i += 0xFFFF / 500) {
    pixel.fill(pixel.ColorHSV(i, 255, 100));
    pixel.show();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  INITIALISE_DBG(115200);

  delay(2000);

  // while (!Serial) delay(100);

  DBG(SCK);
  DBG(MOSI);
  DBG(MISO);
  DBG(NEOPIXEL);

  DBG("Create topic");
  GLOBAL::EventHandler.create_topic("test", sizeof(uint8_t));

  xTaskCreate(testTask, "test1", 1000, NULL, 0, NULL);
  xTaskCreate(testTask, "test2", 1000, NULL, 0, NULL);
  xTaskCreate(state_visualiser, "neopix", 1000, NULL, 0, NULL);

  if (!GLOBAL::ConfigManager.begin()) {
    DBG("failed to start config manager");
  };

  // LoRa.setPins(RFM95_CS, RFM95_RST, RFM95_INT);
  // if (!LoRa.begin(915E6)) {
  //   Serial.println("LoRa radio init failed");
  //   while (1);
  // }
}

void loop() {
  // delay(1000);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
  // delay(1000);
  // DBG("Sending test...");
  // delay(100);
  uint8_t i = 1;
  GLOBAL::EventHandler.publish("test", &i);

  config_t cfg;
  GLOBAL::ConfigManager.getConfig(&cfg);
  // cfg.bandwidth += 1;
  // GLOBAL::ConfigManager.setConfig(cfg);
  // DBG(cfg.bandwidth);

  // vTaskDelay(1000 / portTICK_PERIOD_MS);
  // //   LoRa.beginPacket();
  // //   LoRa.print("test");
  // //   LoRa.endPacket();
}
 