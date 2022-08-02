#include "Utils.h"
#include "Webserver.h"
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "Logger.h"
#include "InternalStorage.h"

// TODO: 01-Jun-2022 @basshelal: WebServer needs to behave differently depending on which
//  WifiMode we are in, if in AP mode then show a different page (set up page) from when
//  we are in STA mode (camera view etc)

#define FILE_BUFFER_SIZE 4096
private void *fileBuffer;

private esp_err_t webserver_getHandler(httpd_req_t *request) {
    // TODO: 02-Aug-2022 @basshelal: Make log functions use shorter filename
    INFO("URI: %s", request->uri);
    httpd_resp_set_status(request, HTTPD_200);
    const bool exists = internalStorage_queryFileExists("index.html");

    if (exists) {
        uint sizeBytes;
        internalStorage_queryFileSize("index.html", &sizeBytes);

        FILE *file = malloc(sizeof(FILE));
        internalStorage_openFile("index.html", &file);
        uint bytesRemaining = sizeBytes;
        uint position =0;
        while (bytesRemaining > 0) {
            uint bytesRead;
            const uint bytesToRead = (bytesRemaining < FILE_BUFFER_SIZE) ? bytesRemaining : FILE_BUFFER_SIZE;
            internalStorage_readFileChunks(file, position,
                                           fileBuffer, bytesToRead, &bytesRead);
            httpd_resp_send_chunk(request, fileBuffer, (ssize_t) bytesToRead);
            bytesRemaining -= bytesRead;
            position += bytesRead;
        }
        httpd_resp_send_chunk(request, NULL, 0);
        internalStorage_closeFile(file);

    } else {
        char buffer[512];
        sprintf(buffer, "%u at /* exists: %i", esp_log_timestamp(), exists);
        httpd_resp_sendstr_chunk(request, buffer);
        httpd_resp_send_chunk(request, NULL, 0);
    }
    return ESP_OK;
}

private esp_err_t webserver_getHandlerTime(httpd_req_t *request) {
    INFO("URI: %s", request->uri);
    char buffer[512];
    sprintf(buffer, "%u at /time", esp_log_timestamp());
    httpd_resp_set_status(request, HTTPD_200);
    httpd_resp_sendstr_chunk(request, buffer);
    httpd_resp_send_chunk(request, NULL, 0);
    return ESP_OK;
}

public WebserverError webserver_init() {
    esp_err_t err;
    httpd_handle_t server1 = NULL;
    httpd_config_t config1 = HTTPD_DEFAULT_CONFIG();

    config1.max_uri_handlers = 128;

    /* Use the URI wildcard matching function in order to
     * allow the same handler to respond to multiple different
     * target URIs which match the wildcard scheme */
    config1.uri_match_fn = httpd_uri_match_wildcard;

    INFO("Starting Web Server on port: '%d'", config1.server_port);
    err = httpd_start(&server1, &config1);
    if (err != ESP_OK) {
        ERROR("Failed to start Web Server! %i %s", err, esp_err_to_name(err));
        return ESP_FAIL;
    }

    httpd_uri_t time = {
            .uri       = "/time",
            .method    = HTTP_GET,
            .handler   = webserver_getHandlerTime,
            .user_ctx  = NULL
    };
    httpd_register_uri_handler(server1, &time);

    httpd_uri_t file_download = {
            .uri       = "/*",  // Match all URIs of type /path/to/file
            .method    = HTTP_GET,
            .handler   = webserver_getHandler,
            .user_ctx  = NULL    // Pass server data as context
    };
    httpd_register_uri_handler(server1, &file_download);

    INFO("Web Server started!");

    internalStorage_init();

    fileBuffer = malloc(FILE_BUFFER_SIZE);

    return ESP_OK;
}
