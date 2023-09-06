#include "iot_mqtt.h"
#include "mqtt_client.h"
#include "esp_event.h"
#include "esp_log.h"
#include "iot_globals.h"
#include "iot_utils.h"
#include "iot_structs.h"

////////////////////////////////////////////////////////////
// MQTT Subscribe Even Callback Prototype
////////////////////////////////////////////////////////////
//
// void iot_mqtt_callback_prototype(void* data, esp_mqtt_event_t* event)
//
////////////////////////////////////////////////////////////

static void log_error_if_nonzero(const char *message, int error_code);
static void mqtt_app_start(void);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

static void run_mqtt_callback(esp_mqtt_event_t *event);

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

void iot_start_mqtt(void)
{
    mqtt_app_start();
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
//        .broker.address.uri = "mqtt://homeassistant.local",
        .broker.address.uri = "mqtt://172.31.1.132",
        .credentials.username = "mqtt_user",
        .credentials.authentication.password = "mqtt_pass",
    };

    iotMqttClient = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(iotMqttClient, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(iotMqttClient);
}


static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    //esp_mqtt_client_handle_t client = event->client;
    //int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;


    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        //msg_id = esp_mqtt_client_publish(iotMqttClient, "/topic/qos0", "data", 0, 0, 0);
        //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
 

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
            run_mqtt_callback(event);
            break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }

}

static void run_mqtt_callback(esp_mqtt_event_t* event) {
    char* subscribeTopic = concat(baseTopic, event->topic);
    iot_mqtt_subscribe_callback_t* callback = iot_mqtt_subscribe_get(subscribeTopic);
    void (*mqttCallback)(void*, esp_mqtt_event_t*) = callback->callbackFunc;
    mqttCallback(callback->callbackData, event);
}

void iot_send_mqtt(iot_mqtt_message_t* mqttMessage) {
    
    esp_mqtt_client_publish(iotMqttClient, concat(baseTopic, mqttMessage->topic), mqttMessage->data, 0, mqttMessage->qos, mqttMessage->retain);
    ESP_LOGI(TAG, "MQTT Topic: %s\tMQTT Message: %s\n", concat(baseTopic, mqttMessage->topic), mqttMessage->data);
}


void iot_mqtt_callback_add(char* key, iot_mqtt_subscribe_callback_t* callback) {
    HT_ADD(mqttSubscribeMap, key, callback);
}

iot_mqtt_subscribe_callback_t* iot_mqtt_subscribe_get(char* key) {
    return (iot_mqtt_subscribe_callback_t*)HT_LOOKUP(mqttSubscribeMap, key);
    
}

