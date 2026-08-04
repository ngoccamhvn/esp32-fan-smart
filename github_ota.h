#pragma once
#include "esphome.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

namespace esphome {

class GitHubOTA {
private:
    struct CheckParam {
        std::string user;
        std::string repo;
        std::string current_ver;
        text_sensor::TextSensor *sensor_id;
        globals::GlobalsComponent<bool> *global_id;
    };

    static std::string &get_latest_version_str() {
        static std::string latest_ver = "";
        return latest_ver;
    }

    // Luồng ngầm 1: Kiểm tra phiên bản từ GitHub API bằng Arduino HTTP
    static void check_version_task(void *pvParameters) {
        CheckParam *param = (CheckParam *)pvParameters;
        std::string url = "https://github.com" + param->user + "/" + param->repo + "/releases/latest";
        
        HTTPClient http;
        http.begin(url.c_str());
        http.setUserAgent("esphome-device");
        http.addHeader("Accept", "application/vnd.github.v3+json");
        http.setTimeout(5000);
        
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            std::string body = http.getString().c_str();
            size_t pos = body.find("\"tag_name\":\"");
            if (pos != std::string::npos) {
                pos += 12; 
                size_t end_pos = body.find("\"", pos);
                std::string new_ver = body.substr(pos, end_pos - pos);
                
                if (new_ver.rfind("v", 0) == 0) {
                    new_ver = new_ver.substr(1);
                }
                
                get_latest_version_str() = new_ver;
                param->sensor_id->publish_state(new_ver);
                
                if (new_ver != param->current_ver) {
                    param->global_id->value() = true;
                    ESP_LOGI("GitHub_OTA", "Phat hien ban moi: %s", new_ver.c_str());
                } else {
                    param->global_id->value() = false;
                    ESP_LOGI("GitHub_OTA", "Firmware da la moi nhat.");
                }
            }
        } else {
            ESP_LOGE("GitHub_OTA", "Loi ket noi GitHub API, ma loi HTTP: %d", httpCode);
        }
        http.end();
        delete param;
        vTaskDelete(NULL);
    }

    // Luồng ngầm 2: Tải và nạp file .bin từ xa an toàn bằng Arduino Update Core
    static void ota_task(void *pvParameters) {
        std::string *url = (std::string *)pvParameters;
        
        HTTPClient http;
        http.begin(url->c_str());
        http.setTimeout(15000); // Tăng thời gian chờ tải file nặng
        
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            int contentLength = http.getSize();
            bool canUpdate = Update.begin(contentLength);
            
            if (canUpdate) {
                ESP_LOGI("GitHub_OTA", "Dang tai firmware va ghi vao Flash (%d bytes)...", contentLength);
                WiFiClient *stream = http.getStreamPtr();
                size_t written = Update.writeStream(*stream);
                
                if (written == contentLength) {
                    ESP_LOGI("GitHub_OTA", "Ghi file thanh cong %d bytes.", written);
                }
                
                if (Update.end()) {
                    if (Update.isFinished()) {
                        ESP_LOGI("GitHub_OTA", "Cap nhat hoan tat! Chip tu khoi dong lai...");
                        delay(1000);
                        ESP.restart();
                    }
                } else {
                    ESP_LOGE("GitHub_OTA", "Loi ket thuc Update (Ma loi: %d)", Update.getError());
                }
            } else {
                ESP_LOGE("GitHub_OTA", "Khong du bo nho trong de thuc hien ghi OTA.");
            }
        } else {
            ESP_LOGE("GitHub_OTA", "Loi HTTP khi tai firmware: %d", httpCode);
        }
        http.end();
        delete url;
        vTaskDelete(NULL);
    }

public:
    static void start_check(const std::string &user, const std::string &repo, const std::string &current_ver, text_sensor::TextSensor *sensor_id, globals::GlobalsComponent<bool> *global_id) {
        CheckParam *param = new CheckParam{user, repo, current_ver, sensor_id, global_id};
        xTaskCreate(&GitHubOTA::check_version_task, "check_ver_task", 6144, param, 5, NULL);
    }

    static void start_update(const std::string &url) {
        std::string *url_alloc = new std::string(url);
        xTaskCreate(&GitHubOTA::ota_task, "ota_task", 8192, url_alloc, 5, NULL);
    }

    static std::string get_new_version() {
        return get_latest_version_str();
    }
};

}
