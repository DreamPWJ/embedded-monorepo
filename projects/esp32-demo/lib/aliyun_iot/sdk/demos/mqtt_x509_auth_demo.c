/*
 * 这个例程适用于`Linux`这类支持pthread的POSIX设备, 它演示了用SDK配置MQTT参数并建立连接, 之后创建2个线程
 *
 * + 一个线程用于保活长连接
 * + 一个线程用于接收消息, 并在有消息到达时进入默认的数据回调, 在连接状态变化时进入事件回调
 *
 * 注意: 本文件中的MQTT连接, 已经用 aiot_mqtt_setopt() 接口配置成了使用X509双向认证的方式进行底层TLS握手
 *
 * 需要用户关注或修改的部分, 已经用 TODO 在注释中标明
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_mqtt_api.h"
#include "core_string.h"

/*
 * TODO: 用户需按照以下格式, 将从控制台下载到的证书, 写入替换以下字符串
 *
 * 实际量产时, 设备上应该把证书存储到特定区域以方便产线烧写和确保数据安全
 *
 */
const char client_cert[] = {
    "-----BEGIN CERTIFICATE-----\r\n" \
    "MIIDhzCCAm+gAwIBAgIHV92LVv1rJzANBgkqhkiG9w0BAQsFADBTMSgwJgYDVQQD\r\n" \
    "DB9BbGliYWJhIENsb3VkIElvVCBPcGVyYXRpb24gQ0ExMRowGAYDVQQKDBFBbGli\r\n" \
    "YWJhIENsb3VkIElvVDELMAkGA1UEBhMCQ04wIBcNMjAwMjA5MDkxNTQ2WhgPMjEy\r\n" \
    "MDAyMDkwOTE1NDZaMFExJjAkBgNVBAMMHUFsaWJhYmEgQ2xvdWQgSW9UIENlcnRp\r\n" \
    "ZmljYXRlMRowGAYDVQQKDBFBbGliYWJhIENsb3VkIElvVDELMAkGA1UEBhMCQ04w\r\n" \
    "ggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCSfWyTDZRTTBtx+B/DPmDx\r\n" \
    "cVFK3na/6+w2K4GYx4u9zWAQ2svcxezrCF6+SqS5pQO4efQ+uLEC22+V8NHdFUBH\r\n" \
    "EZysp4vhvDyOFL/mLMAbn3od34G4cvuXqh/qhhk6inSaxUhBcyUmud5o999h+xkw\r\n" \
    "46tjzOZn+t35FqV8DRb6Zl1RoGNTrQib7gxVGa1P3Yhiqfnb5asmI0E7zln8PsNZ\r\n" \
    "4sjiEqyX6syMymU5iYymamVHhligk+IcA1QkrVDkYjmSweYp2iR7n5NypXq64Euw\r\n" \
    "Ef8ELJnDgkZ4mZv2Vk1Pofp3cEdX6SfqD3joFUhBItLk1d2j8iHibmvL7xWs2S83\r\n" \
    "AgMBAAGjYDBeMB8GA1UdIwQYMBaAFIo3m6hwzdX5SMiXfiGfWW9JjiQRMB0GA1Ud\r\n" \
    "DgQWBBSYQrmiGDvHUjEVhlIL2bfgNO5o+jAOBgNVHQ8BAf8EBAMCA/gwDAYDVR0T\r\n" \
    "AQH/BAIwADANBgkqhkiG9w0BAQsFAAOCAQEAoENsEhFnP53qUDmF7ENwS0MEmCua\r\n" \
    "nTLvRhkP0jgJo3tmGe+OT0N/+nRS/AUZw7C79BpqRWp2QFEZpcjZS/NxmonxmKEC\r\n" \
    "AJzVO5gWXf1GJ6+fkaCitF5W8bnB1A4RnVQN+/heLh2+A+Ik3Gduzgizq7BlW8IY\r\n" \
    "2LYgy6J/v/5visJAJ2YRhstUPs2cbFfJXdzeGaw7Kz7YUlGZMxLK10VYYtRtQEG+\r\n" \
    "oLNwxrJp6RyS0f2/5dxjFvE/rEhDVQbEwrRZLlLeqB3KNEFFIn2uVQHFaUUEdj4Z\r\n" \
    "ivoD0EILYv2r/jweN3ahxZfcbZ44K1xa+EfHW1GrQz4UTLQH2hwbLRgeAw==\r\n" \
    "-----END CERTIFICATE-----\r\n" \
};

/*
 * TODO: 用户需按照以下格式, 将从控制台下载到的私钥, 写入替换以下字符串
 *
 * 实际量产时, 设备上应该把私钥存储到特定区域以方便产线烧写和确保数据安全
 *
 */
const char client_private_key[] = {
    "-----BEGIN RSA PRIVATE KEY-----\r\n" \
    "MIIEogIBAAKCAQEAkn1skw2UU0wbcfgfwz5g8XFRSt52v+vsNiuBmMeLvc1gENrL\r\n" \
    "3MXs6whevkqkuaUDuHn0PrixAttvlfDR3RVARxGcrKeL4bw8jhS/5izAG596Hd+B\r\n" \
    "uHL7l6of6oYZOop0msVIQXMlJrneaPffYfsZMOOrY8zmZ/rd+RalfA0W+mZdUaBj\r\n" \
    "U60Im+4MVRmtT92IYqn52+WrJiNBO85Z/D7DWeLI4hKsl+rMjMplOYmMpmplR4ZY\r\n" \
    "oJPiHANUJK1Q5GI5ksHmKdoke5+TcqV6uuBLsBH/BCyZw4JGeJmb9lZNT6H6d3BH\r\n" \
    "V+kn6g946BVIQSLS5NXdo/Ih4m5ry+8VrNkvNwIDAQABAoIBABIiZrt5leAN7uPX\r\n" \
    "9I6l/ThGb+rVyVuO6Cn4js2L/lebwgW0IEKPWfnqilgCQ4wbym8e4caV9IvHAHRO\r\n" \
    "YJx+0fs6SevxvdZPCCwKk4r6BTomLubd0WA1E8I9tD/DJAJkO3UhcQVxLKsznT3f\r\n" \
    "WY72l7K+rGvpZKiAnNQGIqxNroeGzOusG0rjE6vLgJeK/1mGr++p4fL8ogskWIVD\r\n" \
    "2L1BgHNwddFN95lmNWH5PlsIgxbFNF8VH9U+UH8BNa+8ibyHoAcwStSsX29t4P4U\r\n" \
    "r7a5JGIZi43O+afVU1hslTwwxFt6JLmcHV2beR6uUuzjh/P8TkqDhGWoCWzHQmAq\r\n" \
    "kBBQptECgYEA/DH7bp9fwtwMpi59DGu6R6zuTrXAkhLPJX7ZRAbZetHCk8hTQsNm\r\n" \
    "Bbzp6Hlzk/507H8h6iXsYBBkJ4/YurfMj+xdBcUG8PTCsv6ThPk3pAseXGikfJJt\r\n" \
    "0/+pUvhkkgBXlb030qsLjP4YY7ExgMlNH1t0ufjd4EP4GfgewPflfo8CgYEAlLMw\r\n" \
    "/rUMKIjH/FD4fIYq+A+S6aFw08oUKI+6fUOccxGcVXuSpBFchSxf1H59Fj6/Yh4k\r\n" \
    "KHrm8LpQzZK1jcdQwxaviIw62ozI+/hTCAuILqBAm2bp+xZqximM3khRUuugV0sH\r\n" \
    "O1mRQJvFjhea5MwbrhVcQFBo8gt+A5dDVssUmNkCgYBBHEZl0Q/QJy+819PBGS8G\r\n" \
    "wkbkW1hUXjbM32sIfRw48V7i+J1GZ1w3rwZU1sZYNyHIzSncYd4dDx5MeH7j9gAC\r\n" \
    "SGvbyXp5SzfZLpC3jAApghVclkehQczJJwB4Q1jzuNLj/e4jnbVluVRiqKS3M0GO\r\n" \
    "Dvab7PybofC0A7Ms7tN5UQKBgHGf00pt0ZKPojD9NkMAyoiubdY0VGChQ3ITEa9y\r\n" \
    "IHQU+t6fBFh2I7pnQ/q9hJug6uDwozSDZUCBPgk3l590tBO+m/a9IKOrfFB9WgUF\r\n" \
    "utPWBEg7BYOlh4VQbqHTpMC159mMLUR+lm1GGlkPVH6MMIJ/M/S4/NgnS+02gnAp\r\n" \
    "0d1RAoGAWOjg0b5iwcIzy1yJvSdUSBdR1juY7dnDmZXnevLS2w8qSB70dvd+SQH+\r\n" \
    "N6M4UYPergu3e0Xo2z7k/JIe3DbExFs0dhFk1ginViU+hlBIiSxj0X4XHhV6ARyZ\r\n" \
    "pOxlriVbXr4QIvSbVNx7IawoJkOmVDH6xASvP8FTat9SrYV159c=\r\n" \
    "-----END RSA PRIVATE KEY-----\r\n" \
};

/* 位于portfiles/aiot_port文件夹下的系统适配函数集合 */
extern aiot_sysdep_portfile_t g_aiot_sysdep_portfile;

/* 位于external/ali_ca_cert.c中的服务器证书 */
extern const char *ali_ca_cert;

static pthread_t g_mqtt_process_thread;
static pthread_t g_mqtt_recv_thread;
static uint8_t g_mqtt_process_thread_running = 0;
static uint8_t g_mqtt_recv_thread_running = 0;
static char *g_product_key = NULL;
static char *g_device_name = NULL;

/* TODO: 如果要关闭日志, 就把这个函数实现为空, 如果要减少日志, 可根据code选择不打印
 *
 * 例如: [1577589489.033][LK-0317] {YourDeviceName}&{YourProductKey}
 *
 * 上面这条日志的code就是0317(十六进制), code值的定义见core/aiot_state_api.h
 *
 */

/* 日志回调函数, SDK的日志会从这里输出 */
int32_t demo_state_logcb(int32_t code, char *message)
{
    printf("%s", message);
    return 0;
}

/* 获取云端下发的设备信息, 包括 productKey 和 deviceName */

/* TODO: 用户需要在这个回调函数中把云端下推的 productKey 和 deviceName 做持久化的保存, 后面使用SDK的其它接口都会用到 */

static void demo_get_device_info(const char *topic, uint16_t topic_len, const char *payload, uint32_t payload_len)
{
    const char *target_topic = "/ext/auth/identity/response";
    char *p_product_key = NULL;
    uint32_t product_key_len = 0;
    char *p_device_name = NULL;
    uint32_t device_name_len = 0;
    int32_t res = STATE_SUCCESS;

    if (topic_len != strlen(target_topic) || memcmp(topic, target_topic, topic_len) != 0) {
        return;
    }

    /* TODO: 此处为说明上的方便, 使用了SDK内部接口 core_json_value(), 这不是一个官方API, 未来有可能变化

             用户实际使用时, 需要换成用自己设备上可用的JSON解析函数库的接口处理payload, 比如流行的 cJSON 等
    */
    res = core_json_value(payload, payload_len, "productKey", strlen("productKey"), &p_product_key, &product_key_len);
    if (res < 0) {
        return;
    }
    res = core_json_value(payload, payload_len, "deviceName", strlen("deviceName"), &p_device_name, &device_name_len);
    if (res < 0) {
        return;
    }

    if (g_product_key == NULL) {
        g_product_key = malloc(product_key_len + 1);
        if (NULL == g_product_key) {
            return;
        }

        memset(g_product_key, 0, product_key_len + 1);
        memcpy(g_product_key, p_product_key, product_key_len);
    }
    if (g_device_name == NULL) {
        g_device_name = malloc(device_name_len + 1);
        if (NULL == g_device_name) {
            return;
        }

        memset(g_device_name, 0, device_name_len + 1);
        memcpy(g_device_name, p_device_name, device_name_len);
    }

    printf("device productKey: %s\r\n", g_product_key);
    printf("device deviceName: %s\r\n", g_device_name);
}

/* MQTT事件回调函数, 当网络连接/重连/断开时被触发, 事件定义见core/aiot_mqtt_api.h */
void demo_mqtt_event_handler(void *handle, const aiot_mqtt_event_t *event, void *userdata)
{
    switch (event->type) {
        /* SDK因为用户调用了aiot_mqtt_connect()接口, 与mqtt服务器建立连接已成功 */
        case AIOT_MQTTEVT_CONNECT: {
            printf("AIOT_MQTTEVT_CONNECT\n");
        }
        break;

        /* SDK因为网络状况被动断连后, 自动发起重连已成功 */
        case AIOT_MQTTEVT_RECONNECT: {
            printf("AIOT_MQTTEVT_RECONNECT\n");
        }
        break;

        /* SDK因为网络的状况而被动断开了连接, network是底层读写失败, heartbeat是没有按预期得到服务端心跳应答 */
        case AIOT_MQTTEVT_DISCONNECT: {
            char *cause = (event->data.disconnect == AIOT_MQTTDISCONNEVT_NETWORK_DISCONNECT) ? ("network disconnect") :
                          ("heartbeat disconnect");
            printf("AIOT_MQTTEVT_DISCONNECT: %s\n", cause);
        }
        break;

        default: {

        }
    }
}

/* MQTT默认消息处理回调, 当SDK从服务器收到MQTT消息时, 且无对应用户回调处理时被调用 */
void demo_mqtt_default_recv_handler(void *handle, const aiot_mqtt_recv_t *packet, void *userdata)
{
    switch (packet->type) {
        case AIOT_MQTTRECV_HEARTBEAT_RESPONSE: {
            printf("heartbeat response\n");
        }
        break;

        case AIOT_MQTTRECV_SUB_ACK: {
            printf("suback, res: -0x%04X, packet id: %d, max qos: %d\n",
                   -packet->data.sub_ack.res, packet->data.sub_ack.packet_id, packet->data.sub_ack.max_qos);
        }
        break;

        case AIOT_MQTTRECV_PUB: {
            printf("pub, qos: %d, topic: %.*s\n", packet->data.pub.qos, packet->data.pub.topic_len, packet->data.pub.topic);
            printf("pub, payload: %.*s\n", packet->data.pub.payload_len, packet->data.pub.payload);
            /* TODO: 处理服务器下发的业务报文 */

            /* 处理云端下发的productKey和deviceName */
            demo_get_device_info(packet->data.pub.topic, packet->data.pub.topic_len,
                                 (char *)packet->data.pub.payload, packet->data.pub.payload_len);
        }
        break;

        case AIOT_MQTTRECV_PUB_ACK: {
            printf("puback, packet id: %d\n", packet->data.pub_ack.packet_id);
        }
        break;

        default: {

        }
    }
}

/* 执行aiot_mqtt_process的线程, 包含心跳发送和QoS1消息重发 */
void *demo_mqtt_process_thread(void *args)
{
    int32_t res = STATE_SUCCESS;

    while (g_mqtt_process_thread_running) {
        res = aiot_mqtt_process(args);
        if (res == STATE_USER_INPUT_EXEC_DISABLED) {
            break;
        }
        sleep(1);
    }
    return NULL;
}

/* 执行aiot_mqtt_recv的线程, 包含网络自动重连和从服务器收取MQTT消息 */
void *demo_mqtt_recv_thread(void *args)
{
    int32_t res = STATE_SUCCESS;

    while (g_mqtt_recv_thread_running) {
        res = aiot_mqtt_recv(args);
        if (res < STATE_SUCCESS) {
            if (res == STATE_USER_INPUT_EXEC_DISABLED) {
                break;
            }
            sleep(1);
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int32_t     res = STATE_SUCCESS;
    void       *mqtt_handle = NULL;

    /* TODO: 使用X509双向认证时, MQTT连接的服务器域名与通常情况不同 */
    char       *host = "x509.itls.cn-shanghai.aliyuncs.com";

    uint16_t    port = 1883; /* X.509服务器目的端口是1883 */
    aiot_sysdep_network_cred_t cred; /* 安全凭据结构体, 如果要用TLS, 这个结构体中配置CA证书等参数 */

    /* TODO: 使用X.509认证模式建连的设备, 三元组信息都需用空字符串进行配置 */
    char *product_key       = "";
    char *device_name       = "";
    char *device_secret     = "";

    /* 配置SDK的底层依赖 */
    aiot_sysdep_set_portfile(&g_aiot_sysdep_portfile);
    /* 配置SDK的日志输出 */
    aiot_state_set_logcb(demo_state_logcb);

    /* 创建SDK的安全凭据, 用于建立TLS连接 */
    memset(&cred, 0, sizeof(aiot_sysdep_network_cred_t));
    cred.option = AIOT_SYSDEP_NETWORK_CRED_SVRCERT_CA;  /* 使用RSA证书校验MQTT服务端 */
    cred.max_tls_fragment = 16384; /* 最大的分片长度为16K, 其它可选值还有4K, 2K, 1K, 0.5K */
    cred.sni_enabled = 1;                               /* TLS建连时, 支持Server Name Indicator */
    cred.x509_server_cert = ali_ca_cert;                 /* 用来验证MQTT服务端的RSA根证书 */
    cred.x509_server_cert_len = strlen(ali_ca_cert);     /* 用来验证MQTT服务端的RSA根证书长度 */

    /* TODO: 留意以下4行, 使用X509双向认证时, 用户对安全凭据的设置就只要增加这一部分 */
    cred.x509_client_cert = client_cert;
    cred.x509_client_cert_len = strlen(client_cert);
    cred.x509_client_privkey = client_private_key;
    cred.x509_client_privkey_len = strlen(client_private_key);

    /* 创建1个MQTT客户端实例并内部初始化默认参数 */
    mqtt_handle = aiot_mqtt_init();
    if (mqtt_handle == NULL) {
        printf("aiot_mqtt_init failed\n");
        return -1;
    }

    /* 配置MQTT服务器地址 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_HOST, (void *)host);
    /* 配置MQTT服务器端口 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PORT, (void *)&port);
    /* 配置设备productKey */
    /* TODO: 与不使用X509时不同的是, productKey 是从云平台推下来, 此处先用""配置给SDK */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PRODUCT_KEY, (void *)product_key);
    /* 配置设备deviceName */
    /* TODO: 与不使用X509时不同的是, deviceName 是从云平台推下来, 此处先用""配置给SDK */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_DEVICE_NAME, (void *)device_name);
    /* 配置设备deviceSecret */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_DEVICE_SECRET, (void *)device_secret);
    /* 配置网络连接的安全凭据, 上面已经创建好了 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_NETWORK_CRED, (void *)&cred);
    /* 配置MQTT默认消息接收回调函数 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_RECV_HANDLER, (void *)demo_mqtt_default_recv_handler);
    /* 配置MQTT事件回调函数 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_EVENT_HANDLER, (void *)demo_mqtt_event_handler);

    /* 与服务器建立MQTT连接 */
    res = aiot_mqtt_connect(mqtt_handle);
    if (res < STATE_SUCCESS) {
        /* 尝试建立连接失败, 销毁MQTT实例, 回收资源 */
        aiot_mqtt_deinit(&mqtt_handle);
        printf("aiot_mqtt_connect failed: -0x%04X\n", -res);
        return -1;
    }

    /* MQTT 订阅topic功能示例, 请根据自己的业务需求进行使用 */
    /* {
        char *sub_topic = "/sys/{YourProductKey}/{YourDeviceName}/thing/event/+/post_reply";

        res = aiot_mqtt_sub(mqtt_handle, sub_topic, NULL, 1, NULL);
        if (res < 0) {
            printf("aiot_mqtt_sub failed, res: -0x%04X\n", -res);
            return -1;
        }
    } */

    /* MQTT 发布消息功能示例, 请根据自己的业务需求进行使用 */
    /* {
        char *pub_topic = "/sys/{YourProductKey}/{YourDeviceName}/thing/event/property/post";
        char *pub_payload = "{\"id\":\"1\",\"version\":\"1.0\",\"params\":{\"LightSwitch\":0}}";

        res = aiot_mqtt_pub(mqtt_handle, pub_topic, (uint8_t *)pub_payload, (uint32_t)strlen(pub_payload), 0);
        if (res < 0) {
            printf("aiot_mqtt_sub failed, res: -0x%04X\n", -res);
            return -1;
        }
    } */

    /* 创建一个单独的线程, 专用于执行aiot_mqtt_process, 它会自动发送心跳保活, 以及重发QoS1的未应答报文 */
    g_mqtt_process_thread_running = 1;
    res = pthread_create(&g_mqtt_process_thread, NULL, demo_mqtt_process_thread, mqtt_handle);
    if (res < 0) {
        printf("pthread_create demo_mqtt_process_thread failed: %d\n", res);
        return -1;
    }

    /* 创建一个单独的线程用于执行aiot_mqtt_recv, 它会循环收取服务器下发的MQTT消息, 并在断线时自动重连 */
    g_mqtt_recv_thread_running = 1;
    res = pthread_create(&g_mqtt_recv_thread, NULL, demo_mqtt_recv_thread, mqtt_handle);
    if (res < 0) {
        printf("pthread_create demo_mqtt_recv_thread failed: %d\n", res);
        return -1;
    }

    /* 主循环进入休眠 */
    while (1) {
        sleep(1);
    }

    /* 断开MQTT连接, 一般不会运行到这里 */
    res = aiot_mqtt_disconnect(mqtt_handle);
    if (res < STATE_SUCCESS) {
        aiot_mqtt_deinit(&mqtt_handle);
        printf("aiot_mqtt_disconnect failed: -0x%04X\n", -res);
        return -1;
    }

    /* 销毁MQTT实例, 一般不会运行到这里 */
    res = aiot_mqtt_deinit(&mqtt_handle);
    if (res < STATE_SUCCESS) {
        printf("aiot_mqtt_deinit failed: -0x%04X\n", -res);
        return -1;
    }

    g_mqtt_process_thread_running = 0;
    g_mqtt_recv_thread_running = 0;
    pthread_join(g_mqtt_process_thread, NULL);
    pthread_join(g_mqtt_recv_thread, NULL);

    return 0;
}

