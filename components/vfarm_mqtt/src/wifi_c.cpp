#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_log.h"
#include "wifi_c.hpp"


namespace vfarm {

/* static members */

WifiCust::WifiCust(const char *wifi_ssid, const char *wifi_pass):
    wifi_ssid_(wifi_ssid), wifi_pass_(wifi_pass)
{
    this->init_wifi();
}

WifiCust::WifiCust(const char *wifi_ssid, const char *wifi_pass, const char *server_url):
    wifi_ssid_(wifi_ssid), wifi_pass_(wifi_pass), server_url_(server_url)
{
    this->init_wifi();
}

void WifiCust::init_wifi(){
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();
}

void WifiCust::set_wifi_creds(const char* wifi_ssid, const char *wifi_pass){
    
    this->wifi_ssid_ = wifi_ssid;
    this->wifi_pass_ = wifi_pass;
}

void WifiCust::set_server_url(const char *server_url){
    this->server_url_ = server_url;
}

void WifiCust::wifi_init_sta(){
    {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {};
    strncpy((char *)wifi_config.sta.ssid, wifi_ssid_, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, wifi_pass_, sizeof(wifi_config.sta.password));

    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(tag, "Connecting to Wi-Fi...");
    ESP_ERROR_CHECK(esp_wifi_connect());
    }
}

bool WifiCust::server_is_up() {
    if (!server_url_) return false;

    esp_http_client_config_t config = {};
    config.url = server_url_;
    config.method = HTTP_METHOD_GET;
    config.timeout_ms = 1000;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) return false;

    esp_err_t err = esp_http_client_perform(client);
    int status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);

    return (err == ESP_OK && status == 200);
}

bool WifiCust::check_status(){
    wifi_ap_record_t info;
    if (esp_wifi_sta_get_ap_info(&info) == ESP_OK) {
        ESP_LOGI(tag, "Connected to SSID: %s, RSSI: %d", info.ssid, info.rssi);
        return 1;}

    return 0 ;
}


}