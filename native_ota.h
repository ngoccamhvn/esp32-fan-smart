#pragma once
#include "esphome.h"
#include <WiFi.h>
#include <HTTPClient.h>

// Gọi trực tiếp thư viện ghi phân vùng của ESP-IDF Core có sẵn, bỏ qua Update.h để tránh lỗi MD5
#include "esp_ota_ops.h"
#include "esp_partition.h"

namespace esphome {

class NativeOTA {
private:
    static void ota_task(void *pvParameters) {
        std::string *url = (std::string *)pvParameters;
        
        HTTPClient http;
        http.begin(url->c_str());
        http.setTimeout(15000);
        
        int httpCode = http.GET();
        if (httpCode == 200) {
            int contentLength = http.getSize();
            WiFiClient *stream = http.getStreamPtr();
            
            const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
            if (update_partition != NULL) {
                esp_ota_handle_t update_handle = 0;
                esp_err_t err = esp_ota_begin(update_partition, contentLength, &update_handle);
                
                if (err == ESP_OK) {
                    ESP_LOGI("Native_OTA", "Dang ghi file tu GitHub vao Flash (%d bytes)...", contentLength);
                    uint8_t *ota_write_data = (uint8_t *)malloc(1024);
                    int binary_size = 0;
                    
                    while (stream->connected() && binary_size < contentLength) {
                        size_t available = stream->available();
                        if (available > 0) {
                            int read_len = stream->read(ota_write_data, available > 1024 ? 1024 : available);
                            esp_ota_write(update_handle, (const void *)ota_write_data, read_len);
                            binary_size += read_len;
                        }
                        delay(1);
                    }
                    free(ota_write_data);
                    
                    if (esp_ota_end(update_handle) == ESP_OK && esp_ota_set_boot_partition(update_partition) == ESP_OK) {
                        ESP_LOGI("Native_OTA", "Cap nhat hoan tat! Chip tu khoi dong lai...");
                        delay(1000);
                        esp_restart();
                    } else {
                        ESP_LOGE("Native_OTA", "Loi ket thuc ghi hoac phan vung khoi dong.");
                    }
                } else {
                    ESP_LOGE("Native_OTA", "Khong the khoi tao phan vung ghi Flash.");
                }
            }
        } else {
            ESP_LOGE("Native_OTA", "Loi HTTP khi tai firmware tu GitHub: %d", httpCode);
        }
        http.end();
        delete url;
        vTaskDelete(NULL);
    }

public:
    static void start_update(const std::string &url) {
        std::string *url_alloc = new std::string(url);
        // Khởi tạo luồng ngầm độc lập FreeRTOS cách ly hoàn toàn với luồng chạy quạt chính
        xTaskCreate(&NativeOTA::ota_task, "native_ota_task", 8192, url_alloc, 5, NULL);
    }
};

}
