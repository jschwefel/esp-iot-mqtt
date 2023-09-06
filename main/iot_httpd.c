#include "esp_http_server.h"
#include "esp_spiffs.h"
#include "esp_vfs.h"
#include "esp_log.h"
#include "http_parser.h"
#include "iot_globals.h"
#include "iot_utils.h"
#include "iot_nvs.h"
#include "iot_defines.h"
#include "iot_config.h"
#include "../URLDecode/urldecode.h"

static httpd_req_t* set_content_type(httpd_req_t* req);
static char* get_web_server_file(const char* uri);
static void update_settings(char* postString);
static esp_err_t get_handler(httpd_req_t *req);
static esp_err_t post_handler(httpd_req_t *req);


static char* get_web_server_file(const char* uri)
{
    char* filePath = NULL;
    const char slash = '/';
    if(uri[strlen(uri)-1] == slash) {
        filePath = concat(uri, "index.html");
    } else {
        filePath = (char*)uri;
    }

    struct stat st;
    if (stat(filePath, &st))
    {
        ESP_LOGE(TAG, "%s not found", filePath);
        return NULL;
    }

    char* fileData = calloc(1, st.st_size + 1);
    FILE *fp = fopen(filePath, "r");
    if (fread(fileData, 1, st.st_size, fp) == 0)
    {
        ESP_LOGE(TAG, "fread failed");
    }
    fclose(fp);
    ESP_LOGI(TAG, "Serving file: %s", filePath);
    return fileData;
}


static httpd_req_t* set_content_type(httpd_req_t* req){
    char* uriExt = get_filename_ext((char*)req->uri);
    const char* cssExt =        "css";
    const char* jsExt =         "js";
    const char* pngExt =        "image/png";
    const char* jsonExt =       "application/json";
    const char* webpExt =       "image/webp";
    const char* jpegExt =       "js";
    const char* jpgExt =        "js";

    if(strcmp(uriExt,cssExt) == 0) {
        const char* contentType = "text/css";
        ESP_ERROR_CHECK(httpd_resp_set_type(req, contentType));
    } else if(strcmp(uriExt,jsExt) == 0) {
        const char* contentType = "text/javascript";
        ESP_ERROR_CHECK(httpd_resp_set_type(req, contentType));
    } else if(strcmp(uriExt,pngExt) == 0) {
        const char* contentType = "image/png";
        ESP_ERROR_CHECK(httpd_resp_set_type(req, contentType));
    } else if(strcmp(uriExt,jsonExt) == 0) {
        const char* contentType = "application/json";
        ESP_ERROR_CHECK(httpd_resp_set_type(req, contentType));
    } else if(strcmp(uriExt,webpExt) == 0) {
        const char* contentType = "image/webp";
        ESP_ERROR_CHECK(httpd_resp_set_type(req, contentType));
    } else if(strcmp(uriExt,jpegExt) == 0) {
        const char* contentType = "image/jpeg";
        ESP_ERROR_CHECK(httpd_resp_set_type(req, contentType));
    } else if(strcmp(uriExt,jpgExt) == 0) {
        const char* contentType = "image/jpeg";
        ESP_ERROR_CHECK(httpd_resp_set_type(req, contentType));
    }
    
    return req;
}


static esp_err_t get_handler(httpd_req_t *req)
{
    int response;
    char* fileData = get_web_server_file(req->uri);
    if(fileData != NULL) {
        if(strcmp(req->uri, (char*)"/system-settings.html") == 0) {
            fileData = iot_system_settings_populate_html(fileData);
        }
        response = httpd_resp_send(set_content_type(req), fileData, HTTPD_RESP_USE_STRLEN);
        free(fileData);
    } else {
        response = httpd_resp_send_404(req);
    }
    return response;    
    //return send_req_file(req);
}

static esp_err_t iot_settings_post_handler(httpd_req_t *req) {
    size_t queryLen = httpd_req_get_url_query_len(req) + sizeof(char);
    //char* queryStr = malloc(queryLen);
    char* queryStr = NULL;
    if(queryLen > 1) {
        queryStr = malloc(queryLen);
        ESP_ERROR_CHECK(httpd_req_get_url_query_str(req, queryStr, queryLen));
        
    } else {
        size_t recv_size = req->content_len;
        queryStr = malloc(recv_size);
        
        httpd_req_recv(req, queryStr, recv_size);
    }

    char* queryStrDecode = urlDecode(queryStr);
    ESP_LOGI(TAG, "Query string: %s\n", queryStrDecode);
    free(queryStrDecode);
    iot_iot_settings_process_config_update(queryStr);
    


    
    esp_restart();
    iot_init();
}

static esp_err_t post_handler(httpd_req_t *req)
{
    /* Destination buffer for content of HTTP POST request.
     * httpd_req_recv() accepts char* only, but content could
     * as well be any binary data (needs type casting).
     * In case of string data, null termination will be absent, and
     * content length would give length of string */
    
    
    char content[req->content_len];

    

    /* Truncate if content length larger than the buffer */
    size_t recv_size = sizeof(content);

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            httpd_resp_send_408(req);
        }
        /* In case of error, returning ESP_FAIL will
         * ensure that the underlying socket is closed */
        return ESP_FAIL;
    }

    /* Send a simple response */
    const char resp[] = "URI POST Response";
    update_settings(content);
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


static void update_settings(char* postString)
{
    bool reset = false;
    char* ssidKey = IOT_KEY_WIFI_SSID;
    char* passwdKey = IOT_KEY_WIFI_PASS;
    
    char delimeter[] = "&";

    int count = tokenCount(postString, delimeter);
	
    char* tokenArray[count+1];
    stringSplitter(postString, delimeter, tokenArray);

    for(int i = 0; i <= count; i++) 
    {   

        
        char* paramArray[2];
        printf("Token Array: %s", tokenArray[1]);
        
        stringSplitter(tokenArray[i], "=", paramArray);
        if((strcmp(paramArray[0], ssidKey) == 0) || (strcmp(paramArray[0], passwdKey) == 0))
        {
            reset = true;
            if(iot_nvs_set_int_value("wifi-mode", 2))
            {
                ESP_LOGI(TAG, "wifi-mode set to 2");
            }
            else 
            {
                ESP_LOGE(TAG, "Failed to set wifi-mode");
            }

        }

        iot_nvs_set_str_value(paramArray[0], paramArray[1]);
    }

    if(reset) 
    {
        esp_restart();
    }
}

httpd_uri_t uri_wild_get = {
    .uri = "/*",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL};

httpd_uri_t uri_post_settings_iot = {
    .uri = "/iot_settings.html",
    .method = HTTP_POST,
    .handler = iot_settings_post_handler,
    .user_ctx = NULL};


httpd_uri_t uri_post = {
    .uri = "/",
    .method = HTTP_POST,
    .handler = post_handler,
    .user_ctx = NULL};

httpd_handle_t iot_start_httpd(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "",
        .partition_label = NULL,
        .max_files = 100,
        .format_if_mount_failed = false};

    esp_vfs_spiffs_register(&conf);

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    config.uri_match_fn = httpd_uri_match_wildcard;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_post_settings_iot);
        httpd_register_uri_handler(server, &uri_wild_get);
        httpd_register_uri_handler(server, &uri_post);
        
    }
    

    return server;
}