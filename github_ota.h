#pragma once
#include "esphome.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

namespace esphome {

class GitHubOTA {
public:
    static void start_update(const std::string &url) {
        esp_http_client_config_t config = {};
        config.url = url.c_str();
        config.skip_cert_common_name_check = true;
        config.keep_alive_enable = true;

        ESP_LOGI("GitHub_OTA", "Đang tải firmware trực tiếp từ ESP-IDF Core: %s", url.c_str());
        
        esp_err_t ret = esp_https_ota(&config);
        if (ret == ESP_OK) {
            ESP_LOGI("GitHub_OTA", "Nạp Firmware thành công! Đang khởi động lại thiết bị...");
            delay(1000);
            esp_restart();
        } else {
            ESP_LOGE("GitHub_OTA", "Lỗi nạp firmware thất bại từ GitHub (Mã lỗi: %d)", ret);
        }
    }
};

}
