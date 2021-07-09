/*
 * =====================================================================================
 *
 *       Filename:  UpdateData.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/21/2010 03:04:24 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ericsha (shakaibo), ericsha@tencent.com
 *        Company:  Tencent, China
 *
 * =====================================================================================
 */

#ifndef __UPDATEDATA_H__
#define __UPDATEDATA_H__

#include "IAction.h"

USING_ASYNCFRAME_NS;

class CUpdateData
: public IAction
{
    public:
        // 请求打包
        virtual int HandleEncode( CAsyncFrame *pFrame,
                char *buf,
                int &len,
                CMsgBase *pMsg);

        // 回应包完整性检查
        virtual int HandleInput( CAsyncFrame *pFrame,
            const char *buf,
            int len,
            CMsgBase *pMsg);

        // 回应包处理
        virtual int HandleProcess( CAsyncFrame *pFrame,
            const char *buf,
            int len,
            CMsgBase *pMsg);


        virtual int HandleError( CAsyncFrame *pFrame,
                int err_no,
                CMsgBase *pMsg);
};

#endif
