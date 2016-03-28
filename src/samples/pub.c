/*
 public.c 用于发送消息. 测试时新开一个项目,用receive.c同步接收消息或async_sub.c异步接收消息
 sub.c 用于同步接收消息
 async_sub.c 用于异步接收消息(注意异步接收消息时，只能引用MQTTAsync，不能引用MQTTClient)
 */

#include "stdio.h"
#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include "dotlink.h"
#include "MQTTClient.h"

#define TOPIC       "subscribe_topic"
#define PAYLOAD     "{\"code\": \"1001\",\"access_key\": \"access_key1001\",\"param1\": \"param1001\"}"
#define QOS         1
#define TIMEOUT     10000L

PRO_info my_PRO_info;
volatile int isContinue=1;

int main(int argc, char* argv[])
{
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;
    
    char *pid = "b1e57467c92140e299022deb808cdd24"; //产品唯一标识
    char *productSecret = "2c15e161b5064d32ba6a6f664fbcde15"; //密钥
    char *did = "111111"; //设备ID
    
    //1.发送HTTP请求，获取用于创建MQTT连接使用的clientId, username, password, host.
    int res = MQTTClient_setup_with_pid_and_did(pid, did, productSecret, &my_PRO_info);
    if (res < 0) {
        printf("can't get product info\n");
        return 0;
    }
    printf("Get product info: client_id:%s,username:%s,password:%s, host:%s\n", my_PRO_info.client_id, my_PRO_info.username, my_PRO_info.password, my_PRO_info.host);
    
    //2.创建MQTT连接：测试时发送端和接收端不能是同一clientId，两边如果相同，会判断是同一设备而收不到消息
    MQTTClient_create(&client, my_PRO_info.host, my_PRO_info.client_id,
                      MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.reliable = 1;
    conn_opts.cleansession = 1;
    conn_opts.password=my_PRO_info.password;
    conn_opts.username=my_PRO_info.username;
    
    //3.连接MQTT：返回值为0时，连接成功
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
//    rc = MQTTClient_subscribe(client, TOPIC, 1);
    
    int order_id=525121535;
    int msgid=1;
    while (isContinue)
    {
        int code=1;
        char payload[2048];
        long payloadLen;
        int did2=2554612;
        char streams[1024];
        char stream[200]="{\"id\": \"switch\",\"value\": \"s\"}";  //开关指令
//        char stream[200]="{\"id\": \"p\",\"value\": \"22:00\"}";  //设置定时
//        char stream[200]="{\"id\": \"dis\",\"value\": \"disconnect\"}";  //断开连接指令
        
        sprintf(streams, "[");
        strcat(streams, stream);
        strcat(streams, "]");
        
        sprintf(payload,"{\"code\":%d,\"order_id\":%d,\"data_points\":%s,\"did\":%d}",code,order_id,streams,did2);
        payloadLen =strlen(payload);
        
        pubmsg.payload = payload; //publish消息内容
        pubmsg.payloadlen = payloadLen;
        pubmsg.qos = QOS; //三种消息发布服务质量. 0：“至多一次”，消息发布完全依赖底层 TCP/IP 网络。会发生消息丢失或重复。这一级别可用于如下情况，环境传感器数据，丢失一次读记录无所谓，因为不久后还会有第二次发送。1：“至少一次”，确保消息到达，但消息重复可能会发生。2：“只有一次”，确保消息到达一次。这一级别可用于如下情况，在计费系统中，消息重复或丢失会导致不正确的结果。
        pubmsg.msgid=msgid;
        
        //4.发送消息，payload即为需要发送的消息内容，可以拼成JSON字符串，接收端收到后对它进行解析处理
        rc=MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
        if(rc==0){
            printf("\n\n");
            printf("publish-success:");
            for (int i=0; i<payloadLen; i++) {
                printf("%c",payload[i]);
            }
        }
        rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
        order_id++;
        msgid++;
    }
    
    
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}

