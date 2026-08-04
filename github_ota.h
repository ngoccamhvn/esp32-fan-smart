#pragma once
#include "esphome.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

namespace esphome {

class GitHubOTA {
private:
    static void ota_task(void *pvParameters) {
        std::string *url = (std::string *)pvParameters;
        
        esp_http_client_config_t http_config = {};
        http_config.url = url->c_str();
        http_config.skip_cert_common_name_check = true;
        http_config.keep_alive_enable = true;
        http_config.buffer_size = 1024 * 4; // Tăng bộ đệm mạng

        esp_https_ota_config_t ota_config = {};
        ota_config.http_config = &http_config;

        esp_err_t ret = esp_https_ota(&ota_config);
        if (ret == ESP_OK) {
            esp_restart();
        }
        delete url;
        vTaskDelete(NULL);
    }

public:
    static std::string check_version(const std::string &user, const std::string &repo) {
        std::string url = "https://github.com" + user + "/" + repo + "/releases/latest";
        char rx_buffer[1024] = {0};
        
        esp_http_client_config_t config = {};
        config.url = url.c_str();
        config.skip_cert_common_name_check = true;
        config.method = HTTP_METHOD_GET;
        config.user_agent = "esphome-device";
        
        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_http_client_set_header(client, "Accept", "application/vnd.github.v3+json");
        
        std::string result = "";
        if (esp_http_client_open(client, 0) == ESP_OK) {
            esp_http_client_fetch_headers(client);
            int read_len = esp_http_client_read(client, rx_buffer, sizeof(rx_buffer) - 1);
            if (read_len > 0) {
                rx_buffer[read_len] = '\0';
                result = std::string(rx_buffer);
            }
        }
        esp_http_client_cleanup(client);
        return result;
    }

    static void start_update(const std::string &url) {
        std::string *url_alloc = new std::string(url);
        // Tạo task chạy ngầm với bộ nhớ 8KB để không làm treo chip khi tải file
        xTaskCreate(&GitHubOTA::ota_task, "ota_task", 8192, url_alloc, 5, NULL);
    }
};

}
