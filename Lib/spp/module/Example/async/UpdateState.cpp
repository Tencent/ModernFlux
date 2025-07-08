/*
 * =====================================================================================
 *
 *       Filename:  GetState.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/21/2010 03:39:54 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ericsha (shakaibo), ericsha@tencent.com
 *        Company:  Tencent, China
 *
 * =====================================================================================
 */

#include "UpdateState.h"
#include "UpdateData.h"
#include "ActionInfo.h"
#include "AsyncFrame.h"
#include "CommDef.h"
#include "msg.h"


int CUpdateState::HandleEncode(CAsyncFrame *pFrame,
        CActionSet *pActionSet,
        CMsgBase *pMsg) 
{
    static CUpdateData UpdateData;

    CActionInfo *pAction1 = new CActionInfo(512);
    pAction1->SetID(3);
    pAction1->SetDestIp("ip");
    pAction1->SetDestPort(5575);
    pAction1->SetProto(ProtoType_TCP);
    pAction1->SetActionType(ActionType_SendRecv_KeepAliveWithPending);
    pAction1->SetTimeout(200);
    pAction1->SetIActionPtr((IAction *)&UpdateData);

    pActionSet->AddAction(pAction1);

    return 0;
}

int CUpdateState::HandleProcess(CAsyncFrame *pFrame,
        CActionSet *pActionSet,
        CMsgBase *pMsg)
{
    CMsg *msg = (CMsg *)pMsg;
    int err1 = 0 ;
    int cost1 = 0;
    int size1 = 0;

    CActionSet::ActionSet &action_set = pActionSet->GetActionSet();
    CActionSet::ActionSet::iterator it = action_set.begin();
    for(; it != action_set.end(); it++ )
    {
        CActionInfo *pInfo = *it;
        char *buf = NULL;

        int id;
        pInfo->GetID(id);
        
        if( id == 3 )
        {
            pInfo->GetErrno(err1);
            pInfo->GetBuffer( &buf, size1);
            if( err1 == 0 )
            {
                if( size1 == sizeof(int)*3
                        && msg->uin == *(int *)buf )
                {
                    msg->result = *(int*)(buf+sizeof(int)*2);
                }
            }
            else
            {
                msg->result = -1;
                msg->level = -3;
                msg->coin = -3;

                break;
            }

            pInfo->GetTimeCost( cost1 );

        }
    }

    /*
    pFrame->FRAME_LOG( LOG_DEBUG, 
            "uin: %d, level: %d, coin: %d, err1: %d, cost1: %d, size1: %d, err2: %d, cost2: %d, size2: %d, action_num: %u\n", 
            msg->uin, msg->level, msg->coin, err1, cost1, size1, err2, cost2, size2, action_set.size());
    */
    return STATE_ID_FINISHED;
}
