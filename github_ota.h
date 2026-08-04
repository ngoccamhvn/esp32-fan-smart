#pragma once
#include "esphome.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

namespace esphome {

// Khai báo trước các biến toàn cục từ ESPHome sang C++ để đồng bộ dữ liệu ngầm
extern text_sensor::TemplateTextSensor *latest_version;
extern globals::GlobalsComponent<bool> *has_new_fw;

class GitHubOTA {
private:
    struct CheckParam {
        std::string user;
        std::string repo;
        std::string current_ver;
    };

    // Luồng ngầm 1: Tự động kết nối GitHub API lấy phiên bản mà không gây treo quạt
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
            // Cấp phát bộ đệm lớn để hứng trọn gói tin phản hồi của GitHub JSON
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
                        
                        // Đẩy dữ liệu an toàn về lại giao diện ESPHome thông qua hàng đợi luồng
                        esphome::App.feed_wdt();
                        latest_version->publish_state(new_ver);
                        
                        if (new_ver != param->current_ver) {
                            has_new_fw->value() = true;
                            ESP_LOGI("GitHub_OTA", "Phát hiện bản mới: %s. Đã mở khóa nút Cập nhật.", new_ver.c_str());
                        } else {
                            has_new_fw->value() = false;
                            ESP_LOGI("GitHub_OTA", "Mã nguồn hiện tại đã là mới nhất.");
                        }
                    }
                }
                free(buffer);
            }
        } else {
            ESP_LOGE("GitHub_OTA", "Lỗi: Không thể kết nối tới GitHub API (Kiểm tra Internet của ESP32)");
        }
        esp_http_client_cleanup(client);
        delete param;
        vTaskDelete(NULL);
    }

    // Luồng ngầm 2: Tự động kéo file .bin nặng vài MB từ GitHub về nạp đè vào Flash
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
            ESP_LOGI("GitHub_OTA", "Nạp phần mềm thành công! Chip tự khởi động lại...");
            delay(1000);
            esp_restart();
        } else {
            ESP_LOGE("GitHub_OTA", "Lỗi quy trình nạp OTA từ xa thất bại: %d", ret);
        }
        delete url;
        vTaskDelete(NULL);
    }

public:
    static void start_check(const std::string &user, const std::string &repo, const std::string &current_ver) {
        CheckParam *param = new CheckParam{user, repo, current_ver};
        xTaskCreate(&GitHubOTA::check_version_task, "check_ver_task", 6144, param, 5, NULL);
    }

    static void start_update(const std::string &url) {
        std::string *url_alloc = new std::string(url);
        xTaskCreate(&GitHubOTA::ota_task, "ota_task", 8192, url_alloc, 5, NULL);
    }
};

}
