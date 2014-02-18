#pragma once
#include "Defines.h"
#include ".\ThostTraderApi\ThostFtdcTraderApi.h"

#include <time.h>
#include <map>
#include <queue>
#include <wx/string.h>

#include <wx/event.h>

class wxLog;

struct OrderEntity
{
   int reqID_;
   int orderRef_;
   time_t orderStartTime_;
   time_t orderDoneTime_;
};

class CTraderSpi : public CThostFtdcTraderSpi
{
public:

    explicit CTraderSpi(wxEvtHandler* handler = NULL);
    virtual ~CTraderSpi();


    void SetInvestorInfo(const char* investor, const char* pwd);
    	///用户登录请求
    void ReqUserLogin(const char*, const char*);

    void ReqUserConnect(const char* front, const char* broker);

	///投资者结算结果确认
    void ReqSettlementInfoConfirm();
    ///请求查询合约
    void ReqQryInstrument();
    ///请求查询资金账户
    void ReqQryTradingAccount();
    ///请求查询投资者持仓
    void ReqQryInvestorPosition();
    ///报单录入请求
    void ReqOrderInsert(const char* instrument, double price,
        int director, int offset, int volume);
    void ReqOrderInsert(const char* instrument, int director, 
        int offset, int volume);

    void ReqQryInvestorPositionDetail();
	///报单操作请求
	void ReqOrderAction(const char* instrument, int session, int frontid, const char* orderref);
    void ReqQryAccountregister();
    void ReqQueryBankAccountMoneyByFuture(const char* bankID, const char* bankPWD, const char* accountPWD);
    void ReqTransferByFuture(const char* bankID, const char* bankPWD, const char* accountPWD, double amount, bool f2B);

private:
    //当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
    virtual void OnFrontConnected();

    ///登录请求响应
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///投资者结算结果确认响应
    virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
    ///请求查询合约响应
    virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    //请求查询资金账户响应
    virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///请求查询投资者持仓响应
    virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///报单录入请求响应
    virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///报单操作请求响应
    virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///错误应答
    virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
    ///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
    virtual void OnFrontDisconnected(int nReason);
		
    ///心跳超时警告。当长时间未收到报文时，该方法被调用。
    virtual void OnHeartBeatWarning(int nTimeLapse);
	
    virtual void OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm,
        CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
    ///请求查询投资者结算结果响应
    virtual void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, 
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField* pInvestorPositionDetail,
										CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
    virtual void OnRspQryAccountregister(CThostFtdcAccountregisterField* pAccountregister,
                                    CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
    virtual void OnRtnQueryBankBalanceByFuture(CThostFtdcNotifyQueryAccountField* pNotifyQueryAccount);
    ///报单通知
    virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

    ///成交通知
    virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);
        
    CThostFtdcInstrumentField GetInstrumentById(const std::string& id) const;
    CThostFtdcInstrumentField GetInstrumentByLabel(const std::string& label) const;

private:

    // 是否收到成功的响应
    bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
    // 是否正在交易的报单
    bool IsTradingOrder(CThostFtdcOrderField *pOrder);

    void DisplayMsg(const wxString& msg, int type = MAIN_WIN_MSG_RUNTIME_LOG);    

private:

    wxLog* logTarget_;

    int reqID_;
    int	orderRef_;	//报单引用
    TThostFtdcFrontIDType	frontID_;	//前置编号
    TThostFtdcSessionIDType	sessionID_;	//会话编号

    std::string investor_;
    std::string password_;
    std::string broker_;
    std::string frontAddr_;
    std::string tradingDay_;
    std::string settlementInfo_;

    CThostFtdcTraderApi* pTraderApi_;


    // all the following data are accessed from separated thread
    // don't access them from GUI thread.
    std::map<int, CThostFtdcInvestorPositionDetailField> positionDetailData_;
    std::map<std::string, CThostFtdcInstrumentField> instrumentData_;
    std::map<std::string, OrderEntity> orderSend_;
    std::map<std::string, OrderEntity> orderDelete_;
    std::map<std::string, OrderEntity> orderDone_;
    std::map<std::string, CThostFtdcInvestorPositionField> positionData_;
    std::map<std::string, std::string> instrumentName2ID_;

    std::queue<std::string> msgDisplayer_;

    wxEvtHandler* winHandler_;
};

