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

#include "GetState.h"
#include "GetInfo.h"
#include "GetExtInfo.h"
#include "ActionInfo.h"
#include "AsyncFrame.h"
#include "CommDef.h"
#include "msg.h"


int CGetState::HandleEncode(CAsyncFrame *pFrame,
        CActionSet *pActionSet,
        CMsgBase *pMsg) 
{
    static CGetInfo GetInfo;
    static CGetExtInfo GetExtInfo;

    //CMsg *msg = (CMsg *)pUserData;
    CActionInfo *pAction1 = new CActionInfo(512);
    pAction1->SetID(1);
    pAction1->SetDestIp("172.25.0.29");
    pAction1->SetDestPort(5574);
    pAction1->SetProto(ProtoType_TCP);
    pAction1->SetActionType(ActionType_SendRecv_KeepAlive);
    pAction1->SetTimeout(200);
    pAction1->SetIActionPtr((IAction *)&GetInfo);

    pActionSet->AddAction(pAction1);

    CActionInfo *pAction2 = new CActionInfo(512);
    pAction2->SetID(2);
    pAction2->SetDestIp("172.25.0.29");
    pAction2->SetDestPort(5574);
    pAction2->SetProto(ProtoType_TCP);
    pAction2->SetActionType(ActionType_SendRecv_KeepAliveWithPending);
    pAction2->SetTimeout(200);
    pAction2->SetIActionPtr((IAction *)&GetExtInfo);

    pActionSet->AddAction(pAction2);

    return 0;
}

int CGetState::HandleProcess(CAsyncFrame *pFrame,
        CActionSet *pActionSet,
        CMsgBase *pMsg)
{
    int nNextStateID = STATE_ID_UPDATE;

    CMsg *msg = (CMsg *)pMsg;
    int err1 = 0, err2 = 0;
    int cost1 = 0, cost2 = 0;
    int size1 = 0, size2 = 0;

    CActionSet::ActionSet &action_set = pActionSet->GetActionSet();
    CActionSet::ActionSet::iterator it = action_set.begin();
    for(; it != action_set.end(); it++ )
    {
        CActionInfo *pInfo = *it;
        char *buf = NULL;

        int id;
        pInfo->GetID(id);
        
        if( id == 1 )
        {
            pInfo->GetErrno(err1);
            pInfo->GetBuffer( &buf, size1);
            if( err1 == 0 )
            {
                if( size1 == 16 
                        && msg->uin == *(int *)buf )
                {
                    msg->level = *(int *)(buf+sizeof(int));
                    msg->coin = *(int*)(buf+sizeof(int)*2);
                }
            }
            else
            {
                msg->level = -1;
                msg->coin = -1;

                nNextStateID = -1;
                break;
            }

            pInfo->GetTimeCost( cost1 );

        }
        else if( id == 2 )
        {
            pInfo->GetErrno(err2);
            pInfo->GetBuffer( &buf, size2);
            if( err2 == 0 )
            {
                if( size2 == 16 
                        && msg->uin == *(int *)buf )
                {
                    msg->seed_num = *(int *)(buf+sizeof(int));
                    msg->gain_num = *(int*)(buf+sizeof(int)*2);
                }
            }
            else
            {
                msg->level = -2;
                msg->coin = -2;

                nNextStateID = -1;
                break;
            }

            pInfo->GetTimeCost( cost2 );


        }



    }

    /*
    pFrame->FRAME_LOG( LOG_DEBUG, 
            "uin: %d, level: %d, coin: %d, err1: %d, cost1: %d, size1: %d, err2: %d, cost2: %d, size2: %d, action_num: %u\n", 
            msg->uin, msg->level, msg->coin, err1, cost1, size1, err2, cost2, size2, action_set.size());
    */
    //return nNextStateID;
    return STATE_ID_UPDATE;
}
