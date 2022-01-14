#ifndef __PDU_HEADER_H_11a1ffaa_1b77_4a16_86e3_b79d02e9b4e6
#define __PDU_HEADER_H_11a1ffaa_1b77_4a16_86e3_b79d02e9b4e6

#include <stdint.h>
#include <arpa/inet.h>

#define PDU_ERR_BUFFER_NOT_ENOUGH -100
#define PDU_ERR_FIELD_DISMATCH -101
#define PDU_ERR_TAG_INVALID -102
#define PDU_ERR_FIELD_NOTEXISTS -103
#define PDU_ERR_WRONG_PKG -104
#define PDU_ERR_ARG_INVALID -105
#define ISDProtocolSOH 0x04
#define ISDProtocolEOT 0x05

#pragma pack(1)
struct pdu_protocol_header{
    uint8_t version;
    uint32_t cmd;
    uint16_t checksum;

    uint32_t seq;
    uint32_t key;
    uint8_t response_flag;	// 2 byte, Server端回应标识 :
    /* 0 - [正常数据, 处理成功],
       1 - [正常数据, 处理失败]
       2 - [异常数据, 服务器拒绝处理]
       3 - [正常数据, 服务器忙, 可重试]
       10 - [服务器重定向]
       20 - [回执包],
       100 - [client请求, 非server回应]
     */

    uint16_t response_info;	// 2 bytes, Server端回应附加信息
    /* 对于处理失败(1):  表示处理失败的错误号errcode
       对于服务器忙(3):  表示重试时间(网络字节序)
       对于服务器拒绝服务(2): 表示拒绝原因(网络字节序)
       其中, 服务器拒绝服务原因定义如下:
       使用的每bit表示不同的拒绝理由, 由低位字节至高分别定义为:
       0x1: 当前协议版本
       0x2: 当前协议命令字
       0x4: 当前client类型
       0x8: 当前client版本
       0x10: 当前client子系统
       相应的位置1表示拒绝, 置0表示不拒绝, 如5位全为0表示无理由拒绝.
       例如, 服务器拒绝当前client类型的当前client版本, 则ServerResponseInfo的取值为0x12.
     */
    char reserved[1];	// 预留
    uint32_t len;		// 协议总长度

} ;
#pragma pack()

/*
输入：
	header qzone包头指针
	buffer 打包目标缓冲区
	len	  目标缓冲区大小
输出:
	返回值 0 成功， <0 失败
	len   打包后使用缓冲区大小
*/
int pdu_header_pack1(struct pdu_protocol_header* header, uint8_t* buffer, uint32_t *len);

/*
输入：
	buffer 打包目标缓冲区,传入的缓冲区至少要有一个字节的额外空间来放置ETX
	buf_len	  目标缓冲区大小
	used_len  已经使用的大小，即来用设置包头的包长字段
输出:
	返回值 0 成功， <0 失败
	used_len   打包后使用缓冲区大小,一般为传入长度+1,用来放置ETX
*/
int pdu_header_pack2(uint8_t* buffer, uint32_t *buf_len, uint32_t *used_len);

/*
输入：
	buffer 待解包的缓冲区
	len    待解包缓冲区长度
	header qzone包头指针
输出：
	返回值 0 成功， <0 失败
	end_len 尾增区长度
*/
int pdu_header_unpack(uint8_t* buffer, uint32_t *len, struct pdu_protocol_header* header, uint32_t* end_len);

//不检查结束符，主要是用来单解包头的时候使用
int pdu_header_unpack2(uint8_t* buffer, uint32_t *len, struct pdu_protocol_header* header, uint32_t* end_len);

#endif
