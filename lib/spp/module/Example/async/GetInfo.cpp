/*
 * =====================================================================================
 *
 *       Filename:  GetInfo.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/21/2010 03:04:10 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ericsha (shakaibo), ericsha@tencent.com
 *        Company:  Tencent, China
 *
 * =====================================================================================
 */

#include "GetInfo.h"
#include "msg.h"
#include "sppincl.h"

extern int seq;

// 请求打包
int CGetInfo::HandleEncode( CAsyncFrame *pFrame,
        char *buf,
        int &len,
        CMsgBase *pMsg)
{
    return -1;
    if( len < 1024 )
    {
        return 1024;
    }

    CMsg *msg = (CMsg*)pMsg;
    *(int *)buf = msg->uin;
    *((int*)buf+1) = seq;
    len = 2*sizeof(int);

    SetCurrentSeq(seq++); // Set when using ActionType_SendRecv_Parallel

    pFrame->FRAME_LOG( LOG_DEBUG, "CGetInfo::HandleEncode. seq: %d\n", *((int*)buf+1));
    return 0;
}

// 回应包完整性检查
int CGetInfo::HandleInput( CAsyncFrame *pFrame,
        const char *buf,
        int len,
        CMsgBase *pMsg)
{
    pFrame->FRAME_LOG( LOG_DEBUG, "CGetInfo::HandleInput. buf len: %d\n", len);
    if( len == sizeof(int)*4 )
    {
        int seqno = *(int*)(buf+sizeof(int)*3);
        SetCurrentSeq( seqno ); // Set when using ActionType_SendRecv_Parallel
        return len;
    }
    return 0;
}

// 回应包处理
int CGetInfo::HandleProcess( CAsyncFrame *pFrame,
        const char *buf,
        int len,
        CMsgBase *pMsg)
{
    
    int seq = *(int*)(buf+sizeof(int)*3);
    int uin = *(int*)(buf);
    int level = *(int*)(buf+sizeof(int));
    int coin = *(int*)(buf+2*sizeof(int));

    pFrame->FRAME_LOG( LOG_DEBUG, "CGetInfo::HandleProcess. buf len: %d; seq: %d; uin: %d; level: %d: coin: %d\n", 
            len, seq, uin, level, coin);

    return 0;
}


int CGetInfo::HandleError( CAsyncFrame *pFrame,
        int err_no,
        CMsgBase *pMsg)
{
    pFrame->FRAME_LOG( LOG_ERROR, "CGetInfo::HandleError. errno: %d\n", err_no);

    return 0;
}
