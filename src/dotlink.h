//
//  dotlink.h
//  dotlink-mqtt-sample
//
//  Created by Jin on 16/3/14.
//  Copyright © 2016年 AppCan. All rights reserved.
//
#if defined(WIN32) || defined(WIN64)
#define DLLImport __declspec(dllimport)
#define DLLExport __declspec(dllexport)
#else
#define DLLImport extern
#define DLLExport __attribute__ ((visibility ("default")))
#endif

#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include "md5.h"
#include "cJSON.h"
#include "Socket.h"

typedef void* appcan;
typedef struct {
    char client_id[200];
    char username[200];
    char password[200];
    char device_id[200];
    char host[200];
} PRO_info;

typedef struct {
    int code;
    int order_id;
    int did;
    char data_points[2048];
    int result;
    char resp[128];
    int timestamp;
} payload_info;

typedef struct {
    char key[100];
    char value[100];
} data_point_info;

DLLExport int MQTTClient_setup_with_pid_and_did(char* pid, char *did, char *productSecret, PRO_info *info);