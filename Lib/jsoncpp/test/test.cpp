#include <iostream>
#include <string>
using namespace std;

#include "json/json.h"

int main()
{
	/*string str = "{\"iRet\":\"0\", \"sMsg\":\"ok\", \"sReason\":\"\", \"iActivityId\":\"1000101\", \"iRuleId\":\"11094\", \"iPackageId\":\"22908\", \"sPackageName\":\"1000ª∂¿÷∂π\", \"iPackageType\":\"0\", \"sPackageOtherInfo\":\"\", \"dTimeNow\":\"2013-12-10 15:09:48\", \"sSerialNum\":\"AMS-QQGAME-20131210150948-NWLRBB-1000140-11094\" }";

	Json::Reader reader;
	Json::Value root;
	if(reader.parse(str, root)) {
		string iRet = root["iRet"].asString();
		string iPackageId = root["iPackageId"].asString();
		string sPackageName = root["sPackageName"].asString();
		printf("%s %s %s\n", iRet.c_str(), iPackageId.c_str(), sPackageName.c_str());
	}*/
	Json::Value root;
	Json::Value arrayObj;
	Json::Value item;
	item["cpp"] = "jsoncpp";
	item["java"] = "jsoninjava";
	item["php"] = "support";
	arrayObj.append(item);
	Json::Value item2;
	item2["cpp"] = "jsoncpp";
	item2["java"] = "jsoninjava";
	item2["php"] = "support";
	arrayObj.append(item2);
	root["name"] = "json";
	root["array"] = arrayObj;
	root["age"] = "30";
	std::cout << root.toStyledString() << std::endl;

	return 0;
}
