#ifndef WIFI_HPP
#define WIFI_HPP

#include <cstdint>
namespace vfarm {

class WifiCust {
public:
    const char* tag = "wifi_c";
    int64_t timeout = 30000;
    int64_t start_wait;
    
    int64_t pulse_start;
    int64_t pulse_end;
    float distance_cm;

    WifiCust();
    WifiCust(const char *wifi_ssid, const char *wifi_pass);
    WifiCust(const char *wifi_ssid, const char *wifi_pass, const char *server_url);



    void init_wifi();
    void set_wifi_creds(const char* wifi_ssid, const char *wifi_pass);
    void set_server_url(const char *server_url);
    void wifi_init_sta();
    bool server_is_up();
    bool check_status();


 private:

    const char *wifi_ssid_;
    const char *wifi_pass_;
    const char *server_url_;

};
}

#endif