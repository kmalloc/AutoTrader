#ifndef _FEATURE_TRADING_QUOTE_H_
#define _FEATURE_TRADING_QUOTE_H_
//#pragma once
#include "ThostApi/ThostFtdcMdApi.h"
#include "Defines.h"
#include <time.h>
#include <map>
#include <queue>
#include <wx/string.h>
#include <wx/event.h>
#include <wx/log.h>
#include <wx/thread.h>


class QuoteSpi : public CThostFtdcMdSpi
{
public:
    QuoteSpi(wxEvtHandler* handler, std::map<std::string, CThostFtdcInstrumentField>&);

private:
	///错误应答
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast);

	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
	virtual void OnFrontDisconnected(int nReason);
		
	///心跳超时警告。当长时间未收到报文时，该方法被调用。
	///@param nTimeLapse 距离上次接收报文的时间
	virtual void OnHeartBeatWarning(int nTimeLapse);

	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected();
	
	///登录请求响应
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///订阅行情应答
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///取消订阅行情应答
	virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///深度行情通知
	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);
	virtual void SaveTickInfo(CThostFtdcDepthMarketDataField* f);
public:
	void ReqUserLogin(const std::string& investor, const std::string& password);
	void SubscribeMarketData(char* instIdList);
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
	void DisplayMsg(const wxString& msg, int type = MAIN_WIN_MSG_RUNTIME_LOG);
	void SetLoginInfo(const char* investor, const char* password);
	CThostFtdcDepthMarketDataField GetMarketDataByInstrumentId(const char* id) const;

private:
	std::string investor_;
	std::string password_;
	std::string tradingDay_;

	mutable wxMutex tickLocker_;
	std::map<std::string, CThostFtdcDepthMarketDataField> tickData_;
	const std::map<std::string, CThostFtdcInstrumentField> instrumentData_;

	wxEvtHandler* mainWin_;
    CThostFtdcMdApi* pUserApi;
};

#endif