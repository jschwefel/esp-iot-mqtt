#include "esp_http_server.h"
#include "esp_spiffs.h"
#include "esp_vfs.h"
#include "esp_log.h"
#include "http_parser.h"
#include "iot_globals.h"
#include "iot_utils.h"
#include "iot_nvs.h"
#include "iot_defines.h"

char* get_web_server_file(const char* uri);

void update_settings(char* postString);

//char response_data[4096];
//char responseData[IOT_HTTPD_RESPONSE_BUFFER];

char* get_web_server_file(const char* uri)
{
    //This should not be needed.
    //memset((void *)fileData, 0, sizeof(fileData));

    struct stat st;


    char* filePath = NULL;
    const char slash = '/';
    if(uri[strlen(uri)-1] == slash) {
        filePath = concat("/spiffs", concat(uri, "index.html"));
    } else {
        filePath = concat("/spiffs", uri);
    }

    if (stat(filePath, &st))
    {
        ESP_LOGE(TAG, "%s not found", filePath);
        return NULL;
    }

    char* fileData = malloc((size_t)st.st_size);


    FILE *fp = fopen(filePath, "r");
    if (fread(fileData, st.st_size, 1, fp) == 0)
    {
        ESP_LOGE(TAG, "fread failed");
    }
    fclose(fp);
    //fread adds 5 garbage byts on the end, so NULLing the proper last byte
    fileData[st.st_size] = '\0';
    return fileData;
}
/* 
void init_web_page_buffer(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false};

    esp_vfs_spiffs_register(&conf);

    memset((void *)index_html, 0, sizeof(index_html));
    struct stat st;
    if (stat(INDEX_HTML_PATH, &st))
    {
        ESP_LOGE(TAG, "index.html not found");
        return;
    }

    FILE *fp = fopen(INDEX_HTML_PATH, "r");
    if (fread(index_html, st.st_size, 1, fp) == 0)
    {
        ESP_LOGE(TAG, "fread failed");
    }
    fclose(fp);
}
 */
esp_err_t send_web_page(httpd_req_t *req)
{
    
    char* fileData = get_web_server_file(req->uri);
    
    int response;

    //sprintf(response_data, fileData, iot_wifi_conf.ssid, iot_wifi_conf.passwd);
    //response = httpd_resp_send(req, response_data, HTTPD_RESP_USE_STRLEN);
    //char responseData[IOT_HTTPD_RESPONSE_BUFFER];
    //memset((void *)responseData, 0, sizeof(responseData));
    //Only works when 'responseData[] is declared global scope. Otherwise panic.
    //sprintf(responseData, fileData, iot_wifi_conf.ssid, iot_wifi_conf.passwd);
    response = httpd_resp_send(req, fileData, HTTPD_RESP_USE_STRLEN);

    return response;
}


esp_err_t get_handler(httpd_req_t *req)
{
    return send_web_page(req);
}


esp_err_t post_handler(httpd_req_t *req)
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

void update_settings(char* postString)
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

httpd_uri_t uri_index_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL};

httpd_uri_t uri_wild_get = {
    .uri = "/*",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL};

httpd_uri_t uri_post = {
    .uri = "/",
    .method = HTTP_POST,
    .handler = post_handler,
    .user_ctx = NULL};

httpd_handle_t iot_start_httpd(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false};

    esp_vfs_spiffs_register(&conf);

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    config.uri_match_fn = httpd_uri_match_wildcard;
    //init_web_page_buffer();

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_index_get);
        httpd_register_uri_handler(server, &uri_wild_get);
        httpd_register_uri_handler(server, &uri_post);
        
    }

    return server;
}