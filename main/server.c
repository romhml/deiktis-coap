#include <string.h>
#include <sys/socket.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include "nvs_flash.h"

#include "deiktis.h"
#include "protocol_common.h"

#if 1
/* Needed until coap_dtls.h becomes a part of libcoap proper */
#include "libcoap.h"
#include "coap_dtls.h"
#endif
#include "coap.h"

#define INITIAL_DATA "Hello World!"

/*
   Note: PSK will only be used if the URI is prefixed with coaps://
   instead of coap:// and the PSK must be one that the server supports
   (potentially associated with the IDENTITY)
*/
#define COAP_PSK_KEY CONFIG_COAP_PSK_KEY
#define COAP_LOG_DEFAULT_LEVEL 7

const static char *TAG = "deiktis";

static void
hnd_leds_put(
    coap_context_t *ctx,
    coap_resource_t *resource,
    coap_session_t *session,
    coap_pdu_t *request,
    coap_binary_t *token,
    coap_string_t *query,
    coap_pdu_t *response)
{
    int index = (int)strtol((char *)request->data, (char **)NULL, 10);
    lights_up(index);
    response->code = COAP_RESPONSE_CODE(201);
}


static void coap_server(void *p)
{
    coap_context_t *ctx = NULL;
    coap_address_t serv_addr;

    coap_set_log_level(COAP_LOG_DEFAULT_LEVEL);

    while (1) {
        coap_endpoint_t *ep = NULL;
        unsigned wait_ms;

        /* Prepare the CoAP server socket */
        coap_address_init(&serv_addr);
        serv_addr.addr.sin.sin_family      = AF_INET;
        serv_addr.addr.sin.sin_addr.s_addr = INADDR_ANY;
        serv_addr.addr.sin.sin_port        = htons(COAP_DEFAULT_PORT);

        ctx = coap_new_context(NULL);
        if (!ctx) {
            ESP_LOGE(TAG, "coap_new_context() failed");
            continue;
        }

        ep = coap_new_endpoint(ctx, &serv_addr, COAP_PROTO_UDP);
        if (!ep) {
            ESP_LOGE(TAG, "udp: coap_new_endpoint() failed");
            goto clean_up;
        }

        ep = coap_new_endpoint(ctx, &serv_addr, COAP_PROTO_TCP);
        if (!ep) {
            ESP_LOGE(TAG, "tcp: coap_new_endpoint() failed");
            goto clean_up;
        }

        /* Need PSK setup before we set up endpoints */
        /* coap_context_set_psk(ctx, "CoAP",
                             (const uint8_t *)COAP_PSK_KEY,
                             sizeof(COAP_PSK_KEY) - 1);
        */
        // Init coap Resources
        coap_resource_t *leds_resource = coap_resource_init(coap_make_str_const("leds"), 0);

        coap_register_handler(leds_resource, COAP_REQUEST_PUT, hnd_leds_put);
        coap_add_resource(ctx, leds_resource);

        wait_ms = COAP_RESOURCE_CHECK_TIME * 1000;

        while (1) {
            int result = coap_run_once(ctx, wait_ms);
            if (result < 0) {
                break;
            } else if (result && (unsigned)result < wait_ms) {
                /* decrement if there is a result wait time returned */
                wait_ms -= result;
            }
            if (result) {
                /* result must have been >= wait_ms, so reset wait_ms */
                wait_ms = COAP_RESOURCE_CHECK_TIME * 1000;
            }
        }
    }

    clean_up:
        coap_free_context(ctx);
        coap_cleanup();

        vTaskDelete(NULL);

}

void app_main(void)
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(network_connect());
    init_leds();
    xTaskCreate(coap_server, "coap", 8 * 1024, NULL, 5, NULL);
}
