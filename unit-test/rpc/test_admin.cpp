﻿/**
 * Tencent is pleased to support the open source community by making Tars available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed 
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 */

#include "hello_test.h"
#include "../server/WinServer.h"
#include "servant/AdminF.h"
#include "mock/TarsMockUtil.h"
#include "mock/DbHandle.h"
#include "mock/ConfigImp.h"

TEST_F(HelloTest, testAdmin)
{
    TarsMockUtil tarsMockUtil;
    tarsMockUtil.startFramework();
    ConfigImp::setConfigFile("test.conf", "test-content");

	CDbHandle::cleanEndPoint();

	WinServer ws;
	startServer(ws, WIN_CONFIG());

	CommunicatorPtr c = ws.getApplicationCommunicator();

	string adminObj = "AdminObj@" + getLocalEndpoint(WIN_CONFIG()).toString();

	AdminFPrx adminFPrx = c->stringToProxy<AdminFPrx>(adminObj);

    adminFPrx->tars_ping();

    LOG_CONSOLE_DEBUG << endl;
	string loadconfig = adminFPrx->notify("tars.loadconfig test.conf");
    LOG_CONSOLE_DEBUG << loadconfig << endl;

	EXPECT_TRUE(loadconfig.find("[succ] get remote config:") != string::npos);
	loadconfig = adminFPrx->notify("tars.loadconfig no-test.conf");

	EXPECT_TRUE(loadconfig.find("[fail] get remote config ") != string::npos);

	string setloglevel = adminFPrx->notify("tars.setloglevel DEBUG");
	EXPECT_TRUE(setloglevel.find("set log level [DEBUG] ok") != string::npos);

	string viewstatus = adminFPrx->notify("tars.viewstatus");
	EXPECT_TRUE(viewstatus.find("notify prefix object num:1") != string::npos);

	string viewversion = adminFPrx->notify("tars.viewversion");
	EXPECT_TRUE(viewversion.find("$") != string::npos);

	string connection = adminFPrx->notify("tars.connection");
	EXPECT_TRUE(connection.find("[adapter:AdminAdapter]") != string::npos);

	ServerConfig::ConfigFile = CONFIGPATH + "/server/windows.conf";

	string loadproperty = adminFPrx->notify("tars.loadproperty");
	EXPECT_TRUE(loadproperty.find("loaded config items:") != string::npos);

	string help = adminFPrx->notify("tars.help");
	EXPECT_TRUE(help.find("tars.closecore") != string::npos);

	string closecout = adminFPrx->notify("tars.closecout NO");
	string enabledaylog = adminFPrx->notify("tars.enabledaylog local|daylog|true");
	EXPECT_TRUE(enabledaylog.find("set local daylog true ok") != string::npos);
	enabledaylog = adminFPrx->notify("tars.enabledaylog remote|false");
	EXPECT_TRUE(enabledaylog.find("set remote false ok") != string::npos);

	string reloadlocator = adminFPrx->notify("tars.reloadlocator reload");
	EXPECT_TRUE(reloadlocator.find("[notify prefix object num:1]") != string::npos);

	string errorcmd = adminFPrx->notify("tars.errorcmd");
	EXPECT_STREQ(errorcmd.c_str(), "");

	string normalcmd = adminFPrx->notify("AdminCmdNormalTest returnMark");
	EXPECT_STREQ(normalcmd.c_str(), "[notify servant object num:1]\n[1]:return Mark AdminCmdNormalTest success!\n");

	string normaldeletecmd = adminFPrx->notify("DeletePrefixCmd");
	EXPECT_STREQ(normaldeletecmd.c_str(), "[notify servant object num:1]\n[1]:Delete success!\n");

	stopServer(ws);
    tarsMockUtil.stopFramework();
}

TEST_F(HelloTest, testAdminShutdown)
{
    TarsMockUtil tarsMockUtil;
    tarsMockUtil.startFramework();
    ConfigImp::setConfigFile("test.conf", "test-content");

    WinServer ws;
    startServer(ws, WIN_CONFIG());

    CommunicatorPtr c = ws.getApplicationCommunicator();

    string adminObj = "AdminObj@" + getLocalEndpoint(WIN_CONFIG()).toString();

    AdminFPrx adminFPrx = c->stringToProxy<AdminFPrx>(adminObj);

    adminFPrx->tars_ping();

    try
    {
        adminFPrx->tars_set_timeout(2000)->shutdown();
        TC_Common::msleep(500);
    }
    catch(exception &ex)
    {
        LOG_CONSOLE_DEBUG << ex.what() << endl;
    }

    EXPECT_EQ(ws._destroyApp, true);

    stopServer(ws);
    tarsMockUtil.stopFramework();

}