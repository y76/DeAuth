/*
 * ESP32 BLE Extended Advertisement Receiver
 * Companion code for DeAuth project
 */

#include "esp_log.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"
#include <ctype.h>
#include "nimble/nimble_port_freertos.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "esp_random.h"
#include "sdkconfig.h"
#include "driver/uart.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/base64.h"
#include "mbedtls/error.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/ecp.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "mbedtls/pk.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/aes.h"
#include "esp_log.h"

#define INSTANCE_ID 0
#define EXAMPLE_ESP_WIFI_SSID "NETGEAR68"
#define EXAMPLE_ESP_WIFI_PASS "icyvalley700"
#define EXAMPLE_ESP_MAXIMUM_RETRY 5
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define MAX_ADV_DATA_LEN 255 
#define BUF_SIZE (155)
#define UART_BUF_SIZE (255)
#define MBUF_DATA_SIZE 260 
#define ECHO_UART_PORT_NUM (1)
#define ECHO_TEST_TXD (6)
#define ECHO_TEST_RXD (7)
#define ECHO_TEST_RTS (-1)
#define ECHO_TEST_CTS (-1)
#define ECHO_UART_BAUD_RATE (115200)
#define CERT_BUFFER_SIZE 2048 
#define PUBKEY_BUFFER_SIZE 1024

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
static uint8_t saved_n_dev[32] = {0};
static uint32_t cur_timestamp = 0;
static const char *TAG = "BLE_RECEIVER";
static int ble_gap_event(struct ble_gap_event *event, void *arg);
static uint8_t own_addr_type;
struct os_mbuf_pool large_mbuf_pool;
struct os_mempool large_mbuf_mempool;
uint8_t large_mbuf_buffer[OS_MEMPOOL_BYTES(10, MBUF_DATA_SIZE)];
static uint8_t global_n_dev[32];
static uint32_t global_timestamp;
static uint8_t global_url[100];
static uint8_t global_url_len;
static uint8_t global_attest_result;
static uint32_t global_time_attest;
static uint8_t global_signature[256];
static size_t global_signature_len;
static uint32_t global_device_id = 0;
static char certificate_of_manufacturer[CERT_BUFFER_SIZE] = {0};
static char public_key_manufacturer[PUBKEY_BUFFER_SIZE] = {0};
static void IRAM_ATTR uart_rx_isr_handler(void *arg);
static char response_buffer[4096];
static int response_len = 0;
static char public_key[PUBKEY_BUFFER_SIZE] = {0};

struct __attribute__((packed)) ble_gap_disc_desc_debug
{
    uint8_t event_type;
    uint8_t length_data;
    uint8_t addr_type;
    uint8_t reserved;
    uint8_t addr[6];
    uint16_t flags;
    uint8_t phy_primary;
    uint8_t phy_secondary;
    uint32_t extended_flags;
    uint16_t more_flags;
    uint8_t *data; 
};

typedef struct
{
    uint32_t key0;
    uint32_t key1;
    uint32_t key2;
    uint32_t key3;
} sts_key_t;

typedef struct
{
    uint32_t iv0;
    uint32_t iv1;
    uint32_t iv2;
    uint32_t iv3;
} sts_iv_t;

typedef struct
{
    uint32_t key0;
    uint32_t key1;
    uint32_t key2;
    uint32_t key3;
    uint32_t key4;
    uint32_t key5;
    uint32_t key6;
    uint32_t key7;
} aes_key_t;

typedef struct {
    char *public_key;
    uint32_t challenge;
    char *hash_chain;
    uint32_t badge_id;
    uint32_t hash_chain_ind;
} deauth_message_t;

void send_to_python(double distance)
{
    char url[100];
    snprintf(url, sizeof(url), "http://192.168.1.24:8080/distance");  // Use 192.168.1.24, not 169.234.217.234
    
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,  // Increase timeout
        .transport_type = HTTP_TRANSPORT_OVER_TCP,  // Explicitly use HTTP, not HTTPS
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    
    // Send the double as raw bytes (8 bytes)
    esp_http_client_set_post_field(client, (char *)&distance, sizeof(double));
    esp_http_client_set_header(client, "Content-Type", "application/octet-stream");
    
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Distance sent to Python: %.2f meters", distance);
    } else {
        ESP_LOGE(TAG, "Failed to send distance: %s", esp_err_to_name(err));
    }
    
    esp_http_client_cleanup(client);
}

void uart_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));
    ESP_LOGI(TAG, "UART init done");
}

static void uart_task(void *pvParameters)
{
    ESP_LOGI(TAG, "UART task started");
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE + 1);
    
    if (data == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for UART buffer");
        vTaskDelete(NULL);
    }

    while (1) {
        const int rxBytes = uart_read_bytes(ECHO_UART_PORT_NUM, data, BUF_SIZE, 20 / portTICK_PERIOD_MS);
        
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            
            ESP_LOGI(TAG, "Received %d bytes from UART:", rxBytes);
        
            printf("HEX: ");
            for (int i = 0; i < rxBytes; i++) {
                printf("%02X ", data[i]);
                if ((i + 1) % 16 == 0) {
                    printf("\n     ");
                }
            }
            printf("\n");
            
            printf("ASCII: ");
            for (int i = 0; i < rxBytes; i++) {
                if (data[i] >= 32 && data[i] <= 126) {
                    printf("%c", data[i]);
                } else {
                    printf(".");
                }
            }
            printf("\n");
            
            // Check if we received 'b' followed by a double (1 byte + 8 bytes = 9 bytes minimum)
            if (rxBytes >= 9 && data[0] == 'b') {
                double distance;
                memcpy(&distance, &data[1], sizeof(double));
                ESP_LOGI(TAG, "Distance: %.2f meters", distance);
                printf("Distance: %.2f meters\n", distance);
                send_to_python(distance);
            }
            
            if (rxBytes > 6 && memcmp(&data[rxBytes - 6], "MSGEND", 6) == 0) {
                ESP_LOGI(TAG, "Received message with MSGEND marker");               
                uint8_t response[] = "ACK: Message received";
            } 
        }
    }
    free(data);
    vTaskDelete(NULL);
}

static void send_uart_data(const uint8_t *data, uint8_t data_len)
{
    const char *start_marker = "DEAUTHSTART:";
    const char *end_marker = ":DEAUTHEND";
    size_t start_marker_len = strlen(start_marker);
    size_t end_marker_len = strlen(end_marker);
    ESP_LOGI(TAG, "DATA_LEN: %d", data_len);
    char *buf = (char *)malloc(UART_BUF_SIZE);
    memset(buf, 0, UART_BUF_SIZE);

    size_t pos = 0;

    memcpy(buf, start_marker, start_marker_len);
    pos += start_marker_len;

    if (data && data_len > 0)
    {
        memcpy(buf + pos, data, data_len);
        pos += data_len;
    }

    memcpy(buf + pos, end_marker, end_marker_len);
    pos += end_marker_len;

    printf("Sending message (hex): ");
    for (size_t i = 0; i < pos; i++)
    {
        printf("%02X", (unsigned char)buf[i]);
    }
    printf("\nSending message (ASCII): ");
    for (size_t i = 0; i < pos; i++)
    {
        printf("%c", buf[i]);
    }
    printf("\n");

    ESP_LOGI(TAG, "Sending complete message:");
    uart_write_bytes(ECHO_UART_PORT_NUM, buf, pos);

    free(buf);
}

void init_large_mbuf_pool(void)
{
    int rc;

    rc = os_mempool_init(
        &large_mbuf_mempool,
        10,
        MBUF_DATA_SIZE,
        large_mbuf_buffer,
        "large_mbuf_mempool");
    assert(rc == 0);

    rc = os_mbuf_pool_init(
        &large_mbuf_pool,
        &large_mbuf_mempool,
        MBUF_DATA_SIZE,
        10);
    assert(rc == 0);
}

static void ext_adv_init(void)
{
    struct ble_gap_ext_adv_params params;
    int rc;

    if (ble_gap_ext_adv_active(INSTANCE_ID))
    {
        rc = ble_gap_ext_adv_stop(INSTANCE_ID);
        assert(rc == 0);
    }

    memset(&params, 0, sizeof(params));

    params.connectable = 0;
    params.scannable = 0;
    params.legacy_pdu = 0;
    params.own_addr_type = own_addr_type;
    params.primary_phy = BLE_HCI_LE_PHY_1M;
    params.secondary_phy = BLE_HCI_LE_PHY_2M;
    params.sid = 1;
    params.itvl_min = BLE_GAP_ADV_FAST_INTERVAL1_MIN;
    params.itvl_max = BLE_GAP_ADV_FAST_INTERVAL1_MIN;

    rc = ble_gap_ext_adv_configure(INSTANCE_ID, &params, NULL,
                                   ble_gap_event, NULL);
    assert(rc == 0);
}

static void send_ble_message(uint8_t *data, size_t data_len)
{
    const char *prefix = "-DeAuth-";
    size_t prefix_len = strlen(prefix);
    size_t total_data_len = prefix_len + data_len;

    uint8_t total_adv_length = 3 + 2 + 2 + total_data_len; // Flags + header + company ID + prefix + data

    uint8_t *adv_data = malloc(total_adv_length);
    if (adv_data == NULL)
    {
        ESP_LOGE(TAG, "Memory allocation failed!");
        return;
    }

    adv_data[0] = 0x02;
    adv_data[1] = 0x01;
    adv_data[2] = 0x06;

    adv_data[3] = total_data_len + 3;
    adv_data[4] = 0xFF;
    adv_data[5] = 0xE5;
    adv_data[6] = 0x02;

    memcpy(&adv_data[7], prefix, prefix_len);
    memcpy(&adv_data[7 + prefix_len], data, data_len);

    printf("  Complete packet (hex):\n    ");
    for (size_t i = 0; i < total_adv_length; i++) {
        printf("%02X ", adv_data[i]);
        if ((i + 1) % 16 == 0 && i < total_adv_length - 1) {
            printf("\n    ");
        }
    }
    printf("\n");

    //ESP_LOGI(tag, "Sending message (ASCII): ");
    for (size_t i = 0; i < total_adv_length; i++) {  // Only print first part
        if (adv_data[i] >= 32 && adv_data[i] <= 126) {
            printf("%c", adv_data[i]);
        } else {
            printf(".");
        }
    }

    struct os_mbuf *mbuf;
    int rc;

    if (ble_gap_ext_adv_active(INSTANCE_ID))
    {
        rc = ble_gap_ext_adv_stop(INSTANCE_ID);
        assert(rc == 0);
    }

    mbuf = os_mbuf_get_pkthdr(&large_mbuf_pool, 0);
    if (!mbuf)
    {
        ESP_LOGE(TAG, "Failed to allocate mbuf!");
        free(adv_data);
        return;
    }

    rc = os_mbuf_append(mbuf, adv_data, total_adv_length);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Failed to append to mbuf! rc=%d", rc);
        free(adv_data);
        return;
    }

    rc = ble_gap_ext_adv_set_data(INSTANCE_ID, mbuf);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Failed to set advertisement data! rc=%d", rc);
        free(adv_data);
        return;
    }

    rc = ble_gap_ext_adv_start(INSTANCE_ID, 100, 0);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Failed to start advertising! rc=%d", rc);
        free(adv_data);
        return;
    }

    free(adv_data);
    ESP_LOGI(TAG, "BLE message sent (size: %d bytes)", total_data_len);
}

static void ble_scanner_init(void)
{
    int rc;

    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Error determining address type; rc=%d", rc);
        return;
    }

    struct ble_gap_disc_params scan_params = {
        .itvl = BLE_GAP_SCAN_ITVL_MS(100),
        .window = BLE_GAP_SCAN_WIN_MS(50),
        .filter_duplicates = 0, 
        .limited = 0,           
        .passive = 0,          
        .filter_policy = 0      
    };

    rc = ble_gap_disc(own_addr_type, 0, 
                      &scan_params,
                      ble_gap_event, NULL);

    if (rc != 0)
    {
        ESP_LOGE(TAG, "Error initiating scan; rc=%d", rc);
        return;
    }
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_HEADER:
        if (evt->header_key != NULL)
        {
            response_len = 0;
            memset(response_buffer, 0, sizeof(response_buffer));
        }
        break;

    case HTTP_EVENT_ON_DATA:
        if (response_len + evt->data_len < sizeof(response_buffer))
        {
            memcpy(response_buffer + response_len, evt->data, evt->data_len);
            response_len += evt->data_len;
            response_buffer[response_len] = 0;
        }
        break;

    case HTTP_EVENT_ON_FINISH:
        if (response_len < sizeof(response_buffer))
        {
            response_buffer[response_len] = 0;
        }
        break;

    case HTTP_EVENT_ERROR:
        response_len = 0;
        memset(response_buffer, 0, sizeof(response_buffer));
        break;

    default:
        break;
    }
    return ESP_OK;
}
#define TAG "ENCRYPTION"
static int encrypt_with_public_key(const uint8_t *data, size_t data_len,
                                   const char *public_key_pem,
                                   uint8_t *encrypted_data, size_t *encrypted_len,
                                   uint8_t *out_ephemeral_public, 
                                   uint8_t *out_iv)             
{
    mbedtls_ecdh_context ctx;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_pk_context peer_pk;
    const char *pers = "gen_key";
    int ret = 0;
    uint8_t *decrypted = NULL; 
    uint8_t shared_secret[32];
    uint8_t iv[12] = {0};
    uint8_t tag[16];

    esp_fill_random(&iv, sizeof(iv));

    mbedtls_ecdh_init(&ctx);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_pk_init(&peer_pk);

    decrypted = (uint8_t *)malloc(data_len);
    if (decrypted == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate decryption buffer");
        ret = -1;
        goto cleanup;
    }

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                (const unsigned char *)pers, strlen(pers));
    if (ret != 0)
    {
        ESP_LOGE(TAG, "Failed to seed RNG: -0x%x", -ret);
        goto cleanup;
    }

    ret = mbedtls_ecdh_setup(&ctx, MBEDTLS_ECP_DP_SECP256R1);
    if (ret != 0)
    {
        ESP_LOGE(TAG, "Failed to setup ECDH: -0x%x", -ret);
        goto cleanup;
    }

    ret = mbedtls_ecdh_gen_public(&ctx.MBEDTLS_PRIVATE(grp),
                                  &ctx.MBEDTLS_PRIVATE(d),
                                  &ctx.MBEDTLS_PRIVATE(Q),
                                  mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0)
    {
        ESP_LOGE(TAG, "Failed to generate keypair: -0x%x", -ret);
        goto cleanup;
    }

    ret = mbedtls_pk_parse_public_key(&peer_pk, (const unsigned char *)public_key_pem,
                                      strlen(public_key_pem) + 1);
    if (ret != 0)
    {
        ESP_LOGE(TAG, "Failed to parse public key: -0x%x", -ret);
        goto cleanup;
    }

    const mbedtls_ecp_keypair *peer_keypair = mbedtls_pk_ec(peer_pk);
    ret = mbedtls_ecdh_compute_shared(&ctx.MBEDTLS_PRIVATE(grp),
                                      &ctx.MBEDTLS_PRIVATE(z),
                                      &peer_keypair->MBEDTLS_PRIVATE(Q),
                                      &ctx.MBEDTLS_PRIVATE(d),
                                      mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0)
    {
        ESP_LOGE(TAG, "Failed to compute shared secret: -0x%x", -ret);
        goto cleanup;
    }

    size_t secret_len;
    ret = mbedtls_mpi_write_binary(&ctx.MBEDTLS_PRIVATE(z), shared_secret, sizeof(shared_secret));
    if (ret != 0)
    {
        ESP_LOGE(TAG, "Failed to extract shared secret: -0x%x", -ret);
        goto cleanup;
    }

    mbedtls_gcm_context gcm;
    mbedtls_gcm_init(&gcm);
    ret = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, shared_secret, 256);
    if (ret != 0)
    {
        ESP_LOGE(TAG, "Failed to set GCM key: -0x%x", -ret);
        goto cleanup_gcm;
    }

    ret = mbedtls_gcm_crypt_and_tag(&gcm, MBEDTLS_GCM_ENCRYPT, data_len,
                                    iv, sizeof(iv), NULL, 0,
                                    data, encrypted_data,
                                    16, tag);
    if (ret != 0)
    {
        ESP_LOGE(TAG, "Failed to encrypt: -0x%x", -ret);
        goto cleanup_gcm;
    }

    memcpy(encrypted_data + data_len, tag, 16);
    *encrypted_len = data_len + 16;

    ESP_LOGI(TAG, "Encryption successful");

    size_t public_key_len;
    ret = mbedtls_ecp_point_write_binary(&ctx.MBEDTLS_PRIVATE(grp),
                                         &ctx.MBEDTLS_PRIVATE(Q),
                                         MBEDTLS_ECP_PF_UNCOMPRESSED,
                                         &public_key_len,
                                         out_ephemeral_public,
                                         65);

    //ESP_LOGI(TAG, "Ephemeral public key export:");
    //ESP_LOGI(TAG, "Return code: -0x%x", -ret);
    ESP_LOGI(TAG, "Exported key length: %zu", public_key_len);

    ESP_LOGI(TAG, "First 8 bytes of ephemeral public key:");
    for (int i = 0; i < 65; i++)
    {
        printf("%02x ", out_ephemeral_public[i]);
    }
    printf("\n");

    memcpy(out_iv, iv, 12);

    ESP_LOGI(TAG, "Shared secret:");
    for (int i = 0; i < 32; i++)
    {
        printf("%02x", shared_secret[i]);
    }
    printf("\n");

    ESP_LOGI(TAG, "Encrypted data + tag (%d bytes):", *encrypted_len);
    for (int i = 0; i < *encrypted_len; i++)
    {
        printf("%02x", encrypted_data[i]);
    }
    printf("\n");

    memcpy(tag, encrypted_data + data_len, 16);

    ret = mbedtls_gcm_auth_decrypt(&gcm, data_len,
                                   iv, sizeof(iv),
                                   NULL, 0,
                                   tag, 16,
                                   encrypted_data, 
                                   decrypted);

    if (ret == 0)
    {
        ESP_LOGI(TAG, "Decryption successful");
        ESP_LOGI(TAG, "Decrypted text: ");
        for (int i = 0; i < data_len; i++)
        {
            printf("%c", decrypted[i]);
        }
        printf("\n");

        ESP_LOGI(TAG, "Decrypted data (hex):");
        for (int i = 0; i < data_len; i++)
        {
            printf("%02x", decrypted[i]);
        }
        printf("\n");

    }
    else
    {
        ESP_LOGE(TAG, "Decryption failed: -0x%x", -ret);
    }

cleanup_gcm:
    mbedtls_gcm_free(&gcm);
cleanup:
    if (decrypted)
        free(decrypted); 
    mbedtls_ecdh_free(&ctx);
    mbedtls_entropy_free(&entropy);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_pk_free(&peer_pk);
    return ret;
}
static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_DISC_COMPLETE:
        ble_scanner_init();
        break;

    default:
        break;
    }
    return 0;
}

static void ble_host_task(void *param)
{
    ESP_LOGI(TAG, "BLE Host Task Started");
    nimble_port_run();
    nimble_port_freertos_deinit();
}

static void ble_sync_cb(void)
{
    int rc;

    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Error determining address type; rc=%d", rc);
        return;
    }

    ext_adv_init();

    ble_scanner_init();

    ESP_LOGI(TAG, "BLE stack synchronized");
}

static bool parse_deauth_message(const char *content, deauth_message_t *msg)
{
    const char *start_marker = "DEAUTHSTART:";
    const char *end_marker = ":DEAUTHEND";
    
    char *start = strstr((char *)content, start_marker);
    char *end = strstr((char *)content, end_marker);
    
    if (!start || !end) {
        return false;
    }
    
    start += strlen(start_marker);
    
    // Parse public key
    char *pubkey_end = strchr(start, ':');
    if (!pubkey_end) return false;
    size_t pubkey_len = pubkey_end - start;
    msg->public_key = malloc(pubkey_len + 1);
    memcpy(msg->public_key, start, pubkey_len);
    msg->public_key[pubkey_len] = '\0';

    // Parse challenge (uint32_t)
    char *chal_start = pubkey_end + 1;
    char *chal_end = strchr(chal_start, ':');
    if (!chal_end) return false;
    size_t chal_len = chal_end - chal_start;
    char chal_str[32] = {0};
    memcpy(chal_str, chal_start, chal_len < sizeof(chal_str) ? chal_len : sizeof(chal_str) - 1);
    msg->challenge = (uint32_t)strtoul(chal_str, NULL, 10);

    // Parse hash_chain (64-char hex string = 32 bytes)
    char *hash_start = chal_end + 1;
    char *hash_end = strchr(hash_start, ':');
    if (!hash_end) return false;
    size_t hash_len = hash_end - hash_start;
    
    // Store as hex string (65 bytes: 64 chars + null terminator)
    msg->hash_chain = malloc(65);  // 64 hex chars + null terminator
    memcpy(msg->hash_chain, hash_start, hash_len < 64 ? hash_len : 64);
    msg->hash_chain[64] = '\0';  // Ensure null-terminated

    // Parse badge_id
    char *badge_start = hash_end + 1;
    char *badge_end = strchr(badge_start, ':');
    if (!badge_end) return false;
    size_t badge_len = end - badge_start;
    char badge_str[32] = {0};
    memcpy(badge_str, badge_start, badge_len < sizeof(badge_str) ? badge_len : sizeof(badge_str) - 1);
    msg->badge_id = (uint16_t)strtoul(badge_str, NULL, 10);

    char *ind_start = badge_end + 1;
    char *ind_end = strchr(ind_start, ':');
    if (!ind_end) return false;
    size_t ind_len = ind_end - ind_start;
    char ind_str[32] = {0};
    memcpy(ind_str, ind_start, ind_len < sizeof(ind_str) ? ind_len : sizeof(ind_str) - 1);
    msg->hash_chain_ind = (uint32_t)strtoul(ind_str, NULL, 10);   
    return true;
}

static esp_err_t deauth_handler(httpd_req_t *req)
{
    char *content = malloc(2048);
    if (!content) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    int ret = httpd_req_recv(req, content, 2047);
    
    if (ret <= 0) {
        free(content);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    content[ret] = '\0';
    
    ESP_LOGI(TAG, "Received HTTP message (%d bytes): %s", ret, content);

    deauth_message_t msg = {0};
    if (!parse_deauth_message(content, &msg)) {
        ESP_LOGE(TAG, "Failed to parse deauth message");
        free(content);
        httpd_resp_send(req, "ERROR", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Parsed values:");
    ESP_LOGI(TAG, "Public Key:\n%s", msg.public_key);
    ESP_LOGI(TAG, "Challenge: %lu", msg.challenge);
    ESP_LOGI(TAG, "Hash Chain: %s", msg.hash_chain);
    ESP_LOGI(TAG, "Badge ID: %lu", msg.badge_id);
    ESP_LOGI(TAG, "Hash Chain Index: %lu", msg.hash_chain_ind);
    
    // TODO: Save these values to global variables or process them
    // For example:
    // global_public_key = msg.public_key;
    // global_challenge = msg.challenge;
    // global_hash_chain = msg.hash_chain;
    // global_badge_id = msg.badge_id;


    sts_key_t sts_key;           
    sts_iv_t sts_iv;

    // for now, use generic hardcoded values for development
    const char *key_string = "HELLOP@ISA2024"; // 13 chars
    const char *iv_string = "PAISA@HELLO2024"; // 13 chars

    uint16_t src_addr;
    uint16_t dst_addr;

    // Convert strings to key values (4 bytes per value)
    sts_key.key0 = *((uint32_t *)&key_string[0]); // HELL
    sts_key.key1 = *((uint32_t *)&key_string[4]); // OP@I
    sts_key.key2 = *((uint32_t *)&key_string[8]); // SA20
    sts_key.key3 = *((uint32_t *)&key_string[9]); // A202

    sts_iv.iv0 = *((uint32_t *)&iv_string[0]); // PAIS
    sts_iv.iv1 = *((uint32_t *)&iv_string[4]); // A@HE
    sts_iv.iv2 = *((uint32_t *)&iv_string[8]); // LLO2
    sts_iv.iv3 = *((uint32_t *)&iv_string[9]); // O202

    // Generate random values for key and IV using ESP32's hardware RNG
   // esp_fill_random(&sts_key, sizeof(sts_key));
   // esp_fill_random(&sts_iv, sizeof(sts_iv));
    esp_fill_random(&src_addr, sizeof(src_addr));
    esp_fill_random(&dst_addr, sizeof(dst_addr));

    ESP_LOGI(TAG, "STS KEY: 0x%08lX 0x%08lX 0x%08lX 0x%08lX",
             sts_key.key0, sts_key.key1, sts_key.key2, sts_key.key3);

    ESP_LOGI(TAG, "STS IV: 0x%08lX 0x%08lX 0x%08lX 0x%08lX",
             sts_iv.iv0, sts_iv.iv1, sts_iv.iv2, sts_iv.iv3);


    uint8_t crypto_data[36]; // 16 (STS key) + 16 (STS IV) + 2 (src_addr) + 2 (dst_addr)
    memcpy(crypto_data, &sts_key, sizeof(sts_key));
    memcpy(crypto_data + sizeof(sts_key), &sts_iv, sizeof(sts_iv));
    memcpy(crypto_data + sizeof(sts_key) + sizeof(sts_iv), &src_addr, sizeof(src_addr));
    memcpy(crypto_data + sizeof(sts_key) + sizeof(sts_iv) + sizeof(src_addr), &dst_addr, sizeof(dst_addr));
    send_uart_data(crypto_data, sizeof(crypto_data)); //SEND TO UWB MODULE

    uint8_t encrypted_sts_data[256];
    size_t encrypted_length = sizeof(encrypted_sts_data);
    uint8_t complete_message[256 + 65 + 12 + 32];
    uint8_t ephemeral_public[65];
    uint8_t iv[12];

    int ret_enc = encrypt_with_public_key(crypto_data, sizeof(crypto_data),
                                      msg.public_key,
                                      encrypted_sts_data, &encrypted_length,
                                      ephemeral_public, // Pass buffer for public key
                                      iv);              // Pass buffer for IV
    
    
    //challenge, ephemeral public key, iv, hash chain, badge, encrypted length
    if (ret_enc == 0)
    {
        size_t total_length = 0;
        memcpy(complete_message, &msg.challenge, 4); 
        total_length += 4;
        // Print what we have so far
        ESP_LOGI(TAG, "After challenge (4 bytes):");
        for (size_t i = 0; i < total_length; i++) {
            printf("%02X ", complete_message[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("\n");
        memcpy(complete_message + total_length, ephemeral_public, 65);
        total_length += 65;
        ESP_LOGI(TAG, "After ephemeral key (%zu bytes total):", total_length);
        for (size_t i = 0; i < total_length; i++) {
            printf("%02X ", complete_message[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("\n");
        memcpy(complete_message + total_length, iv, 12);
        total_length += 12;

        ESP_LOGI(TAG, "After iv (%zu bytes total):", total_length);
        for (size_t i = 0; i < total_length; i++) {
            printf("%02X ", complete_message[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("\n");

        // Convert hash_chain hex string to 32-byte array
        uint8_t hash_chain_bytes[32];
        if (strlen(msg.hash_chain) >= 64) {
            for (int i = 0; i < 32; i++) {
                char hex_byte[3] = {msg.hash_chain[i*2], msg.hash_chain[i*2+1], '\0'};
                hash_chain_bytes[i] = (uint8_t)strtoul(hex_byte, NULL, 16);
            }
        } else {
            ESP_LOGE(TAG, "Invalid hash chain length");
            // Handle error
        }

        memcpy(complete_message + total_length, hash_chain_bytes, 32);
        total_length += 32;

        memcpy(complete_message + total_length, &msg.badge_id, sizeof(msg.badge_id)); 
        total_length += sizeof(msg.badge_id);


        ESP_LOGI(TAG, "After badge (%zu bytes total):", total_length);
        for (size_t i = 0; i < total_length; i++) {
            printf("%02X ", complete_message[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("\n");

        memcpy(complete_message + total_length, &msg.hash_chain_ind, sizeof(msg.hash_chain_ind));
        total_length += sizeof(msg.hash_chain_ind);

        ESP_LOGI(TAG, "After hash_chain_ind (%zu bytes total):", total_length);
        for (size_t i = 0; i < total_length; i++) {
            printf("%02X ", complete_message[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("\n");

        memcpy(complete_message + total_length, encrypted_sts_data, encrypted_length);
        total_length += encrypted_length;


        ESP_LOGI(TAG, "After encrypted (%zu bytes total):", total_length);
        for (size_t i = 0; i < total_length; i++) {
            printf("%02X ", complete_message[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("\n");

        uint8_t computed_mac[32];
        mbedtls_md_context_t ctx;
        const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

        mbedtls_md_init(&ctx);
        mbedtls_md_setup(&ctx, md_info, 1);  // 1 = HMAC mode
        mbedtls_md_hmac_starts(&ctx, hash_chain_bytes, 32);  // Use hash_chain as MAC key
        mbedtls_md_hmac_update(&ctx, complete_message, total_length);  // MAC over all fields
        mbedtls_md_hmac_finish(&ctx, computed_mac);
        mbedtls_md_free(&ctx);

        // Append MAC to message
        memcpy(complete_message + total_length, computed_mac, 32);
        total_length += 32;

        ESP_LOGI(TAG, "Complete message with MAC (%zu bytes total):", total_length);
        for (size_t i = 0; i < total_length; i++) {
            printf("%02X ", complete_message[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("\n");


        send_ble_message(complete_message, total_length);
    }


    free(msg.public_key);
    free(content);
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void start_http_server(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.stack_size = 8192;  // Increase stack size to prevent overflow
    config.max_uri_handlers = 10;
    config.max_resp_headers = 8;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t deauth_uri = {
            .uri       = "/deauth",
            .method    = HTTP_POST,
            .handler   = deauth_handler,
        };
        httpd_register_uri_handler(server, &deauth_uri);
        ESP_LOGI(TAG, "HTTP server started on port 80");
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    uart_init();

    ESP_ERROR_CHECK(esp_netif_init());
    wifi_init_sta();

    nimble_port_init();

    init_large_mbuf_pool();

    ble_hs_cfg.sync_cb = ble_sync_cb; 
    ble_hs_cfg.reset_cb = NULL;
    ble_hs_cfg.store_status_cb = NULL;

    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
    ble_hs_cfg.sm_sc = 0;

    start_http_server();

    nimble_port_freertos_init(ble_host_task);
    xTaskCreate(uart_task, "uart_task", 4096, NULL, 10, NULL);


    ESP_LOGI(TAG, "BLE initialization completed");
}