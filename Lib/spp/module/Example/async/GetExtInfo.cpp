/*
 * =====================================================================================
 *
 *       Filename:  GetExtInfo.cpp
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

#include "GetExtInfo.h"
#include "msg.h"
#include "sppincl.h"

extern int seq;

// 请求打包
int CGetExtInfo::HandleEncode( CAsyncFrame *pFrame,
        char *buf,
        int &len,
        CMsgBase *pMsg)
{
    return -1;

    CMsg *msg = (CMsg*)pMsg;
    *(int *)buf = msg->uin;
    *((int*)buf+1) = seq;
    len = 2*sizeof(int);

    SetCurrentSeq( seq++ ); // Set when using ActionType_SendRecv_Parallel

    pFrame->FRAME_LOG( LOG_DEBUG, "CGetExtInfo::HandleEncode. seq: %d\n", *((int*)buf+1));
    return 0;
}

// 回应包完整性检查
int CGetExtInfo::HandleInput( CAsyncFrame *pFrame,
        const char *buf,
        int len,
        CMsgBase *pMsg)
{
    pFrame->FRAME_LOG( LOG_DEBUG, "CGetExtInfo::HandleInput. buf len: %d\n", len);
    if( len == sizeof(int)*4 )
    {
        int seqno = *(int*)(buf+sizeof(int)*3);
        SetCurrentSeq( seqno ); // Set when using ActionType_SendRecv_Parallel
        return len;
    }
    return 0;
}

// 回应包处理
int CGetExtInfo::HandleProcess( CAsyncFrame *pFrame,
        const char *buf,
        int len,
        CMsgBase *pMsg)
{
    
    int seq = *(int*)(buf+sizeof(int)*3);
    int uin = *(int*)(buf);
    int seed_num  = *(int*)(buf+sizeof(int));
    int gain_num  = *(int*)(buf+2*sizeof(int));

    pFrame->FRAME_LOG( LOG_DEBUG, "CGetExtInfo::HandleProcess. buf len: %d; seq: %d; uin: %d; seed_num: %d: gain_num: %d\n", 
            len, seq, uin, seed_num, gain_num);

    return 0;
}


int CGetExtInfo::HandleError( CAsyncFrame *pFrame,
        int err_no,
        CMsgBase *pMsg)
{
    pFrame->FRAME_LOG( LOG_ERROR, "CGetExtInfo::HandleError. errno: %d\n", err_no);

    return 0;
}
