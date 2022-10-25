#include "Utils.h"
#include "Webserver.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "Logger.h"
#include "InternalStorage.h"
#include "ExternalStorage.h"
#include "cJSON.h"
#include "Battery.h"
#include "Camera.h"

#define FILE_BUFFER_SIZE 4096
#define CAMERA_IMAGE_BUFFER_SIZE 2048

private httpd_handle_t server;
private void *fileBuffer;
private void *favIconFileBuffer;
private LogList *logList;

#define requestHandler(endpoint, name) private esp_err_t requestHandler_ ## name(httpd_req_t *request)
#define allowCORS(request) httpd_resp_set_hdr(request, "Access-Control-Allow-Origin", "*")

private bool socketsListIntEquals(const ListItem *a, const ListItem *b) {
    if (a == NULL && b == NULL) return true;
    else if (a != NULL && b != NULL) {
        int numA = *((int *) a);
        int numb = *((int *) b);
        return numA == numb;
    } else {
        return false;
    }
}

typedef struct {
    List *socketsList;
    const char *string;
} WebsocketAsyncData;
private WebsocketAsyncData *asyncData;

requestHandler(NULL, 404) {
    allowCORS(request);
    INFO("URI: %s", request->uri);
    httpd_resp_send_404(request);
    return ESP_OK;
}

requestHandler("/pages*", pages) {
    allowCORS(request);
    INFO("URI: %s", request->uri);
    httpd_resp_set_status(request, HTTPD_200);
    bool exists;
    internalStorage_queryFileExists("index.html", &exists);

    esp_err_t espErr;
    if (exists) {
        FileInfo fileInfo;
        internalStorage_queryFileInfo("index.html", &fileInfo);

        FILE *file;
        internalStorage_openFile("index.html", &file, FILE_MODE_READ);
        uint32_t bytesRemaining = fileInfo.sizeBytes;
        uint32_t readPosition = 0;
        while (bytesRemaining > 0) {
            uint bytesRead;
            const uint bytesToRead = (bytesRemaining < FILE_BUFFER_SIZE) ? bytesRemaining : FILE_BUFFER_SIZE;
            internalStorage_readFile(file, readPosition,
                                     fileBuffer, bytesToRead, &bytesRead);
            espErr = httpd_resp_send_chunk(request, fileBuffer, (ssize_t) bytesRead);
            if (espErr != ESP_OK) {
                ERROR("httpd_resp_send_chunk() returned %i : %s", espErr, esp_err_to_name(espErr));
            }
            bytesRemaining -= bytesRead;
            readPosition += bytesRead;
        }
        espErr = httpd_resp_send_chunk(request, NULL, 0);
        if (espErr != ESP_OK) {
            ERROR("httpd_resp_send_chunk() returned %i : %s", espErr, esp_err_to_name(espErr));
        }
        internalStorage_closeFile(file);
    } else {
        httpd_resp_send_err(request, HTTPD_404_NOT_FOUND, "index.html could not be located");
    }
    return ESP_OK;
}

requestHandler("/pages/favicon", favIcon) {
    allowCORS(request);
    INFO("URI: %s", request->uri);
    httpd_resp_set_status(request, HTTPD_200);
    bool exists;
    internalStorage_queryFileExists("favicon.ico", &exists);

    esp_err_t espErr;
    if (exists) {
        FileInfo fileInfo;
        internalStorage_queryFileInfo("favicon.ico", &fileInfo);

        FILE *file;
        internalStorage_openFile("favicon.ico", &file, FILE_MODE_READ);
        uint32_t bytesRemaining = fileInfo.sizeBytes;
        uint32_t readPosition = 0;
        while (bytesRemaining > 0) {
            uint bytesRead;
            const uint bytesToRead = (bytesRemaining < FILE_BUFFER_SIZE) ? bytesRemaining : FILE_BUFFER_SIZE;
            internalStorage_readFile(file, readPosition, favIconFileBuffer,
                                     bytesToRead, &bytesRead);
            espErr = httpd_resp_send_chunk(request, favIconFileBuffer, (ssize_t) bytesRead);
            if (espErr != ESP_OK) {
                ERROR("httpd_resp_send_chunk() returned %i : %s", espErr, esp_err_to_name(espErr));
            }
            bytesRemaining -= bytesRead;
            readPosition += bytesRead;
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

requestHandler("/api/log", apiLog) {
    allowCORS(request);
    INFO("URI: %s", request->uri);
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

requestHandler("/api/battery", apiBattery) {
    allowCORS(request);
    /*{ isCharging:boolean, voltage:number, percentage:number }*/
    BatteryInfo batteryInfo = {};
    battery_getInfo(&batteryInfo);
    char buffer[128];
    sprintf(buffer, "{\"isCharging\":%s,\"voltage\":%.f,\"percentage\":%.f}",
            batteryInfo.isCharging ? "true" : "false",
            batteryInfo.voltage, batteryInfo.percentage);
    httpd_resp_set_type(request, "application/json");
    httpd_resp_send(request, buffer, (ssize_t) strlen(buffer));
    // TODO: 08-Aug-2022 @basshelal: Implement using cJSON?
    return ESP_OK;
}

private void cameraReadCallback(char *buffer, int bufferSize, void *userArgs) {
    httpd_req_t *request = (httpd_req_t *) userArgs;
    httpd_resp_send_chunk(request, buffer, bufferSize);
}

requestHandler("/api/camera", apiCamera) {
    allowCORS(request);
    httpd_resp_set_type(request, "image/jpeg");

    camera_readImageWithCallback(CAMERA_IMAGE_BUFFER_SIZE, cameraReadCallback, request);
    httpd_resp_send_chunk(request, NULL, 0);

    return ESP_OK;
}

requestHandler("/socket/log", socketLog) {
    allowCORS(request);
    INFO("URI: %s", request->uri);
    INFO("Method: %s", http_method_str(request->method));
    int socketNumber = httpd_req_to_sockfd(request);
    if (socketNumber == -1) return ESP_OK;
    int *socketNumberPtr = new(int);
    *socketNumberPtr = socketNumber;
    index_t foundIndex = list_indexOfItemFunction(asyncData->socketsList, socketNumberPtr, socketsListIntEquals);
    if (foundIndex == LIST_INVALID_INDEX_CAPACITY) {
        list_addItem(asyncData->socketsList, socketNumberPtr);
    } else {
        free(socketNumberPtr);
    }
    INFO("Socket fd: %i", socketNumber);
    if (request->method == HTTP_GET) {
        httpd_resp_set_status(request, HTTPD_200);
        return ESP_OK;
    }
    return ESP_OK;
}

requestHandler("/files/*", files) {
    allowCORS(request);
    INFO("URI: %s", request->uri);

    // TODO: 19-Sep-2022 @basshelal: Trim the leading "/files/" then you need to convert the URL
    //  if it contains escape/special characters ie %20 representing a space, probably need a library to convert
    //  to and from if files are saved with these characters and if URL contains them
    //  maybe use this library https://github.com/uriparser/uriparser

    const int prefixLength = strlen("/files/");
    char *uriCopy = alloc(strlen(request->uri) - prefixLength + 1);
    strcpy(uriCopy, request->uri + prefixLength);

    INFO("%s", uriCopy);

    httpd_resp_sendstr(request, uriCopy);
    return ESP_OK;
}

private void sendLogToWebSocketClients(void *arg) {
    WebsocketAsyncData *asyncDataLocal = (WebsocketAsyncData *) arg;
    if (!asyncDataLocal || !asyncDataLocal->socketsList) return;
    ListOptions options = LIST_DEFAULT_OPTIONS;
    options.capacity = list_getSize(asyncDataLocal->socketsList);
    List *indicesToRemove = list_create();
    for (int i = 0; i < list_getSize(asyncDataLocal->socketsList); i++) {
        int *socketPtr = list_getItem(asyncDataLocal->socketsList, i);
        if (!socketPtr) continue;
        httpd_ws_frame_t websocketFrame = {};
        websocketFrame.type = HTTPD_WS_TYPE_TEXT;
        websocketFrame.payload = asyncDataLocal->string;
        websocketFrame.len = strlen(asyncDataLocal->string);
        esp_err_t err = httpd_ws_send_frame_async(server, *socketPtr, &websocketFrame);
        if (err) {
            if (err == ESP_ERR_INVALID_ARG) {
                int *indexPtr = new(int);
                *indexPtr = i;
                list_addItem(indicesToRemove, indexPtr);
            }
        }
    }
    for (int i = 0; i < list_getSize(indicesToRemove); i++) {
        int *indexPtr = list_getItem(indicesToRemove, i);
        int *socketNumber = list_getItem(asyncDataLocal->socketsList, *indexPtr);
        list_removeItemIndexed(asyncDataLocal->socketsList, *indexPtr);
        free(indexPtr);
        free(socketNumber);
    }
    list_destroy(indicesToRemove);
}

private void onAppendCallback(const LogList *_logList, const char *string) {
    asyncData->string = string;
    httpd_queue_work(server, sendLogToWebSocketClients, asyncData);
}

public Error webserver_init() {
    esp_err_t err;
    server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.max_uri_handlers = 128;
    config.uri_match_fn = httpd_uri_match_wildcard;

    INFO("Starting Web Server on port: '%d'", config.server_port);

    if ((err = httpd_start(&server, &config))) {
        throwESPError(httpd_start(), err);
    }

    httpd_uri_t favIconHandler = {.uri= "/pages/favicon.*", .method= HTTP_GET, .handler= requestHandler_favIcon};
    httpd_register_uri_handler(server, &favIconHandler);

    httpd_uri_t blobHandler = {.uri= "/pages/blob*", .method= HTTP_GET, .handler= requestHandler_404};
    httpd_register_uri_handler(server, &blobHandler);

    httpd_uri_t webPageHandler = {.uri= "/pages*", .method= HTTP_GET, .handler= requestHandler_pages};
    httpd_register_uri_handler(server, &webPageHandler);

    httpd_uri_t logApiHandler = {.uri= "/api/log", .method= HTTP_GET, .handler= requestHandler_apiLog};
    httpd_register_uri_handler(server, &logApiHandler);

    httpd_uri_t batteryApiHandler = {.uri= "/api/battery", .method= HTTP_GET, .handler= requestHandler_apiBattery};
    httpd_register_uri_handler(server, &batteryApiHandler);

    httpd_uri_t cameraApiHandler = {.uri= "/api/camera", .method= HTTP_GET, .handler= requestHandler_apiCamera};
    httpd_register_uri_handler(server, &cameraApiHandler);

    httpd_uri_t logSocketHandler = {.uri= "/socket/log", .method= HTTP_GET, .handler= requestHandler_socketLog,
            .is_websocket= true};
    httpd_register_uri_handler(server, &logSocketHandler);

    httpd_uri_t filesApiHandler = {.uri= "/files/*", .method= HTTP_GET, .handler= requestHandler_files};
    httpd_register_uri_handler(server, &filesApiHandler);

    httpd_uri_t rootHandler = {.uri= "/", .method= HTTP_GET, .handler= requestHandler_pages};
    httpd_register_uri_handler(server, &rootHandler);

    INFO("Web Server started!");

    internalStorage_init();

    fileBuffer = alloc(FILE_BUFFER_SIZE);
    favIconFileBuffer = alloc(FILE_BUFFER_SIZE);
    logList = log_getLogList();
    logList_addOnAppendCallback(logList, onAppendCallback);
    asyncData = new(WebsocketAsyncData);
    ListOptions socketsListOptions = LIST_DEFAULT_OPTIONS;
    socketsListOptions.isGrowable = true;
    socketsListOptions.capacity = CONFIG_LWIP_MAX_SOCKETS;
    asyncData->socketsList = list_createWithOptions(&socketsListOptions);

    return ERROR_NONE;
}
