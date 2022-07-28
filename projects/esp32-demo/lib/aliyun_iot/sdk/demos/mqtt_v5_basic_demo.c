/*
 * 这个例程演示MQTT 5.0协议连接阿里云物联网平台.
 * 适用于`Linux`这类支持pthread的POSIX设备, 它演示了用SDK配置MQTT参数并建立连接, 之后创建2个线程
 *
 * + 一个线程用于保活长连接
 * + 一个线程用于接收消息, 并在有消息到达时进入默认的数据回调, 在连接状态变化时进入事件回调
 *
 * 需要用户关注或修改的部分, 已经用 TODO 在注释中标明
 *
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_mqtt_api.h"

/*
    TODO: 替换为自己实例的接入点.

    Mqtt 5.0目前仅支持企业实例, 或者2021年07月30日之后（含当日）开通的物联网平台服务下公共实例
    url的格式为"${YourInstanceId}.mqtt.iothub.aliyuncs.com"
    其中${YourInstanceId}: 请替换为您企业/公共实例的Id

    详情请见: https://help.aliyun.com/document_detail/147356.html
*/
char *url = "{YourInstanceId}.mqtt.iothub.aliyuncs.com";

/* TODO: 替换为自己设备的三元组 */
char *product_key       = "{YourProductKey}";
char *device_name       = "{YourDeviceName}";
char *device_secret     = "{YourDeviceSecret}";

/* 位于portfiles/aiot_port文件夹下的系统适配函数集合 */
extern aiot_sysdep_portfile_t g_aiot_sysdep_portfile;

/* 位于external/ali_ca_cert.c中的服务器证书 */
extern const char *ali_ca_cert;

static pthread_t g_mqtt_process_thread;
static pthread_t g_mqtt_recv_thread;
static uint8_t g_mqtt_process_thread_running = 0;
static uint8_t g_mqtt_recv_thread_running = 0;

/* TODO: 如果要关闭日志, 就把这个函数实现为空, 如果要减少日志, 可根据code选择不打印
 *
 * 例如: [1577589489.033][LK-0317] device0001&a1aILF2zWt9
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

/* MQTT事件回调函数, 当网络连接/重连/断开时被触发, 事件定义见core/aiot_mqtt_api.h */
void demo_mqtt_event_handler(void *handle, const aiot_mqtt_event_t *event, void *userdata)
{
    switch (event->type) {
        /* SDK因为用户调用了aiot_mqtt_connect()接口, 与mqtt服务器建立连接已成功 */
        case AIOT_MQTTEVT_CONNECT: {
            printf("AIOT_MQTTEVT_CONNECT\n");
            /* TODO: 处理SDK建连成功, 不可以在这里调用耗时较长的阻塞函数 */
        }
        break;

        /* SDK因为网络状况被动断连后, 自动发起重连已成功 */
        case AIOT_MQTTEVT_RECONNECT: {
            printf("AIOT_MQTTEVT_RECONNECT\n");
            /* TODO: 处理SDK重连成功, 不可以在这里调用耗时较长的阻塞函数 */
        }
        break;

        /* SDK因为网络的状况而被动断开了连接, network是底层读写失败, heartbeat是没有按预期得到服务端心跳应答 */
        case AIOT_MQTTEVT_DISCONNECT: {
            char *cause = (event->data.disconnect == AIOT_MQTTDISCONNEVT_NETWORK_DISCONNECT) ? ("network disconnect") :
                          ("heartbeat disconnect");
            printf("AIOT_MQTTEVT_DISCONNECT: %s\n", cause);
            /* TODO: 处理SDK被动断连, 不可以在这里调用耗时较长的阻塞函数 */
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
            /* TODO: 处理服务器对心跳的回应, 一般不处理 */
        }
        break;

        case AIOT_MQTTRECV_SUB_ACK: {
            printf("suback, res: -0x%04X, packet id: %d, max qos: %d\n",
                   -packet->data.sub_ack.res, packet->data.sub_ack.packet_id, packet->data.sub_ack.max_qos);
            /* TODO: 处理服务器对订阅请求的回应, 一般不处理 */
        }
        break;
        case AIOT_MQTTRECV_UNSUB_ACK: {
            printf("unsuback, , packet id: %d\n",
                   packet->data.unsub_ack.packet_id);
            /* TODO: 处理服务器对订阅请求的回应, 一般不处理 */
        }
        break;
        case AIOT_MQTTRECV_PUB: {
            int idx = 0;
            printf("pub, qos: %d, topic: %.*s\n", packet->data.pub.qos, packet->data.pub.topic_len, packet->data.pub.topic);
            printf("pub, payload: %.*s\n", packet->data.pub.payload_len, packet->data.pub.payload);
            printf("pub, payload len: %x\n", packet->data.pub.payload_len);

            /* TODO: 处理服务器下发的MQTT 5.0的属性和业务报文 */
            if (NULL != packet->data.pub.pub_prop) {
                printf("pub, topic alias is %d\n", packet->data.pub.pub_prop->topic_alias);

                if (packet->data.pub.pub_prop->subscription_identifier > 0) {
                    printf("pub, subscription identifer: 0x%x\n", packet->data.pub.pub_prop->subscription_identifier);
                }

                if (packet->data.pub.pub_prop->response_topic.len > 0) {
                    printf("response_topic is %.*s\n", packet->data.pub.pub_prop->response_topic.len,
                           packet->data.pub.pub_prop->response_topic.value);
                }
                if (packet->data.pub.pub_prop->correlation_data.len > 0) {
                    printf("correlation_data is %.*s\n", packet->data.pub.pub_prop->correlation_data.len,
                           packet->data.pub.pub_prop->correlation_data.value);
                }
                for (idx = 0; idx < USER_PROPERTY_MAX; idx++) {
                    user_property_t *user_prop = packet->data.pub.pub_prop->user_property[idx];
                    if (NULL != user_prop) {
                        printf("pub, user property key: %.*s\n", user_prop->key.len, user_prop->key.value);
                        printf("pub, user property value: %.*s\n", user_prop->value.len, user_prop->value.value);
                    }
                }
            }
            printf("\r\n");
        }
        break;

        case AIOT_MQTTRECV_PUB_ACK: {
            printf("puback, packet id: %d\n", packet->data.pub_ack.packet_id);
            /* TODO: 处理服务器对QoS1上报消息的回应, 一般不处理 */
        }
        break;
        case AIOT_MQTTRECV_CON_ACK: {
            int idx = 0;
            printf("\r\n");
            /* TODO: 处理服务器下发的MQTT 5.0的connack的属性 */
            printf("conack, topic alias max in server side: %d\n", packet->data.con_ack.prop.topic_alias_max);
            printf("conack, max qos: %d\n", packet->data.con_ack.prop.max_qos);
            printf("conack, assigned cid: %s\n", (char *)(packet->data.con_ack.prop.assigned_clientid));
            printf("conack, reason code: 0x%02x\n", packet->data.con_ack.reason_code);
            printf("conack, max_packet_size: 0x%x\n", packet->data.con_ack.prop.max_packet_size);
            printf("conack, shared_subscription_available: 0x%x\n", packet->data.con_ack.prop.shared_subscription_available);
            printf("conack, wildcard_subscription_available: 0x%x\n", packet->data.con_ack.prop.wildcard_subscription_available);
            printf("conack, subscription_identifier_available: 0x%x\n",
                   packet->data.con_ack.prop.subscription_identifier_available);
            printf("conack, server_receive_max: 0x%02x\n", packet->data.con_ack.prop.server_receive_max);

            for (idx = 0; idx < USER_PROPERTY_MAX; idx++) {
                user_property_t *user_prop = packet->data.con_ack.prop.user_property[idx];
                if (NULL != user_prop) {
                    printf("connack, user property key: %.*s\n", user_prop->key.len, user_prop->key.value);
                    printf("connack, user property value: %.*s\n", user_prop->value.len, user_prop->value.value);
                }
            }
        }
        printf("\r\n");
        break;
        case AIOT_MQTTRECV_DISCONNECT: {
            printf("server disconnect, reason code: 0x%x\n", packet->data.server_disconnect.reason_code);
        }
        break;
        default: {

        }
    }
}

/* 执行aiot_mqtt_process的线程, 包含心跳发送和QoS1消息重发 */
void *demo_mqtt_process_thread(void *args)
{
    while (g_mqtt_process_thread_running) {
        int32_t res = STATE_SUCCESS;
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

    uint16_t    port = 443;      /* 无论设备是否使用TLS连接阿里云平台, 目的端口都是443 */
    aiot_sysdep_network_cred_t cred; /* 安全凭据结构体, 如果要用TLS, 这个结构体中配置CA证书等参数 */
    uint8_t protocol_version = 0;

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

    /* 创建1个MQTT客户端实例并内部初始化默认参数 */
    mqtt_handle = aiot_mqtt_init();
    if (mqtt_handle == NULL) {
        printf("aiot_mqtt_init failed\n");
        return -1;
    }

    /* TODO: 如果以下代码不被注释, 则例程会用TCP而不是TLS连接云平台 */
    /*
    {
        memset(&cred, 0, sizeof(aiot_sysdep_network_cred_t));
        cred.option = AIOT_SYSDEP_NETWORK_CRED_NONE;
    }
    */

    /* 配置MQTT协议的版本 */
    protocol_version = AIOT_MQTT_VERSION_5_0;
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_VERSION, (void *)&protocol_version);
    /* 配置MQTT服务器地址 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_HOST, (void *)url);
    /* 配置MQTT服务器端口 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PORT, (void *)&port);
    /* 配置设备productKey */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PRODUCT_KEY, (void *)product_key);
    /* 配置设备deviceName */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_DEVICE_NAME, (void *)device_name);
    /* 配置设备deviceSecret */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_DEVICE_SECRET, (void *)device_secret);
    /* 配置网络连接的安全凭据, 上面已经创建好了 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_NETWORK_CRED, (void *)&cred);
    /* 配置MQTT默认消息接收回调函数 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_RECV_HANDLER, (void *)demo_mqtt_default_recv_handler);
    /* 配置MQTT事件回调函数 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_EVENT_HANDLER, (void *)demo_mqtt_event_handler);

    /* 如果要使用MQTT 5.0的assigned clientId功能, 需要将use_assigned_clientid置为1 */
    uint8_t use_assigned_clientid = 0;
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_ASSIGNED_CLIENTID, (void *)(&use_assigned_clientid));

    /* MQTT 5.0特性中的assigned clientId. 如果要使用, 需要将use_assigned_clientid置为1 */
    conn_property_t conn_prop = {0};
    uint16_t topic_alias_max = 0;
    conn_prop.topic_alias_max = topic_alias_max;

    /* MQTT 5.0特性中的用户属性. 用户需要定义属性的key */
    char *key_str = "demo_key";
    len_value_t key = {
        .len = strlen(key_str),
        .value = (uint8_t *)key_str
    };

    /* MQTT 5.0特性中的用户属性. 用户需要定义属性的value */
    char *value_str =
                "demo_value";
    len_value_t value = {
        .len = strlen(value_str),
        .value = (uint8_t *)value_str
    };

    /* MQTT 5.0特性中的用户属性. 用户将上述的key/value组装成一个完整的用户属性 */
    user_property_t user_property = {
        .key = key,
        .value = value,
    };

    /* MQTT 5.0特性中的用户属性. 在建连属性中增加用户属性. 最多可以增加USER_PROPERTY_MAX(当前是20)个用户属性 */
    conn_prop.user_property[0] = &user_property;
    conn_prop.user_property[1] = &user_property;

    /* 通过MQTT 5.0的方式与服务器建连 */
    res = aiot_mqtt_connect_with_prop(mqtt_handle, &conn_prop);

    if (res < STATE_SUCCESS) {
        /* 尝试建立连接失败, 销毁MQTT实例, 回收资源 */
        aiot_mqtt_deinit(&mqtt_handle);
        printf("aiot_mqtt_connect failed: -0x%04X\n\r\n", -res);
        printf("please check variables like mqtt_host, produt_key, device_name, device_secret in demo\r\n");
        return -1;
    }

    /* MQTT 订阅topic功能示例, 请根据自己的业务需求进行使用 */
#if 0
    {
        char *sub_topic = "/sys/{YourProductKey}/{YourDeviceName}/user/update";
        sub_property_t sub_property = {0};
        sub_property.user_property[0] = &user_property;

        res = aiot_mqtt_sub_with_prop(mqtt_handle, sub_topic, NULL, 0, NULL, &sub_property);
        if (res < 0) {
            printf("aiot_mqtt_sub failed, res: -0x%04X\n", -res);
            aiot_mqtt_deinit(&mqtt_handle);
            return -1;
        }
    }
#endif

    /* MQTT 发布消息功能示例, 请根据自己的业务需求进行使用 */
    {
        char *pub_topic = "/sys/{YourProductKey}/{YourDeviceName}/thing/event/property/post";
        char *pub_payload = "{\"id\":\"1\",\"version\":\"1.0\",\"params\":{\"LightSwitch\":0}}";
        pub_property_t pub_prop = {0};

        /* MQTT 5.0特性中的用户属性. 在pub报文中设置用户属性 */
        pub_prop.user_property[0] = &user_property;
        len_value_t response_topic = {
            .len = strlen(pub_topic),
            .value = (uint8_t *)pub_topic
        };

        /* MQTT 5.0特性中的response topic属性. 在pub报文中设置response topic */
        pub_prop.response_topic = response_topic;

        /* MQTT 5.0特性中的correlation data属性. 在pub报文中设置correlation data */
        char *demo_data_str = "12345";
        len_value_t correlation_data = {
            .len = strlen(demo_data_str),
            .value = (uint8_t *)demo_data_str
        };
        pub_prop.correlation_data = correlation_data;

        res = aiot_mqtt_pub_with_prop(mqtt_handle, pub_topic, (uint8_t *)pub_payload, (uint32_t)(strlen(pub_payload)), 0,
                                      &pub_prop);
        if (res < 0) {
            printf("aiot_mqtt pub failed, res: -0x%04X\n", -res);
            aiot_mqtt_deinit(&mqtt_handle);
            return -1;
        }

        /* MQTT 5.0特性中的topic alias属性. 在一次连接中, 往pub_topic这个topic中发送过报文,
         * 之后再往这个topic发送的话, 会使能topic alias功能, 上行报文不带有topic字段.
        */
        res = aiot_mqtt_pub_with_prop(mqtt_handle, pub_topic, (uint8_t *)pub_payload, (uint32_t)strlen(pub_payload), 0,
                                      &pub_prop);
        if (res < 0) {
            printf("aiot_mqtt_pub failed, res: -0x%04X\n", -res);
            aiot_mqtt_deinit(&mqtt_handle);
            return -1;
        }
        printf("uplink without topic done\n");

    }

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

    while (1) {
        sleep(1);
    }

    /* 断开MQTT连接, 一般不会运行到这里 */
    g_mqtt_process_thread_running = 0;
    g_mqtt_recv_thread_running = 0;
    sleep(1);
    pthread_join(g_mqtt_process_thread, NULL);
    pthread_join(g_mqtt_recv_thread, NULL);

    {
        /* reason code 0x0 表示Normal disconnection */
        int demo_reason_code = 0x0;
        char *demo_reason_string = "normal_exit";
        disconn_property_t discon_prop = {0};
        len_value_t discon_reason = {
            .len = strlen(demo_reason_string),
            .value = (uint8_t *)demo_reason_string
        };
        discon_prop.reason_string = &discon_reason;
        discon_prop.user_property[0] = &user_property;
        res = aiot_mqtt_disconnect_with_prop(mqtt_handle, demo_reason_code,  &discon_prop);
    }

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


    return 0;
}
