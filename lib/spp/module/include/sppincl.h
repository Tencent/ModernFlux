#ifndef _SPP_INCL_H_
#define _SPP_INCL_H_
#include "spp_incl/spp_version.h"	//框架版本号
#include "spp_incl/tlog.h"			//日志
#include "spp_incl/tstat.h"			//统计
#include "spp_incl/tcommu.h"		//通讯组件
#include "spp_incl/serverbase.h"	//服务器容器
#include "spp_incl/asyn_api.h"
#include "spp_incl/spp_async_interface.h"
#include "spp_incl/CommDef.h"
#include "spp_incl/AsyncFrame.h"
#include "spp_incl/IState.h"
#include "spp_incl/IAction.h"
#include "spp_incl/ActionInfo.h"
#include "spp_incl/ActionSet.h"
#include "spp_incl/SEXInterface.h"
#include "spp_incl/ICostStat.h"
#include "spp_incl/report_handle.h"

#define GROUPID(x) (((x)|1<<31))
using namespace tbase::tlog;
using namespace tbase::tstat;
using namespace tbase::tcommu;
using namespace spp::comm;
using namespace spp::comm;
using namespace SPP_STAT_NS;
USING_ASYNCFRAME_NS;

#endif
