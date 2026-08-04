#pragma once
#include "esphome.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_ota_ops.h"
#include "esp_partition.h"

namespace esphome {

class NativeOTA {
private:
    struct CheckParam {
        text_sensor::TextSensor *sensor_id;
        globals::GlobalsComponent<bool> *global_id;
        std::string current_ver;
    };

    // Luồng ngầm 1: Gọi GitHub API lấy phiên bản (Giấu URL vào C++)
    static void check_version_task(void *pvParameters) {
        CheckParam *param = (CheckParam *)pvParameters;
        
        // Khóa chuỗi tại C++ để bảo vệ Bootloader file .bin
        const char* api_url = "https://github.com";
        
        HTTPClient http;
        http.begin(api_url);
        http.setUserAgent("esphome-device");
        http.addHeader("Accept", "application/vnd.github.v3+json");
        http.setTimeout(5000);
        
        int httpCode = http.GET();
        if (httpCode == 200) {
            std::string body = http.getString().c_str();
            size_t pos = body.find("\"tag_name\":\"");
            if (pos != std::string::npos) {
                pos += 12; 
                size_t end_pos = body.find("\"", pos);
                std::string new_ver = body.substr(pos, end_pos - pos);
                
                if (new_ver.rfind("v", 0) == 0) {
                    new_ver = new_ver.substr(1);
                }
                
                param->sensor_id->publish_state(new_ver);
                
                if (new_ver != param->current_ver) {
                    param->global_id->value() = true;
                    ESP_LOGI("Native_OTA", "Phat hien ban moi: %s. San sang cap nhat.", new_ver.c_str());
                } else {
                    param->global_id->value() = false;
                    ESP_LOGI("Native_OTA", "Firmware hien tai da la moi nhat.");
                }
            }
        } else {
            ESP_LOGE("Native_OTA", "Loi HTTP khi check API: %d", httpCode);
        }
        http.end();
        delete param;
        vTaskDelete(NULL);
    }

    // Luồng ngầm 2: Tải và tự Flash OTA file từ GitHub (Giấu URL vào C++)
    static void ota_task(void *pvParameters) {
        // Khóa chuỗi tải file trực tiếp tại phân vùng dữ liệu C++
        const char* ota_url = "https://github.com";
        
        HTTPClient http;
        http.begin(ota_url);
        http.setTimeout(15000);
        
        int httpCode = http.GET();
        if (httpCode == 200) {
            int contentLength = http.getSize();
            WiFiClient *stream = http.getStreamPtr();
            const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
            
            if (update_partition != NULL) {
                esp_ota_handle_t update_handle = 0;
                if (esp_ota_begin(update_partition, contentLength, &update_handle) == ESP_OK) {
                    ESP_LOGI("Native_OTA", "Dang ghi vao Flash: %d bytes", contentLength);
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
                        ESP_LOGI("Native_OTA", "Thanh cong! Khoi dong lai chip...");
                        delay(1000);
                        esp_restart();
                    }
                }
            }
        } else {
            ESP_LOGE("Native_OTA", "Loi HTTP khi tai file: %d", httpCode);
        }
        http.end();
        vTaskDelete(NULL);
    }

public:
    static void start_check(text_sensor::TextSensor *sensor_id, globals::GlobalsComponent<bool> *global_id, const std::string &current_ver) {
        CheckParam *param = new CheckParam{sensor_id, global_id, current_ver};
        xTaskCreate(&NativeOTA::check_version_task, "native_check_task", 6144, param, 5, NULL);
    }

    static void start_update() {
        xTaskCreate(&NativeOTA::ota_task, "native_ota_task", 8192, NULL, 5, NULL);
    }
};

}
