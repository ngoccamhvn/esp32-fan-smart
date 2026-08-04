#pragma once
#include "esphome.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

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

    // Luồng ngầm 1: Kiểm tra phiên bản độc lập
    static void check_version_task(void *pvParameters) {
        CheckParam *param = (CheckParam *)pvParameters;
        std::string url = "https://github.com" + param->user + "/" + param->repo + "/releases/latest";
        
        esp_http_client_config_t config = {};
        config.url = url.c_str();
        config.skip_cert_common_name_check = true;
        config.method = HTTP_METHOD_GET;
        config.user_agent = "esphome-device";
        config.timeout_ms = 5000;
        
        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_http_client_set_header(client, "Accept", "application/vnd.github.v3+json");
        
        if (esp_http_client_open(client, 0) == ESP_OK) {
            esp_http_client_fetch_headers(client);
            char *buffer = (char *)malloc(1024);
            if (buffer != nullptr) {
                int read_len = esp_http_client_read(client, buffer, 1023);
                if (read_len > 0) {
                    buffer[read_len] = '\0';
                    std::string body(buffer);
                    
                    size_t pos = body.find("\"tag_name\":\"");
                    if (pos != std::string::npos) {
                        pos += 12; 
                        size_t end_pos = body.find("\"", pos);
                        std::string new_ver = body.substr(pos, end_pos - pos);
                        
                        if (new_ver.rfind("v", 0) == 0) {
                            new_ver = new_ver.substr(1);
                        }
                        
                        // Đẩy trạng thái về giao diện qua con trỏ được truyền vào
                        param->sensor_id->publish_state(new_ver);
                        
                        if (new_ver != param->current_ver) {
                            param->global_id->value() = true;
                            ESP_LOGI("GitHub_OTA", "Phat hien ban moi: %s. San sang cap nhat.", new_ver.c_str());
                        } else {
                            param->global_id->value() = false;
                            ESP_LOGI("GitHub_OTA", "Phan mem hien tai da la moi nhat.");
                        }
                    }
                }
                free(buffer);
            }
        } else {
            ESP_LOGE("GitHub_OTA", "Loi: Khong the ket noi toi GitHub API");
        }
        esp_http_client_cleanup(client);
        delete param;
        vTaskDelete(NULL);
    }

    // Luồng ngầm 2: Tải file dung lượng lớn và tự Flash OTA
    static void ota_task(void *pvParameters) {
        std::string *url = (std::string *)pvParameters;
        
        esp_http_client_config_t http_config = {};
        http_config.url = url->c_str();
        http_config.skip_cert_common_name_check = true;
        http_config.keep_alive_enable = true;

        esp_https_ota_config_t ota_config = {};
        ota_config.http_config = &http_config;

        esp_err_t ret = esp_https_ota(&ota_config);
        if (ret == ESP_OK) {
            ESP_LOGI("GitHub_OTA", "Cap nhat thanh cong! Dang khoi dong lai chip...");
            delay(1000);
            esp_restart();
        } else {
            ESP_LOGE("GitHub_OTA", "Loi nạp firmware tu xa that bai: %d", ret);
        }
        delete url;
        vTaskDelete(NULL);
    }

public:
    // Nhận trực tiếp con trỏ đối tượng từ file .yaml truyền sang
    static void start_check(const std::string &user, const std::string &repo, const std::string &current_ver, text_sensor::TextSensor *sensor_id, globals::GlobalsComponent<bool> *global_id) {
        CheckParam *param = new CheckParam{user, repo, current_ver, sensor_id, global_id};
        xTaskCreate(&GitHubOTA::check_version_task, "check_ver_task", 6144, param, 5, NULL);
    }

    static void start_update(const std::string &url) {
        std::string *url_alloc = new std::string(url);
        xTaskCreate(&GitHubOTA::ota_task, "ota_task", 8192, url_alloc, 5, NULL);
    }
};

}
