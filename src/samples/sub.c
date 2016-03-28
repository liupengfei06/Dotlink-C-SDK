//#include "stdio.h"
//#include "stdlib.h"
//#include "string.h"
//#include "MQTTClient.h"
//#include "cJSON.h"
//#include "dotlink.h"
//
//#define TOPIC       "subscribe_topic"
//#define QOS         1
//#define TIMEOUT     10000L
//
//PRO_info my_PRO_info;
//payload_info my_payload_info;
//data_point_info my_data_point_info;
//volatile int isContinue=1;
//
//static int get_Payload_info(const char *json_data) {
//    int ret = 0;
//    char buf[2048];
//    memset(buf, 0, sizeof(buf));
//    memcpy(buf, json_data, strlen(json_data));
//    
//    cJSON *data = cJSON_Parse(buf);
//    if (data) {
//        int ret_size = cJSON_GetArraySize(data);
//        if (ret_size >= 4) {
//            my_payload_info.code = cJSON_GetObjectItem(data,"code")->valueint;
//            my_payload_info.order_id = cJSON_GetObjectItem(data,"order_id")->valueint;
//            my_payload_info.did = cJSON_GetObjectItem(data,"did")->valueint;
//            cJSON *data_points=cJSON_GetObjectItem(data, "data_points");
//            if(data_points){
//                //解析data_points
//                cJSON *data_point=cJSON_GetArrayItem(data_points, 0);
//                if(cJSON_GetArraySize(data_point)>=2){
//                    strcpy(my_data_point_info.key , cJSON_GetObjectItem(data_point, "id") ->valuestring);
//                    strcpy(my_data_point_info.value , cJSON_GetObjectItem(data_point, "value") ->valuestring);
//                }
//                else{
//                    return -1;
//                }
//            }
//        } else{
//            ret = -1;
//        }
//        cJSON_Delete(data);
//    }
//    return ret;
//}
//static int get_data_point_info(const char *json_data) {
//    int ret = 0;
//    char buf[2048];
//    memset(buf, 0, sizeof(buf));
//    memcpy(buf, json_data, strlen(json_data));
//    
//    cJSON *root = cJSON_Parse(buf);
//    if (root) {
//        int ret_size = cJSON_GetArraySize(root);
//        if (ret_size >= 2) {
//            strcpy(my_data_point_info.key, cJSON_GetObjectItem(root,"id")->valuestring);
//            strcpy(my_data_point_info.value, cJSON_GetObjectItem(root,"value")->valuestring);
//        } else
//            ret = -1;
//        
//        cJSON_Delete(root);
//    }
//    return ret;
//}
//
//int main(int argc, char* argv[])
//{
//    MQTTClient client;
//    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
//    int rc;
//    
//    
//    char *pid = "b1e57467c92140e299022deb808cdd24"; //产品唯一标识
//    char *productSecret = "2c15e161b5064d32ba6a6f664fbcde15"; //密钥
//    char *did = "222222"; //设备ID
//    
//    //1.发送HTTP请求，获取clientId, username, password, host.
//    int res = MQTTClient_setup_with_pid_and_did(pid, did, productSecret, &my_PRO_info);
//    if (res < 0) {
//        printf("can't get product info\n");
//        return 0;
//    }
//    printf("Get product info: client_id:%s,username:%s,password:%s, host:%s\n", my_PRO_info.client_id, my_PRO_info.username, my_PRO_info.password, my_PRO_info.host);
//    
//    //2.创建MQTT连接：测试时发送端和接收端不能是同一clientId，两边如果相同，会判断是同一设备而收不到消息
//    MQTTClient_create(&client, my_PRO_info.host, my_PRO_info.client_id,
//                      MQTTCLIENT_PERSISTENCE_NONE, NULL);
//    conn_opts.keepAliveInterval = 20;
//    conn_opts.reliable = 0;
//    conn_opts.cleansession = 1;
//    conn_opts.password=my_PRO_info.password;
//    conn_opts.username=my_PRO_info.username;
//    
//    //3.连接MQTT：返回值为0时，连接成功
//    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
//    {
//        printf("Failed to connect, return code %d\n", rc);
//        exit(-1);
//    }
//    //4.订阅消息,topic必须和发送消息使用的一致
//    rc = MQTTClient_subscribe(client, TOPIC, 1);
//    if (rc < 0) {
//        printf("Failed to subscribe, return code %d\n", rc);
//        return 0;
//    }
//    while (isContinue)
//    {
//        char* topicName = TOPIC;
//        int topicLen;
//        MQTTClient_message* message = NULL;
//        //5.同步接收消息（当客户端应用程序不需要支持异步回调方法时使用）
//        rc = MQTTClient_receive(client, &topicName, &topicLen, &message, 10000);
//        if (message)
//        {
//            if((char*)message->payload){
//                //这里可以对接收到的数据进行解析
//                printf("\n\nmessage->payload:%.*s%s", message->payloadlen, (char*)message->payload ,"");
//                char subMSG[2048];
//                memcpy(subMSG, message->payload, message->payloadlen);
//                if (get_Payload_info(subMSG)==0){
//                    if(my_payload_info.code==1){
//                        
//                        //开始-停止烧水指令
//                        if(my_data_point_info.key[0]=='s' && strlen(my_data_point_info.key)==6){
//                            if(my_data_point_info.value[0]=='p'){
//                                printf("\n\n停止烧水指令");
//                            }
//                            if(my_data_point_info.value[0]=='s'){
//                                printf("\n\n开始烧水指令");
//                            }
//                        }
//                        
//                        //设置停止烧水时间
//                        if(my_data_point_info.key[0]=='p' && strlen(my_data_point_info.key)==1){
//                            printf("\n\n设置停止烧水时间:");
//                            for(int i=0 ; i<strlen(my_data_point_info.value); i++){
//                                printf("%c",my_data_point_info.value[i]);
//                            }
//                        }
//                        //设置开始烧水时间
//                        if(my_data_point_info.key[0]=='s' && strlen(my_data_point_info.key)==1){
//                            printf("\n\n设置开始烧水时间:");
//                            for(int i=0 ; i<strlen(my_data_point_info.value); i++){
//                                printf("%c",my_data_point_info.value[i]);
//                            }
//                        }
//                        if(my_data_point_info.key[0]=='d' && strlen(my_data_point_info.key)==3){
//                            printf("\n\n\n\n断开连接指令");
//                            isContinue=0;
//                        }
//                        
//                    }
//                }
//            }
//            fflush(stdout);
//            MQTTClient_freeMessage(&message);
//            MQTTClient_free(topicName);
//        }
//    }
//    
//    MQTTClient_disconnect(client, 10000);
//    MQTTClient_destroy(&client);
//    return rc;
//}
