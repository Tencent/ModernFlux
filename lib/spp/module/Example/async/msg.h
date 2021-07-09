/*
 * =====================================================================================
 *
 *       Filename:  msg.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/21/2010 03:31:57 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ericsha (shakaibo), ericsha@tencent.com
 *        Company:  Tencent, China
 *
 * =====================================================================================
 */

#ifndef __MSG_H__
#define __MSG_H__
#include "MsgBase.h"

USING_ASYNCFRAME_NS;

#define STATE_ID_FINISHED   0
#define STATE_ID_GET        1
#define STATE_ID_UPDATE     2
#define STATE_ID_GET2       3

class CMsg
    : public CMsgBase
{
    public:
        int uin;
        int level;
        int coin;
        int seed_num;
        int gain_num;
        int result;
};


#endif
