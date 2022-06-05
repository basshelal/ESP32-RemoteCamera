#include "WebServer.h"
#include "utils.h"

#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"

static const char *TAG = "WebServer";

// TODO: 01-Jun-2022 @basshelal: WebServer needs to be able to load HTML pages that we place in
//  the FileSystem/DataStore because they might be very large and complex (containing CSS and JS)

// TODO: 01-Jun-2022 @basshelal: WebServer needs to behave differently depending on which
//  WifiMode we are in, if in AP mode then show a different page (set up page) from when
//  we are in STA mode (camera view etc)

// TODO: 05-Jun-2022 @basshelal: To make bi-directional communication easy, we need to use Web-Sockets
//  this may or not be placed here in this service

private esp_err_t webServer_getHandler(httpd_req_t *request) {
    ESP_LOGI(TAG, "Request to URI: %s", request->uri);
    httpd_resp_set_status(request, HTTPD_200);
    httpd_resp_sendstr_chunk(request, "Hello World!");
    httpd_resp_send_chunk(request, NULL, 0);
    ESP_LOGI(TAG, "Responding...");
    return ESP_OK;
}

public esp_err_t webServer_init() {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Use the URI wildcard matching function in order to
     * allow the same handler to respond to multiple different
     * target URIs which match the wildcard scheme */
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "Starting Web Server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start Web Server!");
        return ESP_FAIL;
    }

    httpd_uri_t file_download = {
            .uri       = "/*",  // Match all URIs of type /path/to/file
            .method    = HTTP_GET,
            .handler   = webServer_getHandler,
            .user_ctx  = NULL    // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_download);

    ESP_LOGI(TAG, "Web Server started!");

    return ESP_OK;
}

public typeof(WebServer) WebServer = {
        .init = webServer_init,
};