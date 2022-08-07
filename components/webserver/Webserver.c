#include "Utils.h"
#include "Webserver.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "Logger.h"
#include "InternalStorage.h"
#include "FileMode.h"
#include "cJSON.h"

#define FILE_BUFFER_SIZE 4096
private void *fileBuffer;
private void *favIconFileBuffer;
private LogList *logList;

private esp_err_t webserver_emptyHandler(httpd_req_t *request) {
    INFO("URI: %s", request->uri);
    httpd_resp_send_404(request);
    return ESP_OK;
}

private esp_err_t webserver_pageHandler(httpd_req_t *request) {
    INFO("URI: %s", request->uri);
    httpd_resp_set_status(request, HTTPD_200);
    const bool exists = internalStorage_queryFileExists("index.html");

    esp_err_t espErr;
    if (exists) {
        uint sizeBytes;
        internalStorage_queryFileSize("index.html", &sizeBytes);

        FILE *file;
        internalStorage_openFile("index.html", &file, FILE_MODE_READ);
        uint bytesRemaining = sizeBytes;
        uint position = 0;
        while (bytesRemaining > 0) {
            uint bytesRead;
            const uint bytesToRead = (bytesRemaining < FILE_BUFFER_SIZE) ? bytesRemaining : FILE_BUFFER_SIZE;
            internalStorage_readFileChunks(file, INTERNAL_STORAGE_POSITION_CONTINUE,
                                           fileBuffer, bytesToRead, &bytesRead);
            espErr = httpd_resp_send_chunk(request, fileBuffer, (ssize_t) bytesRead);
            if (espErr != ESP_OK) {
                ERROR("httpd_resp_send_chunk() returned %i : %s", espErr, esp_err_to_name(espErr));
            }
            bytesRemaining -= bytesRead;
            position += bytesRead;
        }
        espErr = httpd_resp_send_chunk(request, NULL, 0);
        if (espErr != ESP_OK) {
            ERROR("httpd_resp_send_chunk() returned %i : %s", espErr, esp_err_to_name(espErr));
        }
        internalStorage_closeFile(file);
    } else {
        char buffer[512];
        sprintf(buffer, "%u at /* exists: %i", esp_log_timestamp(), exists);
        httpd_resp_sendstr_chunk(request, buffer);
        httpd_resp_send_chunk(request, NULL, 0);
    }
    return ESP_OK;
}

private esp_err_t webserver_getFavIcon(httpd_req_t *request) {
    INFO("URI: %s", request->uri);
    httpd_resp_set_status(request, HTTPD_200);
    const bool exists = internalStorage_queryFileExists("favicon.ico");
    esp_err_t espErr;
    if (exists) {
        uint sizeBytes;
        internalStorage_queryFileSize("favicon.ico", &sizeBytes);

        FILE *file;
        internalStorage_openFile("favicon.ico", &file, FILE_MODE_READ);
        uint bytesRemaining = sizeBytes;
        while (bytesRemaining > 0) {
            uint bytesRead;
            const uint bytesToRead = (bytesRemaining < FILE_BUFFER_SIZE) ? bytesRemaining : FILE_BUFFER_SIZE;
            internalStorage_readFileChunks(file, INTERNAL_STORAGE_POSITION_CONTINUE,
                                           favIconFileBuffer, bytesToRead, &bytesRead);
            espErr = httpd_resp_send_chunk(request, favIconFileBuffer, (ssize_t) bytesRead);
            if (espErr != ESP_OK) {
                ERROR("httpd_resp_send_chunk() returned %i : %s", espErr, esp_err_to_name(espErr));
            }
            bytesRemaining -= bytesRead;
        }
        espErr = httpd_resp_send_chunk(request, NULL, 0);
        if (espErr != ESP_OK) {
            ERROR("httpd_resp_send_chunk() returned %i : %s", espErr, esp_err_to_name(espErr));
        }
        internalStorage_closeFile(file);
    } else {
        httpd_resp_send_404(request);
    }
    return ESP_OK;
}

private esp_err_t webserver_logHandler(httpd_req_t *request) {
    capacity_t capacity = logList_getSize(logList);
    ListOptions listOptions = LIST_DEFAULT_OPTIONS;
    listOptions.capacity = capacity;
    List *list = list_createWithOptions(&listOptions);
    logList_getList(logList, list);
    const capacity_t listSize = list_getSize(list);

    cJSON *jsonObject = cJSON_CreateObject();
    cJSON *linesArray = cJSON_CreateArray();
    if (jsonObject == NULL || linesArray == NULL) {
        httpd_resp_send_500(request);
        return ESP_OK;
    }
    if (cJSON_AddNumberToObject(jsonObject, "lineCount", listSize) == false) {
        httpd_resp_send_500(request);
        return ESP_OK;
    }
    if (cJSON_AddItemToObject(jsonObject, "lines", linesArray) == false) {
        httpd_resp_send_500(request);
        return ESP_OK;
    }
    for (index_t i = 0; i < listSize; i++) {
        const char *line = list_getItem(list, i);
        if (line == NULL) continue;
        cJSON *jsonLine = cJSON_CreateString(line);
        if (jsonLine != NULL) {
            cJSON_AddItemToArray(linesArray, jsonLine);
        }
    }

    const char *json = cJSON_PrintUnformatted(jsonObject);

    if (json == NULL) {
        httpd_resp_send_500(request);
        return ESP_OK;
    }

    httpd_resp_set_type(request, "application/json");
    httpd_resp_sendstr(request, json);

    cJSON_Delete(linesArray);
    free(json);
    list_destroy(list);
    return ESP_OK;
}

private void onAppendCallback(const LogList *_logList, const char *string) {
    printf("append: %s\n", string);
}

public WebserverError webserver_init() {
    esp_err_t err;
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.max_uri_handlers = 128;
    config.uri_match_fn = httpd_uri_match_wildcard;

    INFO("Starting Web Server on port: '%d'", config.server_port);
    err = httpd_start(&server, &config);
    if (err != ESP_OK) {
        ERROR("Failed to start Web Server! %i %s", err, esp_err_to_name(err));
        return ESP_FAIL;
    }

    httpd_uri_t favIconHandler = {.uri= "/pages/favicon.*", .method= HTTP_GET, .handler= webserver_getFavIcon};
    httpd_register_uri_handler(server, &favIconHandler);

    httpd_uri_t blobHandler = {.uri= "/pages/blob*", .method= HTTP_GET, .handler= webserver_emptyHandler};
    httpd_register_uri_handler(server, &blobHandler);

    httpd_uri_t webPageHandler = {.uri= "/pages*", .method= HTTP_GET, .handler= webserver_pageHandler};
    httpd_register_uri_handler(server, &webPageHandler);

    httpd_uri_t logApiHandler = {.uri= "/api/log", .method= HTTP_GET, .handler= webserver_logHandler};
    httpd_register_uri_handler(server, &logApiHandler);

    httpd_uri_t rootHandler = {.uri= "/", .method= HTTP_GET, .handler= webserver_pageHandler};
    httpd_register_uri_handler(server, &rootHandler);

    INFO("Web Server started!");

    internalStorage_init();

    fileBuffer = alloc(FILE_BUFFER_SIZE);
    favIconFileBuffer = alloc(FILE_BUFFER_SIZE);
    logList = log_getLogList();
    logList_addOnAppendCallback(logList, onAppendCallback);

    return ESP_OK;
}
