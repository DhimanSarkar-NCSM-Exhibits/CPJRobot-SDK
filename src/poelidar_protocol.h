/*
 * Copyright 2023-2024 Shanghai CPJRobot Co., Ltd. All rights reserved.
 * http://www.cpjrobot.com
 * http://www.poelidar.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POELIDAR_PROTOCOL_H
#define POELIDAR_PROTOCOL_H

#include <stdint.h>

#define CPJ_F1_CMD_TYPE_GET     0x01
#define CPJ_F1_CMD_TYPE_SET     0x02
#define CPJ_F1_CMD_TYPE_START   0x03
#define CPJ_F1_CMD_TYPE_STOP    0x04
#define CPJ_F1_CMD_TYPE_QUERY   0x05
#define CPJ_F1_CMD_TYPE_REMBER  0x06
#define CPJ_F1_CMD_TYPE_REBOOT  0x07


const uint8_t F1_CMD_TYPE_GET[14] =
{
34 , 115, 101, 116, 116,
105, 110, 103, 115, 47 ,
103, 101, 116, 34
};

const uint8_t F1_CMD_TYPE_SET[14] =
{
34 , 115, 101, 116, 116,
105, 110, 103, 115, 47 ,
115, 101, 116, 34
};

const uint8_t F1_CMD_TYPE_START[21] =
{
34 , 115, 99 , 97 , 110,
47 , 115, 116, 97 , 114,
116, 83 , 116, 114, 101,
97 , 109, 105, 110, 103,
34
};

const uint8_t F1_CMD_TYPE_STOP[20] =
{
34 , 115, 99 , 97 , 110,
47 , 115, 116, 111, 112,
83 , 116, 114, 101, 97 ,
109, 105, 110, 103, 34
};

const uint8_t F1_CMD_TYPE_QUERY[18] =
{
34 , 100, 101, 118, 105,
99 , 101, 47 , 113, 117,
101, 114, 121, 73 , 110,
102, 111, 34
};

const uint8_t F1_CMD_TYPE_REMBER[18] =
{
34 , 115, 101, 116, 116,
105, 110, 103, 115, 47 ,
112, 101, 114, 115, 105,
115, 116, 34
};

const uint8_t F1_CMD_TYPE_REBOOT[27] =
{
34 , 100, 101, 118, 105,
99 , 101, 47 , 114, 101,
98 , 111, 111, 116, 84 ,
111, 66 , 111, 111, 116,
108, 111, 97 , 100, 101,
114, 34
};


const uint8_t F1_CMD_HEAD[26] =
{
123, 34 , 106, 115, 111,
110, 114, 112, 99 , 34 ,
58 , 34 , 50 , 46 , 48 ,
34 , 44 , 34 , 109, 101,
116, 104, 111, 100, 34 ,
58
};


const uint8_t F1_CMD_END[3] =
{
125, 13 ,10
};

const uint8_t F1_PARAMS_HEAD[19] =
{
44 , 34 , 112, 97 , 114,
97 , 109, 115, 34 , 58 ,
123, 34 , 101, 110, 116,
114, 121, 34 , 58
};

const uint8_t F1_VALUE_HEAD[9] =
{
44 , 34 , 118, 97 , 108,
117, 101, 34 , 58
};

const uint8_t F1_ID_HEAD[6] =
{
44 , 34 , 105, 100, 34 ,
58
};


const uint8_t F1_PARAMS_SN[17] =
{
34 , 105, 100, 101, 110,
116, 105, 116, 121, 46 ,
115, 101, 114, 105, 97 ,
108, 34
};

const uint8_t F1_PARAMS_IP[35] =
{
34 , 99 , 111, 110, 110,
101, 99 , 116, 105, 118,
105, 116, 121, 46 , 110,
101, 116, 119, 111, 114,
107, 46 , 105, 112, 118,
52 , 46 , 97 , 100, 100,
114, 101, 115, 115, 34
};

const uint8_t F1_PARAMS_SUBNET[34] =
{
34 , 99 , 111, 110, 110,
101, 99 , 116, 105, 118,
105, 116, 121, 46 , 110,
101, 116, 119, 111, 114,
107, 46 , 105, 112, 118,
52 , 46 , 115, 117, 98 ,
110, 101, 116, 34
};

const uint8_t F1_PARAMS_DATA_IP[29] =
{
34 , 116, 114, 97 , 110,
115, 112, 111, 114, 116,
46 , 111, 111, 98 , 46 ,
116, 97 , 114, 103, 101,
116, 65 , 100, 100, 114,
101, 115, 115, 34
};

const uint8_t F1_PARAMS_DATA_PORT[26] =
{
34 , 116, 114, 97 , 110,
115, 112, 111, 114, 116,
46 , 111, 111, 98 , 46 ,
116, 97 , 114, 103, 101,
116, 80 , 111, 114, 116,
34
};

const uint8_t F1_PARAMS_SPEED[16] =
{
34 , 115, 99 , 97 , 110,
46 , 102, 114, 101, 113,
117, 101, 110, 99 , 121,
34
};



typedef struct HeadDataPackF1_ {
    uint16_t  sync_byte;
    uint16_t  rsv1;
    uint8_t   block_idx;
    uint8_t   rsv2;
    uint16_t  count;
    uint32_t  rsv3;
    uint16_t  checksum;
    uint16_t  rsv4;
} HeadDataPackF1;


//-------------------------

const uint8_t M1_CMD_TYPE_PW_ON[2] =
{
  0xA5,
  0x30
};

const uint8_t M1_CMD_TYPE_START[2] =
{
  0xA5,
  0x20
};

const uint8_t M1_CMD_TYPE_STOP[2] =
{
  0xA5,
  0x25
};


#define M1_NODE_NUM_PER_PACK  12
#define M1_RES_SYNC_HEADER    0x54

typedef struct poelidar_m1_node_ {
    uint16_t   distance;
    uint8_t    confidence;
} __attribute__((packed)) poelidar_m1_node;

typedef struct DataPackM1_ {
    uint8_t header;
    uint8_t nodeNum;
    uint16_t speed;
    uint16_t startAngle;
    poelidar_m1_node nodes[M1_NODE_NUM_PER_PACK];
    uint16_t endAngle;
    uint16_t timestamp;
    uint8_t CRC8;
} __attribute__((packed)) DataPackM1;



const uint8_t kM1CrcTable[256] =
{
    0x00, 0x4d, 0x9a, 0xd7, 0x79, 0x34, 0xe3,
    0xae, 0xf2, 0xbf, 0x68, 0x25, 0x8b, 0xc6, 0x11, 0x5c, 0xa9, 0xe4, 0x33,
    0x7e, 0xd0, 0x9d, 0x4a, 0x07, 0x5b, 0x16, 0xc1, 0x8c, 0x22, 0x6f, 0xb8,
    0xf5, 0x1f, 0x52, 0x85, 0xc8, 0x66, 0x2b, 0xfc, 0xb1, 0xed, 0xa0, 0x77,
    0x3a, 0x94, 0xd9, 0x0e, 0x43, 0xb6, 0xfb, 0x2c, 0x61, 0xcf, 0x82, 0x55,
    0x18, 0x44, 0x09, 0xde, 0x93, 0x3d, 0x70, 0xa7, 0xea, 0x3e, 0x73, 0xa4,
    0xe9, 0x47, 0x0a, 0xdd, 0x90, 0xcc, 0x81, 0x56, 0x1b, 0xb5, 0xf8, 0x2f,
    0x62, 0x97, 0xda, 0x0d, 0x40, 0xee, 0xa3, 0x74, 0x39, 0x65, 0x28, 0xff,
    0xb2, 0x1c, 0x51, 0x86, 0xcb, 0x21, 0x6c, 0xbb, 0xf6, 0x58, 0x15, 0xc2,
    0x8f, 0xd3, 0x9e, 0x49, 0x04, 0xaa, 0xe7, 0x30, 0x7d, 0x88, 0xc5, 0x12,
    0x5f, 0xf1, 0xbc, 0x6b, 0x26, 0x7a, 0x37, 0xe0, 0xad, 0x03, 0x4e, 0x99,
    0xd4, 0x7c, 0x31, 0xe6, 0xab, 0x05, 0x48, 0x9f, 0xd2, 0x8e, 0xc3, 0x14,
    0x59, 0xf7, 0xba, 0x6d, 0x20, 0xd5, 0x98, 0x4f, 0x02, 0xac, 0xe1, 0x36,
    0x7b, 0x27, 0x6a, 0xbd, 0xf0, 0x5e, 0x13, 0xc4, 0x89, 0x63, 0x2e, 0xf9,
    0xb4, 0x1a, 0x57, 0x80, 0xcd, 0x91, 0xdc, 0x0b, 0x46, 0xe8, 0xa5, 0x72,
    0x3f, 0xca, 0x87, 0x50, 0x1d, 0xb3, 0xfe, 0x29, 0x64, 0x38, 0x75, 0xa2,
    0xef, 0x41, 0x0c, 0xdb, 0x96, 0x42, 0x0f, 0xd8, 0x95, 0x3b, 0x76, 0xa1,
    0xec, 0xb0, 0xfd, 0x2a, 0x67, 0xc9, 0x84, 0x53, 0x1e, 0xeb, 0xa6, 0x71,
    0x3c, 0x92, 0xdf, 0x08, 0x45, 0x19, 0x54, 0x83, 0xce, 0x60, 0x2d, 0xfa,
    0xb7, 0x5d, 0x10, 0xc7, 0x8a, 0x24, 0x69, 0xbe, 0xf3, 0xaf, 0xe2, 0x35,
    0x78, 0xd6, 0x9b, 0x4c, 0x01, 0xf4, 0xb9, 0x6e, 0x23, 0x8d, 0xc0, 0x17,
    0x5a, 0x06, 0x4b, 0x9c, 0xd1, 0x7f, 0x32, 0xe5, 0xa8
};


#endif // POELIDAR_PROTOCOL_H
