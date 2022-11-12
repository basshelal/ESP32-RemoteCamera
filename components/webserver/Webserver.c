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
#define CAMERA_IMAGE_BUFFER_SIZE 4096
#define CAMERA_SETTINGS_JSON_BUFFER_SIZE 1024

typedef struct {
    int fd; // socket file descriptor, used by ESP-IDF to send Web Socket Frames
    size_t bytesSent; // keep track of this to determine if any data is missing or if starting from a continuation frame
} CameraWebSocket;

private struct {
    bool isInitialized;
    httpd_handle_t server;
    void *pageBuffer;
    void *favIconBuffer;
    char *imageBuffer;
    char *cameraSettingsJSONBuffer;
    LogList *logList;
    struct {
        List *socketsList; // list of sockets, a socket is an int
        const char *logString; // string to send through log websocket
    } logWebsocketData;
    struct {
        List *socketsList; // list of sockets, a socket is an int
        uint8_t *imageBuffer;
        size_t imageBufferLength;
        size_t bytesRead;
        size_t bytesRemaining;
    } cameraWebsocketData;
} this;

#define requestHandler(name, uri) private esp_err_t requestHandler_ ## name(httpd_req_t *request)
#define allowCORS(request) httpd_resp_set_hdr(request, "Access-Control-Allow-Origin", "*")
#define finishRequest(request) httpd_resp_send_chunk(request, NULL, 0)
#define addEndpoint(_uri, _method, _handler) \
do{                                       \
httpd_uri_t uriHandler = {.uri= _uri, .method= _method, .handler= requestHandler_ ## _handler};\
httpd_register_uri_handler(this.server, &uriHandler);\
} while(0)

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

private bool cameraSocketsListEquals(const ListItem *a, const ListItem *b) {
    if (a == NULL && b == NULL) return true;
    else if (a != NULL && b != NULL) {
        CameraWebSocket socketA = *((CameraWebSocket *) a);
        CameraWebSocket socketB = *((CameraWebSocket *) b);
        return socketA.fd == socketB.fd;
    } else {
        return false;
    }
}

requestHandler(404, NULL) {
    allowCORS(request);
    INFO("URI: %s", request->uri);
    httpd_resp_send_404(request);
    return ESP_OK;
}

requestHandler(pages, "/pages*") {
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
                                     this.pageBuffer, bytesToRead, &bytesRead);
            espErr = httpd_resp_send_chunk(request, this.pageBuffer, (ssize_t) bytesRead);
            if (espErr != ESP_OK) {
                ERROR("httpd_resp_send_chunk() returned %i : %s", espErr, esp_err_to_name(espErr));
            }
            bytesRemaining -= bytesRead;
            readPosition += bytesRead;
        }
        espErr = finishRequest(request);
        if (espErr != ESP_OK) {
            ERROR("httpd_resp_send_chunk() returned %i : %s", espErr, esp_err_to_name(espErr));
        }
        internalStorage_closeFile(file);
    } else {
        httpd_resp_send_err(request, HTTPD_404_NOT_FOUND, "index.html could not be located");
    }
    return ESP_OK;
}

requestHandler(favIcon, "/pages/favicon") {
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
            internalStorage_readFile(file, readPosition, this.favIconBuffer,
                                     bytesToRead, &bytesRead);
            espErr = httpd_resp_send_chunk(request, this.favIconBuffer, (ssize_t) bytesRead);
            if (espErr != ESP_OK) {
                ERROR("httpd_resp_send_chunk() returned %i : %s", espErr, esp_err_to_name(espErr));
            }
            bytesRemaining -= bytesRead;
            readPosition += bytesRead;
        }
        espErr = finishRequest(request);
        if (espErr != ESP_OK) {
            ERROR("httpd_resp_send_chunk() returned %i : %s", espErr, esp_err_to_name(espErr));
        }
        internalStorage_closeFile(file);
    } else {
        httpd_resp_send_404(request);
    }
    return ESP_OK;
}

requestHandler(apiLog, "/api/log") {
    allowCORS(request);
    INFO("URI: %s", request->uri);
    capacity_t capacity = logList_getSize(this.logList);
    ListOptions listOptions = LIST_DEFAULT_OPTIONS;
    listOptions.capacity = capacity;
    List *list = list_createWithOptions(&listOptions);
    logList_getList(this.logList, list);
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
    delete(json);
    list_destroy(list);
    return ESP_OK;
}

requestHandler(apiBattery, "/api/battery") {
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
    esp_err_t err = httpd_resp_send_chunk(request, buffer, bufferSize);
    if (err) {
        ERROR("httpd_resp_send_chunk() returned: %i: %s", err, esp_err_to_name(err));
    }
}

requestHandler(cameraSettings, "/api/cameraSettings") {
    allowCORS(request);

    memset(this.cameraSettingsJSONBuffer, 0, CAMERA_SETTINGS_JSON_BUFFER_SIZE);
    httpd_req_recv(request, this.cameraSettingsJSONBuffer, CAMERA_SETTINGS_JSON_BUFFER_SIZE);
    INFO("JSON: %s", this.cameraSettingsJSONBuffer);

    cJSON *json = cJSON_ParseWithOpts(this.cameraSettingsJSONBuffer, NULL, true);

    cJSON *imageSize = cJSON_GetObjectItemCaseSensitive(json, "imageSize");
    if (cJSON_IsNumber(imageSize)) {
        camera_setImageSize(imageSize->valueint);
    }

    cJSON *minutesUntilStandby = cJSON_GetObjectItemCaseSensitive(json, "minutesUntilStandby");
    if (cJSON_IsString(minutesUntilStandby) && minutesUntilStandby->valuestring != NULL) {
        // TODO: 12-Nov-2022 @basshelal: Implement
    }

    cJSON *saturation = cJSON_GetObjectItemCaseSensitive(json, "saturation");
    if (cJSON_IsNumber(saturation)) {
        camera_setSaturation(saturation->valueint);
    }

    cJSON *brightness = cJSON_GetObjectItemCaseSensitive(json, "brightness");
    if (cJSON_IsNumber(brightness)) {
        camera_setBrightness(brightness->valueint);
    }

    cJSON *contrast = cJSON_GetObjectItemCaseSensitive(json, "contrast");
    if (cJSON_IsNumber(contrast)) {
        camera_setContrast(contrast->valueint);
    }

    cJSON *hue = cJSON_GetObjectItemCaseSensitive(json, "hue");
    if (cJSON_IsNumber(hue)) {
        camera_setHue(hue->valueint);
    }

    cJSON *exposure = cJSON_GetObjectItemCaseSensitive(json, "exposure");
    if (cJSON_IsNumber(exposure)) {
        camera_setExposure(exposure->valueint);
    }

    cJSON *sharpness = cJSON_GetObjectItemCaseSensitive(json, "sharpness");
    if (cJSON_IsNumber(sharpness)) {
        camera_setSharpness(sharpness->valueint);
    }

    cJSON *imageQuality = cJSON_GetObjectItemCaseSensitive(json, "imageQuality");
    if (cJSON_IsNumber(imageQuality)) {
        camera_setImageQuality(imageQuality->valueint);
    }

    cJSON_Delete(json);

    httpd_resp_set_status(request, HTTPD_200);
    finishRequest(request);

    return ESP_OK;
}

requestHandler(apiCamera, "/api/camera") {
    allowCORS(request);

    uint32_t imageSize;
    Error err = camera_captureImage(&imageSize);
    INFO("Captured image size: %u", imageSize);
    if (err == ERROR_NONE) {
        httpd_resp_set_type(request, "image/jpeg");
        camera_readImageBufferedWithCallback(this.imageBuffer, CAMERA_IMAGE_BUFFER_SIZE, imageSize,
                                             cameraReadCallback, request);
        finishRequest(request);
    } else {
        httpd_resp_send_err(request, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Unknown error occurred capturing image");
    }

    return ESP_OK;
}

requestHandler(wsLog, "/ws/log") {
    allowCORS(request);
    INFO("URI: %s", request->uri);
    INFO("Method: %s", http_method_str(request->method));
    int socketNumber = httpd_req_to_sockfd(request);
    if (socketNumber == -1) return ESP_OK;
    int *socketNumberPtr = new(int);
    *socketNumberPtr = socketNumber;
    index_t foundIndex = list_indexOfItemFunction(this.logWebsocketData.socketsList, socketNumberPtr,
                                                  socketsListIntEquals);
    if (foundIndex == LIST_INVALID_INDEX_CAPACITY) {
        INFO("New socket fd: %i", socketNumber);
        list_addItem(this.logWebsocketData.socketsList, socketNumberPtr);
    } else {
        delete(socketNumberPtr);
    }
    if (request->method == HTTP_GET) {
        return ESP_OK;
    }
    return ESP_OK;
}

requestHandler(wsCamera, "/ws/camera") {
    allowCORS(request);
    INFO("URI: %s", request->uri);
    INFO("Method: %s", http_method_str(request->method));
    int socketNumber = httpd_req_to_sockfd(request);
    if (socketNumber == -1) return ESP_OK;
    CameraWebSocket *cameraWebSocket = new(CameraWebSocket);
    cameraWebSocket->fd = socketNumber;
    cameraWebSocket->bytesSent = 0;
    index_t foundIndex = list_indexOfItemFunction(this.cameraWebsocketData.socketsList, cameraWebSocket,
                                                  cameraSocketsListEquals);
    if (foundIndex == LIST_INVALID_INDEX_CAPACITY) {
        INFO("New socket fd: %i", socketNumber);
        list_addItem(this.cameraWebsocketData.socketsList, cameraWebSocket);
    } else {
        delete(cameraWebSocket);
    }
    if (request->method == HTTP_GET) {
        return ESP_OK;
    }
    return ESP_OK;
}

requestHandler(files, "/files/*") {
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
    typeof(this.logWebsocketData) *websocketDataPtr = ((typeof(this.logWebsocketData) *) arg);
    if (!websocketDataPtr) return;
    typeof(this.logWebsocketData) websocketData = *websocketDataPtr;
    for (int i = 0; i < list_getSize(websocketData.socketsList); i++) {
        int *socketPtr = list_getItem(websocketData.socketsList, i);
        if (!socketPtr) continue;
        int socketNumber = *socketPtr;
        httpd_ws_frame_t websocketFrame = {
                .type = HTTPD_WS_TYPE_TEXT,
                .payload = websocketData.logString,
                .len = strlen(websocketData.logString),
        };
        esp_err_t err = httpd_ws_send_frame_async(this.server, socketNumber, &websocketFrame);
        if (err) {
            if (err == ESP_ERR_INVALID_ARG) {
                list_removeItemIndexed(websocketData.socketsList, i);
                delete(socketPtr);
            }
        }
    }
    list_shrink(websocketData.socketsList);
}

private void logListOnAppendCallback(const LogList *_logList, const char *string) {
    this.logWebsocketData.logString = string;
    sendLogToWebSocketClients(&this.logWebsocketData);
    // TODO: 29-Oct-2022 @basshelal: Verify that this synchronous approach works
//    httpd_queue_work(this.server, sendLogToWebSocketClients, &this.logWebsocketData);
}

private void sendLiveImageToWebsocketClients(void *arg) {
    typeof(this.cameraWebsocketData) *websocketDataPtr = ((typeof(this.cameraWebsocketData) *) arg);
    if (!websocketDataPtr) return;
    typeof(this.cameraWebsocketData) websocketData = *websocketDataPtr;
    const uint8_t *buffer = websocketData.imageBuffer;
    const size_t bufferLength = websocketData.imageBufferLength;
    const size_t bytesRead = websocketData.bytesRead;
    const size_t bytesRemaining = websocketData.bytesRemaining;
    const bool isFinalFrame = bytesRemaining == 0;
    const bool isFirstFrame = bytesRead == bufferLength;
    for (int i = 0; i < list_getSize(websocketData.socketsList); i++) {
        CameraWebSocket *cameraWebSocket = list_getItem(websocketData.socketsList, i);
        if (!cameraWebSocket) continue;
        int socketNumber = cameraWebSocket->fd;
        if (httpd_ws_get_fd_info(this.server, socketNumber) != HTTPD_WS_CLIENT_WEBSOCKET) {
            list_removeItemIndexed(websocketData.socketsList, i);
            delete(cameraWebSocket);
            INFO("Removed socket fd: %i", socketNumber);
            continue;
        }
        size_t bytesSent = cameraWebSocket->bytesSent;
        if ((bytesSent + bufferLength) == bytesRead) {
            httpd_ws_frame_t websocketFrame = {
                    .type = isFirstFrame ? HTTPD_WS_TYPE_BINARY : HTTPD_WS_TYPE_CONTINUE,
                    .payload = buffer,
                    .len = bufferLength,
                    .fragmented = true,
                    .final = isFinalFrame
            };
            esp_err_t err = httpd_ws_send_frame_async(this.server, socketNumber, &websocketFrame);
            if (err) {
                if (err == ESP_ERR_INVALID_ARG) {
                    list_removeItemIndexed(websocketData.socketsList, i);
                    delete(cameraWebSocket);
                    INFO("Removed socket fd: %i", socketNumber);
                }
            } else {
                if (isFinalFrame) {
                    cameraWebSocket->bytesSent = 0;
                } else {
                    cameraWebSocket->bytesSent += bufferLength;
                }
            }
        }
    }
    list_shrink(websocketData.socketsList);
}

private void cameraLiveCaptureCallback(uint8_t *buffer, size_t bufferLength,
                                       size_t bytesRead, size_t bytesRemaining) {

    this.cameraWebsocketData.imageBuffer = buffer;
    this.cameraWebsocketData.imageBufferLength = bufferLength;
    this.cameraWebsocketData.bytesRead = bytesRead;
    this.cameraWebsocketData.bytesRemaining = bytesRemaining;
    sendLiveImageToWebsocketClients(&this.cameraWebsocketData);
}

public Error webserver_init() {
    if (this.isInitialized) {
        WARN("WebServer has already been initialized");
        return ERROR_NONE;
    }
    esp_err_t err;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.max_uri_handlers = 128;
    config.uri_match_fn = httpd_uri_match_wildcard;

    INFO("Starting Web Server on port: '%d'", config.server_port);

    if ((err = httpd_start(&this.server, &config))) {
        throwESPError(httpd_start(), err);
    }

    addEndpoint("/pages/favicon.*", HTTP_GET, favIcon);
    addEndpoint("/pages*", HTTP_GET, pages);
    addEndpoint("/api/log", HTTP_GET, apiLog);
    addEndpoint("/api/battery", HTTP_GET, apiBattery);
    addEndpoint("/api/camera", HTTP_GET, apiCamera);
    addEndpoint("/api/cameraSettings", HTTP_POST, cameraSettings);
    addEndpoint("/files/*", HTTP_GET, files);
    addEndpoint("/", HTTP_GET, pages);
    httpd_uri_t logWebsocketHandler = {
            .uri= "/ws/log",
            .method= HTTP_GET,
            .handler= requestHandler_wsLog,
            .is_websocket= true
    };
    httpd_register_uri_handler(this.server, &logWebsocketHandler);
    httpd_uri_t cameraWebsocketHandler = {
            .uri= "/ws/camera",
            .method= HTTP_GET,
            .handler= requestHandler_wsCamera,
            .is_websocket= true
    };
    httpd_register_uri_handler(this.server, &cameraWebsocketHandler);

    internalStorage_init();

    this.pageBuffer = alloc(FILE_BUFFER_SIZE);
    this.favIconBuffer = alloc(FILE_BUFFER_SIZE);
    this.imageBuffer = alloc(CAMERA_IMAGE_BUFFER_SIZE);
    this.cameraSettingsJSONBuffer = alloc(CAMERA_SETTINGS_JSON_BUFFER_SIZE);
    this.logList = log_getLogList();
    logList_addOnAppendCallback(this.logList, logListOnAppendCallback);
    ListOptions socketsListOptions = LIST_DEFAULT_OPTIONS;
    socketsListOptions.isGrowable = true;
    socketsListOptions.isShrinkable = false;
    socketsListOptions.capacity = CONFIG_LWIP_MAX_SOCKETS;
    this.logWebsocketData.socketsList = list_createWithOptions(&socketsListOptions);
    this.cameraWebsocketData.socketsList = list_createWithOptions(&socketsListOptions);

    camera_setCameraLiveCaptureCallback(cameraLiveCaptureCallback);

    this.isInitialized = true;

    INFO("Web Server successfully initialized!");
    return ERROR_NONE;
}
