//#include "stdio.h"
//#include "stdlib.h"
//#include "string.h"
//#include "MQTTAsync.h"
//#include "dotlink.h"
//
//#define ADDRESS     "tcp://120.26.77.175:1884"
//#define CLIENTID    "ExampleClientSub"
//#define TOPIC       "subscribe_topic"
//#define PAYLOAD     "Hello World!"
//#define QOS         1
//#define TIMEOUT     10000L
//
//
//PRO_info my_PRO_info;
//volatile int isContinue=1;
//volatile MQTTAsync_token deliveredtoken;
//int disc_finished = 0;
//int subscribed = 0;
//int finished = 0;
//payload_info my_payload_info;
//data_point_info my_data_point_info;
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
//void delivered(void *context, MQTTAsync_token dt)
//{
//    printf("Message with token value %d delivery confirmed\n", dt);
//}
//void connlost(void *context, char *cause)
//{
//    MQTTAsync client = (MQTTAsync)context;
//    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
//    int rc;
//    
//    printf("\nConnection lost\n");
//    printf("     cause: %s\n", cause);
//    
//    printf("Reconnecting\n");
//    conn_opts.keepAliveInterval = 20;
//    conn_opts.cleansession = 1;
//    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
//    {
//        printf("Failed to start connect, return code %d\n", rc);
//        finished = 1;
//    }
//}
//
//
//int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
//{
//    int i;
//    char* payloadptr;
//    
//    //这里可以对接收到的数据进行解析
//    printf("\n\nmessage->payload:%.*s%s", message->payloadlen, (char*)message->payload ,"");
//    char subMSG[2048];
//    memcpy(subMSG, message->payload, message->payloadlen);
//    if (get_Payload_info(subMSG)==0){
//        if(my_payload_info.code==1){
//            
//            //开始-停止烧水指令
//            if(my_data_point_info.key[0]=='s' && strlen(my_data_point_info.key)==6){
//                if(my_data_point_info.value[0]=='p'){
//                    printf("\n\n停止烧水指令");
//                }
//                if(my_data_point_info.value[0]=='s'){
//                    printf("\n\n开始烧水指令");
//                }
//            }
//            
//            //设置停止烧水时间
//            if(my_data_point_info.key[0]=='p' && strlen(my_data_point_info.key)==1){
//                printf("\n\n设置停止烧水时间:");
//                for(int i=0 ; i<strlen(my_data_point_info.value); i++){
//                    printf("%c",my_data_point_info.value[i]);
//                }
//            }
//            //设置开始烧水时间
//            if(my_data_point_info.key[0]=='s' && strlen(my_data_point_info.key)==1){
//                printf("\n\n设置开始烧水时间:");
//                for(int i=0 ; i<strlen(my_data_point_info.value); i++){
//                    printf("%c",my_data_point_info.value[i]);
//                }
//            }
//            if(my_data_point_info.key[0]=='d' && strlen(my_data_point_info.key)==3){
//                printf("\n\n\n\n断开连接指令");
//                isContinue=0;
//            }
//            
//        }
//    }
//    MQTTAsync_freeMessage(&message);
//    MQTTAsync_free(topicName);
//    return 1;
//}
//
//
//void onDisconnect(void* context, MQTTAsync_successData* response)
//{
//    printf("Successful disconnection\n");
//    disc_finished = 1;
//}
//
//
//void onSubscribe(void* context, MQTTAsync_successData* response)
//{
//    printf("Subscribe succeeded\n");
//    subscribed = 1;
//}
//
//void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
//{
//    printf("Subscribe failed, rc %d\n", response ? response->code : 0);
//    finished = 1;
//}
//
//
//void onConnectFailure(void* context, MQTTAsync_failureData* response)
//{
//    printf("Connect failed, rc %d\n", response ? response->code : 0);
//    finished = 1;
//}
//
//
//void onConnect(void* context, MQTTAsync_successData* response)
//{
//    MQTTAsync client = (MQTTAsync)context;
//    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
//    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
//    int rc;
//    
//    printf("Successful connection\n");
//    
//    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
//           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
//    opts.onSuccess = onSubscribe;
//    opts.onFailure = onSubscribeFailure;
//    opts.context = client;
//    
//    deliveredtoken = 0;
//    
//    if ((rc = MQTTAsync_subscribe(client, TOPIC, QOS, &opts)) != MQTTASYNC_SUCCESS)
//    {
//        printf("Failed to start subscribe, return code %d\n", rc);
//        exit(-1);
//    }
//}
//
//
//int main(int argc, char* argv[])
//{
//    MQTTAsync client;
//    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
//    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
//    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
//    MQTTAsync_token token;
//    int rc;
//    int ch;
//    
//    MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
//    
//    MQTTAsync_setCallbacks(client, NULL, connlost, msgarrvd, NULL);
//    
//    conn_opts.keepAliveInterval = 20;
//    conn_opts.cleansession = 1;
//    conn_opts.onSuccess = onConnect;
//    conn_opts.onFailure = onConnectFailure;
//    conn_opts.context = client;
//    conn_opts.username="EmJ4S6fQnt";
//    conn_opts.password="hjACCWFbT7";
//    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
//    {
//        printf("Failed to start connect, return code %d\n", rc);
//        exit(-1);
//    }
//    
//    while   (!subscribed)
//#if defined(WIN32) || defined(WIN64)
//        Sleep(100);
//#else
//    usleep(10000L);
//#endif
//    
//    if (finished)
//        goto exit;
//    
//    do
//    {
//        ch = getchar();
//    } while (ch!='Q' && ch != 'q');
//    
//    disc_opts.onSuccess = onDisconnect;
//    if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS)
//    {
//        printf("Failed to start disconnect, return code %d\n", rc);
//        exit(-1);
//    }
//    while   (!disc_finished)
//#if defined(WIN32) || defined(WIN64)
//        Sleep(100);
//#else
//    usleep(10000L);
//#endif
//    
//exit:
//    MQTTAsync_destroy(&client);
//    return rc;
//}