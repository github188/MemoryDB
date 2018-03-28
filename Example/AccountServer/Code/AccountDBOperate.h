#include "DBNodeOperate.h"

#define LOGIN_STATE_OVER_TIME		(10*60)

enum OPERATE_ERROR
{

};


class f_create_account : public tDBNodeOpreate
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};

class f_login_check : public tDBNodeOpreate
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};


class f_check_login_state: public tDBNodeOpreate
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};

class f_get_beta_test_diamond : public tDBNodeOpreate
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};

class f_use_cdkey : public tDBNodeOpreate
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};

class f_create_cdkey : public tDBNodeOpreate
{
public:
	AString genCDKeyString(char * buff, char c1, int n);

	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};

// 逻辑消息
class AccountThread;
class tLogicBaseMsg : public tDBNodeOpreate
{
public:
	AccountThread* GetThread(MemoryDBNode *pNode);

	void BroadCastMsg(MemoryDBNode *pNode, AutoEvent evt);
};

class e_notify_serverinfo : public tLogicBaseMsg
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};


class e_notify_gs_count : public tLogicBaseMsg
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};

class e_notify_online_count : public tLogicBaseMsg
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};

class e_notify_version_changed : public tLogicBaseMsg
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};

// 通过SDK创建,无需要密码, 中用于保存创建日期, 帐号为渠道ID_UID
class f_sdk_try_create_account : public tDBNodeOpreate
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};

// WEB 调用部分
// WEB 请求处理定单, 参数 GOODS_INDEX, PLAYER_ID, SERVER_ID, SDK_OLDER, AMOUNT
class f_web_pay_event : public tDBNodeOpreate
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);

	virtual bool _OnEvent(AutoEvent &evt) override;

	virtual void InitData() override
	{
		tDBNodeOpreate::InitData();
		mPayTable.setNull();
		mParamData.setNull();
	}

public:
	AutoTable		mPayTable;
	AutoNice			mParamData;
};

// 上传服务器运行状态
class f_update_server_run_state : public tLogicBaseMsg
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
	
};

// 请求统计支付金额
class f_request_total_pay_amount : public tDBNodeOpreate
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};

// 请求服务区的前端控制地址
class f_request_res_fore_net_key : public tLogicBaseMsg
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);

	bool _OnEvent(AutoEvent &evt) override;
};

// 请求所有开放的IP地址
class f_request_all_login_gs_ip : public tLogicBaseMsg
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);

	void ResponseFinish();

	bool _OnEvent(AutoEvent &evt) override;

	virtual bool _OnTimeOver() override
	{
		ResponseFinish();
		return true;
	}

	virtual void InitData() override
	{
		tLogicBaseMsg::InitData();
		mRequestList.clear(true);
		mpNode = NULL;
	}

	f_request_all_login_gs_ip()
		: mpNode(NULL){}

public:
	Array<AutoEvent>		mRequestList;
	MemoryDBNode			*mpNode;
};

class e_request_modify_server_state : public tLogicBaseMsg
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};

// 广播公告
class f_update_anno_info : public tLogicBaseMsg
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};

class AL_RequestPayEvent : public Logic::tServerEvent
{
public:
	virtual void _OnResp(AutoEvent &respEvent) override
	{
		if (mWaitOperate)
			mWaitOperate->OnEvent(respEvent);
	}

	virtual void InitData() override
	{
		Logic::tServerEvent::InitData();
		mWaitOperate.setNull();
	}

	virtual bool _OnTimeOver() override
	{
		AutoEvent evt;
		if (mWaitOperate)
			mWaitOperate->OnEvent(evt);
		return true;
	}

public:
	Hand<tDBNodeOpreate> mWaitOperate;
};

// 请求获取Res Fore net key
class AL_RequestResForeNetKey : public Logic::tServerEvent
{
public:
	virtual void _OnResp(AutoEvent &respEvent) override
	{
		if (mWaitOperate)
			mWaitOperate->OnEvent(respEvent);
	}

	virtual void InitData() override
	{
		Logic::tServerEvent::InitData();
		mWaitOperate.setNull();
	}

	virtual bool _OnTimeOver() override
	{
		AutoEvent evt;
		if (mWaitOperate)
			mWaitOperate->OnEvent(evt);
		return true;
	}

public:
	Hand<tDBNodeOpreate> mWaitOperate;
};

// 请求所有开放地址
class AL_RequestAllLoginGSIP : public Logic::tServerEvent
{
public:
	virtual void _OnResp(AutoEvent &respEvent) override
	{
		mResponeEvent = respEvent;
		if (mWaitOperate)
			mWaitOperate->OnEvent(respEvent);
	}

	virtual void InitData() override
	{
		Logic::tServerEvent::InitData();
		mWaitOperate.setNull();
		mResponeEvent.setNull();
	}

public:
	Hand<tDBNodeOpreate> mWaitOperate;
	AutoEvent			mResponeEvent;
};

// 上传照片数据
class f_upload_photo : public tLogicBaseMsg
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};

class f_load_photo  : public tLogicBaseMsg
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};

class f_load_db_data : public tLogicBaseMsg
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};

class f_update_db_data : public tLogicBaseMsg
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};

class f_save_news_feed : public tLogicBaseMsg
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};

class f_load_news_feed : public tLogicBaseMsg
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);
};

class f_load_news_feed_big_pic : public tLogicBaseMsg
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData);

};