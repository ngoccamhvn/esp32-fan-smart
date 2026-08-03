#pragma once
#include "esphome.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

namespace esphome {

class GitHubOTA {
public:
    static void start_update(const std::string &url) {
        // 1. Cấu hình kết nối HTTP Client gốc
        esp_http_client_config_t http_config = {};
        http_config.url = url.c_str();
        http_config.skip_cert_common_name_check = true;
        http_config.keep_alive_enable = true;

        // 2. Bọc cấu hình mạng vào cấu trúc ota_config chuẩn của ESP-IDF v5.x
        esp_https_ota_config_t ota_config = {};
        ota_config.http_config = &http_config;

        ESP_LOGI("GitHub_OTA", "Đang tải và nạp trực tiếp firmware từ: %s", url.c_str());
        
        // 3. Thực thi nạp dữ liệu từ luồng HTTPS
        esp_err_t ret = esp_https_ota(&ota_config);
        if (ret == ESP_OK) {
            ESP_LOGI("GitHub_OTA", "Nạp Firmware thành công! Đang khởi động lại thiết bị...");
            delay(1000);
            esp_restart();
        } else {
            ESP_LOGE("GitHub_OTA", "Lỗi nạp firmware thất bại (Mã lỗi ESP-IDF: %d)", ret);
        }
    }
};

}
