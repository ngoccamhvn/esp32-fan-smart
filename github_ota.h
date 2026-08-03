#pragma once
#include "esphome.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

namespace esphome {

class GitHubOTA {
public:
    // Hàm kiểm tra phiên bản trực tiếp bằng ESP-IDF Client
    static std::string check_version(const std::string &user, const std::string &repo) {
        std::string url = "https://github.com" + user + "/" + repo + "/releases/latest";
        std::string result = "";
        
        esp_http_client_config_t config = {};
        config.url = url.c_str();
        config.skip_cert_common_name_check = true;
        config.method = HTTP_METHOD_GET;
        
        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_http_client_set_header(client, "Accept", "application/vnd.github.v3+json");
        esp_http_client_set_header(client, "User-Agent", "esphome-device");
        
        if (esp_http_client_open(client, 0) == ESP_OK) {
            esp_http_client_fetch_headers(client);
            char buffer[1024];
            int read_len = esp_http_client_read(client, buffer, sizeof(buffer) - 1);
            if (read_len > 0) {
                buffer[read_len] = '\0';
                result = std::string(buffer);
            }
        }
        esp_http_client_cleanup(client);
        return result;
    }

    // Hàm nạp firmware trực tiếp bằng ESP-IDF OTA
    static void start_update(const std::string &url) {
        esp_http_client_config_t http_config = {};
        http_config.url = url.c_str();
        http_config.skip_cert_common_name_check = true;
        http_config.keep_alive_enable = true;

        esp_https_ota_config_t ota_config = {};
        ota_config.http_config = &http_config;

        ESP_LOGI("GitHub_OTA", "Đang tiến hành nạp firmware mới...");
        esp_err_t ret = esp_https_ota(&ota_config);
        if (ret == ESP_OK) {
            ESP_LOGI("GitHub_OTA", "Cập nhật thành công! Đang khởi động lại...");
            delay(1000);
            esp_restart();
        } else {
            ESP_LOGE("GitHub_OTA", "Lỗi nạp firmware (Mã lỗi: %d)", ret);
        }
    }
};

}
