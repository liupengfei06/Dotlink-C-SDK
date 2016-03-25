
#C/嵌入式 SDK

调用MQTT接口有同步和异步两种，异步调用接口时，只能引用MQTTAsync，不能引用MQTTClient,且要把MQTTClient.h和MQTTClient.m从项目中移除，同步调用接口时同理。请根据实际需求选用同步或者异步，调接口需引入dotlink.h和MQTTClient.h(或MQTTAsync.h)头文件。下面是调用MQTT接口完成消息的发布/订阅接口[[DEMO下载地址]()]：

>## 初始化
>### 获取MQTT连接所需信息

pid和productSecret为应用注册获得，did为设备id。获取成功后返回值为0，MQTT连接所需的serverURI、clientId、username、password（username和password是DotlinkCloud后台生成的）会在info中返回。
```
int res = MQTTClient_setup_with_pid_and_did(pid, did, productSecret, &my_PRO_info);
if (res < 0) {
	printf("can't get product info\n");
	return 0;
}
```

> ## MQTT连接-同步
> ### 创建MQTT连接(同步)

创建MQTT连接MQTTClient_create需要硬件唯一标识clientId和MQTT服务端连接地址serverURI，发送端和接收端会对clientId做判断，clientId相同，会判断是同一设备而收不到消息. 创建成功返回0.
```
MQTTClient_create(&client, my_PRO_info.host, my_PRO_info.client_id,
                      1, NULL);
```

> ###  连接MQTT

username和password为MQTT服务端做连接验证使用，连接成功返回0；
```
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
conn_opts.keepAliveInterval = 20;
conn_opts.reliable = 1;
conn_opts.cleansession = 1;
conn_opts.password=my_PRO_info.password;
conn_opts.username=my_PRO_info.username;
	
if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
{
	printf("Failed to connect, return code %d\n", rc);
        exit(-1);
}
```

> ### 发送消息

topicName 是消息订阅的标识，MQTT是基于订阅/发布的消息，那么这个就是消息订阅的标识，像新闻客户端里的订阅不同的栏目一样，用于区别消息的类别。message内的payload参数为发送的消息主体，需要自定义内容；qos参数控制消息发布服务质量，0表示“至多一次”，1表示“至少一次”，2表示“只有一次”。deliveryToken是消息传递的令牌，可做消息验证用，没有可以传空。
```
MQTTClient_message pubmsg = MQTTClient_message_initializer;
pubmsg.payload = msgPayload;
pubmsg.payloadlen = strlen(msgPayload);
pubmsg.qos = 1;
pubmsg.retained = 0;

MQTTClient_publishMessage(client, topicName, &pubmsg, &deliveryToken);
```

> ### 订阅消息

注意topicName（消息订阅的标识）需包含在发布消息的topicName内。
```
rc = MQTTClient_subscribe(client, topicName, 1);
```

> ### 接收消息

当有新消息到达，或者设置的timeout超时会执行此接口。timeout为等待消息时间长度，单位为毫秒。
```
int topicLen;
MQTTClient_message* message = NULL;
long timeout=10000;

rc = MQTTClient_receive(client, &topicName, &topicLen, &message, timeout);
	if (message)
	{
		if((char*)message->payload){
		//这里可以对接收到的数据进行解析,根据消息内容执行相应操作
		printf("-----------message->payload:%.*s%s\n", message->payloadlen, (char*)message->payload ,"");
	}
	fflush(stdout);
	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);
}
```

> ###断开连接

从MQTT服务器断开断开连接接口。为了让客户端有完成处理消息的时间，在调用函数时，可以指定超时期限。
```
long timeout=10000;
MQTTClient_disconnect(client, timeout);
```

> ###释放内存

释放掉分配到MQTT的内存空间，此接口在设备端不再需要MQTT连接是调用。
```
MQTTClient_destroy(&client);
```

> ## MQTT连接-异步
> ### 创建MQTT连接

```
MQTTAsync_create(&client, my_PRO_info.host, my_PRO_info.client_id, 1, NULL);
```


> ### 设置回调

```
MQTTAsync_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
```
msgarrvd参数指向消息回调函数，此参数不能为空，当有新消息匹配此订阅标识时，客户端会调用这个回调函数。如果你的客户端应用程序没有使用特定的回调，可以将其他参数设置为空。connlost回调函数会在客户端连接失败时调用，delivered回调函数会在客户端发布消息到服务器时调用。

```
void delivered(void *context, MQTTAsync_token dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
}
void connlost(void *context, char *cause)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;
    
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
    
    printf("Reconnecting\n");
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
        finished = 1;
    }
}


int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    int i;
    char* payloadptr;
    
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");
    
    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

```

> ### 连接MQTT

```
conn_opts.keepAliveInterval = 20;
conn_opts.cleansession = 1;
conn_opts.onSuccess = onConnect; //连接成功回调
conn_opts.onFailure = onConnectFailure;  //连接失败回调
conn_opts.context = client;
conn_opts.username=my_PRO_info.username;
conn_opts.password=my_PRO_info.password;
    
//连接MQTT：返回值为0时，连接成功
if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS){
	printf("Failed to start connect, return code %d\n", rc);
	exit(-1);
}
```
连接成功会执行onConnect回调函数，连接失败执行onConnectFailure回调。

```
void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
    printf("Connect failed, rc %d\n", response ? response->code : 0);
    finished = 1;
}


void onConnect(void* context, MQTTAsync_successData* response)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;

    printf("Successful connection\n");

    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, my_PRO_info.client_id, QOS);
    opts.onSuccess = onSubscribe; //订阅成功回调
    opts.onFailure = onSubscribeFailure; //订阅失败回调
    opts.context = client;

    deliveredtoken = 0;
		
    if ((rc = MQTTAsync_subscribe(client, TOPIC, QOS, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start subscribe, return code %d\n", rc);
        exit(-1);
    }
}

```

> ### 发送消息

发送成功到服务器后客户端会执行delivered回调函数

```
MQTTAsync_responseOptions res_opts=MQTTAsync_responseOptions_initializer;
MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
pubmsg.payload = PAYLOAD;
pubmsg.payloadlen = strlen(PAYLOAD);
pubmsg.qos = QOS; 
pubmsg.retained = 0;

//发送消息，payload即为需要发送的消息内容，可以拼成JSON字符串，接收端收到后对它进行解析处理
MQTTAsync_sendMessage(client, TOPIC, &pubmsg, &res_opts);
```

> ### 订阅消息

订阅成功执行onSubscribe回调，失败执行onSubscribeFailure回调。

```
MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
opts.onSuccess = onSubscribe; //订阅成功回调
opts.onFailure = onSubscribeFailure; //订阅失败回调
opts.context = client;

if ((rc = MQTTAsync_subscribe(client, TOPIC, QOS, &opts)) != MQTTASYNC_SUCCESS)
{
    printf("Failed to start subscribe, return code %d\n", rc);
    exit(-1);
}
```
> ### 断开连接

断开MQTT连接，disc_opts内onSuccess指定成功回调，onFailure指定失败回调，timeout设置超时期限。

```
disc_opts.onSuccess = onDisconnect;
disc_opts.onFailure = onFailure;
disc_opts.timeout = 10000;
if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != 0)
{
	printf("Failed to start disconnect, return code %d\n", rc);
	exit(-1);
}
```
> ### 释放内存

释放掉分配到MQTT的内存空间，此接口在设备端不再需要MQTT连接是调用。

```
MQTTAsync_destroy(&client);
```

