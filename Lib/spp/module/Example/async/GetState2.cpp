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

#include "GetState2.h"
#include "GetInfo.h"
#include "GetExtInfo.h"
#include "ActionInfo.h"
#include "NonStateActionInfo.h"
#include "StateActionInfo.h"
#include "AntiParalActionInfo.h"
#include "AsyncFrame.h"
#include "CommDef.h"
#include "msg.h"


int CGetState2::HandleEncode(CAsyncFrame *pFrame,
        CActionSet *pActionSet,
        CMsgBase *pMsg) 
{
    static CGetInfo GetInfo;
    static CGetExtInfo GetExtInfo;

    //CMsg *msg = (CMsg *)pUserData;
    CNonStateActionInfo *pAction1 = new CNonStateActionInfo(512);
    pAction1->SetRouteID(1000, 500);
    /*
    static int64_t key = 0;
    key++;

    CStateActionInfo *pAction1 = new CStateActionInfo(512);
    pAction1->SetRouteKey( 200, key, 0 ); 

    */

    /*
    static int64_t key = 0;
    key++;
    CAntiParalActionInfo *pAction1 = new CAntiParalActionInfo(512);
    pAction1->SetRouteKey(1000, 500, key);
    */

    /*
    CActionInfo *pAction1 = new CActionInfo(512);
    pAction1->SetDestIp("172.25.0.29");
    pAction1->SetDestPort(5574);
    */

    pAction1->SetID(1);
    pAction1->SetProto(ProtoType_TCP);
    pAction1->SetActionType(ActionType_SendRecv_KeepAlive);
    pAction1->SetTimeout(400);
    pAction1->SetIActionPtr((IAction *)&GetInfo);

    pActionSet->AddAction(pAction1);
    /*
    CActionInfo *pAction2 = new CActionInfo(512);
    pAction2->SetID(2);
    pAction2->SetDestIp("172.25.0.29");
    pAction2->SetDestPort(5574);
    pAction2->SetProto(ProtoType_TCP);
    pAction2->SetActionType(ActionType_SendRecv_KeepAliveWithPending);
    pAction2->SetTimeout(200);
    pAction2->SetIActionPtr((IAction *)&GetExtInfo);

    pActionSet->AddAction(pAction2);
    */
    return 0;
}

int CGetState2::HandleProcess(CAsyncFrame *pFrame,
        CActionSet *pActionSet,
        CMsgBase *pMsg)
{
    int nNextStateID = 0;

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
            RouteType type =  pInfo->GetRouteType();
            if( type == L5AntiParalRoute )
            {
                std::string ip;
                unsigned short port = 0;
                pInfo->GetDestIp( ip );
                pInfo->GetDestPort( port );
                CAntiParalActionInfo *pAntiParalInfo = (CAntiParalActionInfo *)pInfo;
                int modid = 0, cmdid = 0;
                int64_t key = -1;
                pAntiParalInfo->GetRouteKey( modid, cmdid, key );

                std::string ideal_ip;
                unsigned short ideal_port = 0;
                pAntiParalInfo->GetIdealHost(ideal_ip, ideal_port);

                pFrame->FRAME_LOG( LOG_ERROR, "modid: %d, cmdid: %d, key: %lld, ip: %s, port: %d, ideal_ip: %s, ideal_port: %d\n",
                        modid, cmdid, key, ip.c_str(), port, ideal_ip.c_str(), ideal_port); 

            }
            pInfo->GetErrno(err1);
            pInfo->GetBuffer( &buf, size1);
            pInfo->GetTimeCost( cost1 );
            if( err1 == -5  )
            {
                pFrame->FRAME_LOG( LOG_ERROR, "ECode: %d, Cost: %d\n", err1, cost1); 
            }

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
    return nNextStateID;
}
