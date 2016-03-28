//
//  totlink.c
//  dotlink-mqtt-sample
//
//  Created by Jin on 16/3/14.
//  Copyright © 2016年 AppCan. All rights reserved.
//

#include "dotlink.h"

#define dotlink_host    "120.26.77.175"
#define dotlink_port    9000
#define dotlink_path    "/mqtt/info"

PRO_info pro_info;

typedef int (*CALLBACK)(char *p);
int http_post_json(char *json_data, char *hostname, uint16_t port, char *path, CALLBACK cb) {
    int ret = -1;
    int sockfd, h;
    fd_set   t_set1;
    struct sockaddr_in servaddr;
    char buf[4096];
    memset(buf, 0, sizeof(buf));
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("创建网络连接失败,本线程即将终止---socket error!\n");
        return -1;
    }
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    struct hostent *host_entry = gethostbyname(hostname);
    char* p = inet_ntoa(*((struct in_addr *)host_entry->h_addr));
    if (inet_pton(AF_INET, p, &servaddr.sin_addr) <= 0){
        printf("创建网络连接失败,本线程即将终止--inet_pton error!\n");
        return -1;
    }
    
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        return -1;
    
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        printf("连接到服务器失败,connect error!\n");
        return -3;
    }
    printf("与远端建立了连接\n");
    
    char temp[128];
    sprintf(temp, "POST %s HTTP/1.1", path);
    strcat(buf, temp);
    strcat(buf, "\r\n");
    sprintf(temp, "Host: %s:%d", hostname, port),
    strcat(buf, temp);
    strcat(buf, "\r\n");
    strcat(buf, "Accept: application/json\r\n");
    strcat(buf, "Content-Type: application/json\r\n");
    strcat(buf, "Content-Length: ");
    sprintf(temp, "%lu", strlen(json_data)),
    strcat(buf, temp);
    strcat(buf, "\n\n");
    strcat(buf, json_data);
    
    ret = write(sockfd, buf, strlen(buf));
    if (ret < 0) {
        printf("发送失败！错误代码是%d，错误信息是'%s'\n",errno, strerror(errno));
        close(sockfd);
        return -4;
    }
    
    struct timeval  tv;
    FD_ZERO(&t_set1);
    FD_SET(sockfd, &t_set1);
    tv.tv_sec= 6;
    tv.tv_usec= 0;
    h = select(sockfd + 1, &t_set1, NULL, NULL, &tv);
    if (h > 0) {
        memset(buf, 0, sizeof(buf));
        ssize_t  i= read(sockfd, buf, sizeof(buf));
        //取body
        char *temp = strstr(buf, "\r\n\r\n");
        if (temp) {
            temp += 4;
            ret = cb(temp);
        }
        else{
            printf("读取数据为空！\n");
            ret = -6;
        }
    } else{
        printf("在读取数据报文时SELECT检测到异常，该异常导致线程终止！\n");
        ret = -5;
    }
    close(sockfd);
    return ret;
}
static int pro_cb(const char *json_data) {
    printf("\ndata:%s\n",json_data);
    int ret = 0;
    char buf[500];
    memset(buf, 0, sizeof(buf));
    memcpy(buf, json_data, strlen(json_data));
    
    cJSON *root = cJSON_Parse(buf);
    if (root) {
        int ret_size = cJSON_GetArraySize(root);
        if (ret_size >= 4) {
            strcpy(pro_info.client_id, cJSON_GetObjectItem(root,"client")->valuestring);
            strcpy(pro_info.username, cJSON_GetObjectItem(root,"username")->valuestring);
            strcpy(pro_info.password, cJSON_GetObjectItem(root,"password")->valuestring);
            strcpy(pro_info.host, cJSON_GetObjectItem(root,"host")->valuestring);
        } else
            ret = -1;
        
        cJSON_Delete(root);
    }
    return ret;
}

int MQTTClient_setup_with_pid_and_did(char* pid, char *did, char *productSecret, PRO_info *info)
{
    char json_data[1024];
    char sign[33];
    if (pid == NULL || did==NULL || productSecret==NULL){
        return -11;
    }
    
    //md5签名
    MD5_CTX md5;
    MD5Init(&md5);
    int i;
    unsigned char encrypt[1024];
    unsigned char decrypt[16];
    sprintf(encrypt,"%sdid%spid%s%s",productSecret, did, pid ,productSecret);
    MD5Update(&md5,encrypt,strlen((char *)encrypt));
    MD5Final(&md5,decrypt);
    printf("\n加密前:%s\n加密后(32位):",encrypt);
    for(i=0;i<16;i++)
    {
        printf("%02x",decrypt[i]);
        sprintf(sign,"%s%02x",sign,decrypt[i]);
    }
    sprintf(json_data,"{\"did\": \"%s\", \"pid\": \"%s\", \"sign\": \"%s\"}", did, pid, sign);
    
    int ret = http_post_json(json_data, dotlink_host, dotlink_port, dotlink_path, pro_cb);
    if (ret < 0)
        return -1;
    
    strcpy(info->client_id, pro_info.client_id);
    strcpy(info->username, pro_info.username);
    strcpy(info->password, pro_info.password);
    strcpy(info->host, pro_info.host);
    return 0;
}
