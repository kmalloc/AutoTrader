#include <iostream>
#include <stdio.h>
using namespace std;

#include ".\ThostTraderApi\ThostFtdcTraderApi.h"
#include ".\ThostTraderApi\ThostFtdcUserApiDataType.h"

#include "Defines.h"
#include "TraderSpi.h"

#include <wx/log.h>

using namespace std;

#pragma warning(disable : 4996)

// 配置参数
extern wxLog* CreateWxLogTarget(const wxString& log_file);


CTraderSpi::CTraderSpi(wxEvtHandler* handler)
    :logTarget_(NULL)
    ,reqID_(0), frontID_(-1)
    ,pTraderApi_(NULL)
    ,winHandler_(handler)
{
}

CTraderSpi::~CTraderSpi()
{
    if (logTarget_) delete logTarget_;

    if (pTraderApi_) pTraderApi_->Release();
}

CThostFtdcInstrumentField CTraderSpi::GetInstrumentById(const string& id) const
 {
     CThostFtdcInstrumentField tmp = {0};
     map<string, CThostFtdcInstrumentField>::const_iterator it = instrumentData_.find(id);
     if (it != instrumentData_.end()) tmp = it->second;

     return tmp;
 }

CThostFtdcInstrumentField CTraderSpi::GetInstrumentByLabel(const string& label) const
 {
     CThostFtdcInstrumentField tmp = {0};
     map<string, string>::const_iterator it1 = instrumentName2ID_.find(label);
     
     if (it1 == instrumentName2ID_.end()) return tmp;

     string id = it1->second;
     map<string, CThostFtdcInstrumentField>::const_iterator it2 = instrumentData_.find(id);
     if (it2 == instrumentData_.end()) return tmp;

     return it2->second;
 }

void CTraderSpi::SetInvestorInfo(const char* investor, const char* pwd)
{
    investor_ = investor;
    password_ = pwd;
}

void CTraderSpi::DisplayMsg(const wxString& msg, int type)
{
    if (winHandler_)
    {
        wxCommandEvent evt(EVENT_MAIN_WIN_SHOW_MSG);
        evt.SetInt(type);
        evt.SetString(msg);
        winHandler_->AddPendingEvent(evt);
    }
    else
    {
       msgDisplayer_.push(string(msg.c_str()));    
    }
       
    wxLogVerbose(msg);
}

void CTraderSpi::ReqUserLogin(const char* investor, const char* pwd)
{
    investor_ = investor;
    password_ = pwd;

    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, broker_.c_str());
    strcpy(req.UserID, investor_.c_str());
    strcpy(req.Password, password_.c_str());
    int iResult = pTraderApi_->ReqUserLogin(&req, ++reqID_);

    DisplayMsg(wxString::Format("--->>> 发送用户登录请求: %s", ((iResult == 0) ? "成功" : "失败")));
}

void CTraderSpi::ReqUserConnect(const char* front, const char* broker)
{
    // registerfront() takes non-const parameter, stupid.
    char front_addr[256] = {0};

    sprintf(front_addr,"tcp://%s",front);

    if (pTraderApi_) pTraderApi_->Release();
     
    // 初始化UserApi
    pTraderApi_ = CThostFtdcTraderApi::CreateFtdcTraderApi();			// 创建UserApi

    pTraderApi_->RegisterSpi(this);			// 注册事件类
    pTraderApi_->SubscribePublicTopic(THOST_TERT_RESTART);					// 注册公有流
    pTraderApi_->SubscribePrivateTopic(THOST_TERT_RESTART);					// 注册私有流

	pTraderApi_->RegisterFront(front_addr);							// connect
	pTraderApi_->Init();

    frontAddr_ = front;
    broker_ = broker;
}

// 报单，限价
void CTraderSpi::ReqOrderInsert(const char* instrument, double price,
        int director, int offset, int volume)
{
    CThostFtdcInputOrderField f;
    memset(&f, 0, sizeof(f));
    f.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	//1投机
    f.ContingentCondition = THOST_FTDC_CC_Immediately;//立即触发
    f.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
    f.IsAutoSuspend = 0;
    f.TimeCondition =  THOST_FTDC_TC_GFD;		//***立即完成1  当日有效3***
    f.VolumeCondition = THOST_FTDC_VC_AV;	//任意数量1  最小数量2  全部成交3
    f.MinVolume = 1;

    strcpy(f.InvestorID, investor_.c_str());
    strcpy(f.UserID, investor_.c_str());
    strcpy(f.BrokerID, broker_.c_str());

    strcpy(f.InstrumentID, instrument);	//合约

    if (price)
        f.OrderPriceType = THOST_FTDC_OPT_LimitPrice;		//***任意价1  限价2***
    else
        f.OrderPriceType = THOST_FTDC_OPT_AnyPrice;

    f.LimitPrice = price;				//***价格***

    if(director == 0)
        f.Direction = THOST_FTDC_D_Buy;			//买
    else
        f.Direction = THOST_FTDC_D_Sell;			//卖

    if(offset == 0)
        f.CombOffsetFlag[0] = THOST_FTDC_OF_Open;//开仓
    else if(offset == 1)
        f.CombOffsetFlag[0] = THOST_FTDC_OF_Close;	//平仓
    else
        f.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;	//平今

    f.VolumeTotalOriginal = volume;//数量

    sprintf(f.OrderRef, "%d", ++orderRef_);//OrderRef
    pTraderApi_->ReqOrderInsert(&f, reqID_++);	//报单

    OrderEntity entity;

    entity.orderRef_ = orderRef_ - 1;
    entity.reqID_    = reqID_ - 1;
    entity.orderStartTime_ = time(NULL);

    string key = wxString::Format("%s|%d|%d|%d", f.InstrumentID, sessionID_, frontID_, orderRef_ - 1).c_str();
    orderSend_[key] = entity;

    DisplayMsg(wxString::Format("报单:%s", f.OrderRef));
}

//报单-市价
void CTraderSpi::ReqOrderInsert(const char* instrument, int director, int offset, int volume)
{
    ReqOrderInsert(instrument, 0, director, offset, volume);
}

//撤单
void CTraderSpi::ReqOrderAction(const char* instrument, int session, int frontid, const char* orderref)
{
    CThostFtdcInputOrderActionField f;
    memset(&f, 0, sizeof(f));
    f.ActionFlag = THOST_FTDC_AF_Delete; 	//THOST_FTDC_AF_Modify
    strcpy(f.BrokerID, broker_.c_str());
    strcpy(f.InvestorID, investor_.c_str());

    strcpy(f.InstrumentID, instrument);
    f.SessionID = session;
    f.FrontID = frontid;
    strcpy(f.OrderRef, orderref);

    pTraderApi_->ReqOrderAction(&f, ++reqID_);

    OrderEntity entity;

    entity.orderRef_ = atoi(orderref);
    entity.reqID_    = reqID_ - 1;
    entity.orderStartTime_ = time(NULL);

    string key = wxString::Format("%s|%d|%d|%d", f.InstrumentID, session, frontid, atoi(orderref)).c_str();
    orderDelete_[key] = entity;
}


/*
 查询数据注意事项：
 1) 综合交易平台仅对查询进行流量限制，对交易指令没有限制。
 2) 如果有在途的查询，不允许发新的查询。
 3) 1 秒钟最多允许发送1个查询。
 4) 返回值“-2”表示“未处理请求超过许可数”
 5) “-3”表示“每秒发送请求数超过许可数”

*/
//查持仓
void CTraderSpi::ReqQryInvestorPosition()
{
    CThostFtdcQryInvestorPositionField f;
    memset(&f, 0, sizeof(f));
    strcpy(f.BrokerID, broker_.c_str());
    strcpy(f.InvestorID, investor_.c_str());
    pTraderApi_->ReqQryInvestorPosition(&f, ++reqID_);
}

//查持仓明细
void CTraderSpi::ReqQryInvestorPositionDetail()
{
    CThostFtdcQryInvestorPositionDetailField f;
    memset(&f, 0, sizeof(f));
    strcpy(f.BrokerID, broker_.c_str());
    strcpy(f.InvestorID, investor_.c_str());
    pTraderApi_->ReqQryInvestorPositionDetail(&f, ++reqID_);
}

//查资金
void CTraderSpi::ReqQryTradingAccount()
{
    CThostFtdcQryTradingAccountField f;
    memset(&f, 0, sizeof(f));
    strcpy(f.BrokerID, broker_.c_str());
    strcpy(f.InvestorID, investor_.c_str());
    int iResult = pTraderApi_->ReqQryTradingAccount(&f, ++reqID_);
    DisplayMsg(wxString::Format("--->>> 请求查询资金账户: %sbankSettingPanel_", ((iResult == 0) ? "成功" : "失败")));
}

//查签约银行
void CTraderSpi::ReqQryAccountregister()
{
    CThostFtdcQryAccountregisterField f;
    memset(&f, 0, sizeof(f));
    strcpy(f.BrokerID, broker_.c_str());
    strcpy(f.AccountID, investor_.c_str());

    pTraderApi_->ReqQryAccountregister(&f, ++reqID_);
}

//查银行帐户
void CTraderSpi::ReqQueryBankAccountMoneyByFuture(const char* bankID, const char* bankPWD, const char* accountPWD)
{
    CThostFtdcReqQueryAccountField f;
    memset(&f, 0, sizeof(f));

    strcpy(f.BankID, bankID);
    strcpy(f.BankBranchID, "0000");		//必须有
    strcpy(f.BankPassWord, bankPWD);

    strcpy(f.BrokerID, broker_.c_str());
    strcpy(f.AccountID, investor_.c_str());
    strcpy(f.Password, accountPWD);
    f.SecuPwdFlag = THOST_FTDC_BPWDF_BlankCheck;	//明文核对
    strcpy(f.CurrencyID, "RMB"); 					//币种：RMB-人民币 USD-美圆 HKD-港元

    pTraderApi_->ReqQueryBankAccountMoneyByFuture(&f, ++reqID_);
}

//银转功能
void CTraderSpi::ReqTransferByFuture(const char* bankID, const char* bankPWD, const char* accountPWD, double amount, bool f2B)
{
    CThostFtdcReqTransferField f;
    memset(&f, 0, sizeof(f));

    strcpy(f.BankID, bankID);
    strcpy(f.BankBranchID, "0000");		//必须有
    strcpy(f.BankPassWord, bankPWD);

    strcpy(f.BrokerID, broker_.c_str());
    strcpy(f.AccountID, investor_.c_str());
    strcpy(f.Password, accountPWD);
    f.SecuPwdFlag = THOST_FTDC_BPWDF_BlankCheck;	//明文核对
    strcpy(f.CurrencyID, "RMB"); 					//币种：RMB-人民币 USD-美圆 HKD-港元

    f.TradeAmount = amount;
    if(f2B)	//期转银
        pTraderApi_->ReqFromFutureToBankByFuture(&f, ++reqID_);
    else	//银转期
        pTraderApi_->ReqFromBankToFutureByFuture(&f, ++reqID_);
}

// event handlers.
void CTraderSpi::OnFrontConnected()
{
	//cerr << "--->>> " << "OnFrontConnected" << endl;
    logTarget_ = CreateWxLogTarget("AutoTraderApp_TraderSpiLog_");
#if wxCHECK_VERSION(2,9,1)
    wxLog::SetThreadActiveTarget(logTarget_);
#else
	wxLog::SetActiveTarget(logTarget_);
#endif
    DisplayMsg(wxString::Format("--->>>  OnFrontConnectedbankSettingPanel_"));
	///用户登录请求

    ReqUserLogin(investor_.c_str(), password_.c_str());
}

void CTraderSpi:: OnFrontDisconnected(int nReason)
{
	DisplayMsg(wxString::Format("--->>> OnFrontDisconnectedbankSettingPanel_"));
	DisplayMsg(wxString::Format("--->>> Reason = %dbankSettingPanel_", nReason));
}

void CTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// cerr << "--->>> " << "OnRspUserLogin" << endl;
	DisplayMsg(wxString::Format("--->>> OnRspUserLogin"));
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		// 保存会话参数
		frontID_ = pRspUserLogin->FrontID;
		sessionID_ = pRspUserLogin->SessionID;
		// int iNextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		// iNextOrderRef++;
		// sprintf(ORDER_REF, "%d", iNextOrderRef);
		///获取当前交易日
		// cerr << "--->>> 获取当前交易日 = " << pUserApi->GetTradingDay() << endl;
		// wxLogVerbose("--->>> 获取当前交易日 = %s", pTraderApi_->GetTradingDay());
		///投资者结算结果确认
		// ReqSettlementInfoConfirm();

        orderRef_ = atoi(pRspUserLogin->MaxOrderRef);
        CThostFtdcQrySettlementInfoConfirmField f;
        memset(&f, 0, sizeof(f));
        strcpy(f.BrokerID, broker_.c_str());
        strcpy(f.InvestorID, investor_.c_str());
        pTraderApi_->ReqQrySettlementInfoConfirm(&f, ++reqID_);
                
        DisplayMsg("login done", MAIN_WIN_MSG_LOGIN);
	}
}

void CTraderSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, broker_.c_str());
	strcpy(req.InvestorID, investor_.c_str());
	int iResult = pTraderApi_->ReqSettlementInfoConfirm(&req, ++reqID_);
	DisplayMsg(wxString::Format("--->>> 投资者结算结果确认: %s", ((iResult == 0) ? "成功" : "失败")));
}

void CTraderSpi::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm,
        CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if(this->IsErrorRspInfo(pRspInfo))
    {
        DisplayMsg(wxString::Format("查询结算确认错误"));
    }
    else
    {
        tradingDay_ = pTraderApi_->GetTradingDay();
        if((pSettlementInfoConfirm) && strcmp(pSettlementInfoConfirm->ConfirmDate, tradingDay_.c_str()) == 0)
        {            
            // 未确认结算，需要确认才能进行别的操作
            DisplayMsg(wxString::Format("结算未确认，正在请求结算确认..."));
              
            CThostFtdcSettlementInfoConfirmField f;
            memset(&f, 0, sizeof(f));
            strcpy(f.BrokerID, broker_.c_str());
            strcpy(f.InvestorID, investor_.c_str());

            pTraderApi_->ReqSettlementInfoConfirm(&f, ++reqID_);
        }
        else
        {            
            // 当天结算结果已确认            
            DisplayMsg(wxString::Format("结算已确认，查看结果中..."));

            CThostFtdcQrySettlementInfoField f;
            memset(&f, 0, sizeof(f));
            strcpy(f.BrokerID, broker_.c_str());
            strcpy(f.InvestorID, investor_.c_str());

            wxThread::Sleep(1000);// CTP requires one query per second.
            pTraderApi_->ReqQrySettlementInfo(&f, ++reqID_);
        }
    }
}

//查结算信息
void CTraderSpi::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField* pSettlementInfo,
                                   CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if(pSettlementInfo)
    {
        settlementInfo_ += pSettlementInfo->Content;
     
        wxString msg = wxString::Format("结算信息：交易日（%s）, 结算编号(%d), 经纪公司(%s), 投资者编号(%s),\
                                        序号(%d), 内容(%d)", pSettlementInfo->TradingDay, pSettlementInfo->SettlementID,
                                        pSettlementInfo->BrokerID, pSettlementInfo->InvestorID, pSettlementInfo->InvestorID,
                                        pSettlementInfo->SequenceNo, pSettlementInfo->Content);

        DisplayMsg(msg);
    }
        
    if (bIsLast)
    {
        wxThread::Sleep(1000);// CTP requires one query per second.
        ReqQryInstrument();
    }
}

void CTraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// cerr << "--->>> " << "OnRspSettlementInfoConfirm" << endl;
	DisplayMsg(wxString::Format("--->>> OnRspSettlementInfoConfirm"));
	if (bIsLast)
	{
        wxThread::Sleep(1000);// CTP requires one query per second.
        ReqQryInstrument();
	}
}

void CTraderSpi::ReqQryInstrument()
{
    // wxThread::Sleep(500);
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	// strcpy(req.InstrumentID, INSTRUMENT_ID);
	int iResult = pTraderApi_->ReqQryInstrument(&req, ++reqID_);
	// cerr << "--->>> 请求查询合约: " << ((iResult == 0) ? "成功" : "失败") << endl;

    wxString msg;

    if (iResult == -1) msg = "网络连接失败";
    else if (iResult == -2) msg = "未处理请求超过许可数";
    else if (iResult == -3) msg = "每秒发送请求数超过许可数";
    else msg = "成功";

	DisplayMsg(wxString::Format("--->>> 请求查询合约: %s", msg.c_str()));
}

void CTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// cerr << "--->>> " << "OnRspQryInstrument" << endl;
	DisplayMsg(wxString::Format("--->>> OnRspQryInstrument"));

    if (IsErrorRspInfo(pRspInfo)) return;

    wxString label = wxString::Format("合约号:%s,合约名称:%s,交易所:%s",
        pInstrument->InstrumentID, pInstrument->InstrumentName, pInstrument->ExchangeID);

    DisplayMsg(label, MAIN_WIN_MSG_INSTRUMENT);

    instrumentName2ID_[string(label.c_str())] = pInstrument->InstrumentID;
    instrumentData_[pInstrument->InstrumentID] = *pInstrument;

	if (bIsLast)
	{
		///请求查询合约
        // wxThread::Sleep(1000);// CTP requires one query per second.
		// ReqQryTradingAccount();
	}
}

void CTraderSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// cerr << "--->>> " << "OnRspQryTradingAccount" << endl;
	DisplayMsg(wxString::Format("--->>> OnRspQryTradingAccount"));
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///请求查询投资者持仓
		// ReqQryInvestorPosition();

        //静态权益=上日结算-出金金额+入金金额
        double preBalance = pTradingAccount->PreBalance
            - pTradingAccount->Withdraw + pTradingAccount->Deposit;
        //动态权益=静态权益+ 平仓盈亏+ 持仓盈亏- 手续费
        double current = preBalance + pTradingAccount->CloseProfit
            + pTradingAccount->PositionProfit - pTradingAccount->Commission;

        wxString format = _T("\n可用资金  平仓盈亏  持仓盈亏  静态权益  动态权益  占用保证金  冻结资金  风险度\n\
            %f  %f  %f  %f  %f  %f  %f  %f");

        wxString msg = wxString::Format(format, pTradingAccount->Available,
                pTradingAccount->CloseProfit, pTradingAccount->PositionProfit,
                preBalance, current, pTradingAccount->CurrMargin,
                (pTradingAccount->FrozenMargin + pTradingAccount->FrozenCommission),
                (current == 0? 0 : (pTradingAccount->CurrMargin / current * 100)));

        DisplayMsg(msg);
    }
}

void CTraderSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// cerr << "--->>> " << "OnRspQryInvestorPosition" << endl;
	DisplayMsg(wxString::Format("--->>> OnRspQryInvestorPosition"));

    if (pInvestorPosition)
    {
        string key = wxString::Format("%s|%d|%d", pInvestorPosition->InstrumentID, pInvestorPosition->PosiDirection, pInvestorPosition->PositionDate).c_str();

        positionData_[key] = *pInvestorPosition;
    }

    if (bIsLast)
    {
        wxString format = _T("%s  %s  %s  %d  %f  %f\n");
        wxString msg = _T("\n持仓：\n合约  买卖  今昨  持仓  成本  持仓盈亏\n");
        map<string, CThostFtdcInvestorPositionField>::iterator it = positionData_.begin();

        while (it != positionData_.end())
        {
            CThostFtdcInvestorPositionField f = it->second;
            wxString str = wxString::Format(format, 
                    f.InstrumentID,
                    (f.PosiDirection == THOST_FTDC_PD_Long? _T("多") : _T("空")),
                    (f.PositionDate == THOST_FTDC_PSD_History? _T("昨仓") : _T("今仓")),
                    f.Position,
                    (f.PositionCost),// / instrumentData_[pInvestorPosition->InstrumentID].VolumeMultiple
                    f.PositionProfit);

            msg += str;
            ++it;
        }

        DisplayMsg(msg);
    }
}

void CTraderSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField* pInvestorPositionDetail,
										CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if(pInvestorPositionDetail)
    {
        positionDetailData_[atoi(pInvestorPositionDetail->TradeID)] = *pInvestorPositionDetail;
    }

    if(bIsLast)
    {
        positionData_.clear();
        wxString format = _T("%s  %s  %s  %s  %d  %f  %f\n");
        wxString msg = _T("\n持仓明细:\n成交编号  合约  买卖  今昨  数量  开仓价  持仓盈亏\n");
        map<int, CThostFtdcInvestorPositionDetailField>::iterator it = positionDetailData_.begin();

        while (it != positionDetailData_.end())
        {
            CThostFtdcInvestorPositionDetailField f = it->second;
            wxString str = wxString::Format(format,
                       f.TradeID, f.InstrumentID,
                       (f.Direction == THOST_FTDC_D_Buy? _T("买"):_T("卖")),
                       (strcmp(f.OpenDate, tradingDay_.c_str()) == 0? _T("今仓"):_T("昨仓")),
                       f.Volume, f.OpenPrice,
                       f.PositionProfitByDate);

            msg += str;
            ++it;

            // TODO

            /*
            //持仓汇总
            double multi = instrumentData_[f.InstrumentID].VolumeMultiple;
            string positionID = wxString::Format("%s|%d|%d", f.InstrumentID,
                    f.Direction, (strcmp(f.OpenDate, tradingDay_.c_str())==0)).c_str();

            map<string, CThostFtdcInvestorPositionField>::iterator iter = positionData_.find(positionID.c_str());
            CThostFtdcInvestorPositionField pf;
            if(iter == positionData_.end())
            {
                memset(&pf, 0, sizeof(pf));
                strcpy(pf.InstrumentID, f.InstrumentID);
                pf.PosiDirection = (f.Direction == THOST_FTDC_D_Buy ? THOST_FTDC_PD_Long: THOST_FTDC_PD_Short);
                pf.PositionDate = (strcmp(f.OpenDate, tradingDay_.c_str()) == 0? THOST_FTDC_PSD_Today : THOST_FTDC_PSD_History);
                pf.Position = 0;//f.Volume;
                pf.PositionCost = 0;//f.OpenPrice;
                pf.PositionProfit = 0;//f.PositionProfitByDate
                pf.TodayPosition = 0;
                pf.YdPosition = 0;
            }
            else
            {
                pf = positionData_[positionID];
            }

            pf.Position  += f.Volume;

            if(pf.PositionDate == THOST_FTDC_PSD_History)
            {
                pf.YdPosition += f.Volume;
                pf.PositionCost += f.Volume * f.LastSettlementPrice * multi;
                //持仓盈亏需要自己运算detail始终为0
                pf.PositionProfit += (f.Direction == THOST_FTDC_PD_Long? 1 : -1) *
                                     (dicTick[f.InstrumentID].LastPrice - f.LastSettlementPrice) * f.Volume * multi;
            }
            else
            {
                pf.TodayPosition += f.Volume;
                pf.PositionCost += f.Volume * f.OpenPrice * multi;
                //持仓盈亏需要自己运算detail始终为0
                pf.PositionProfit += (f.Direction == THOST_FTDC_PD_Long? 1 : -1) *
                                     (dicTick[f.InstrumentID].LastPrice - f.OpenPrice) * f.Volume * multi;
            }
            

            positionData_[positionID] = pf;
            */
        }

        // show(msg, 1);

        DisplayMsg(msg);

        //显示持仓汇总
        format = "%s  %s  %s  %f  %f  %f\n";
        msg = "\n合约  买卖  今昨  持仓  成本  持仓盈亏\n";

        map<string, CThostFtdcInvestorPositionField>::iterator iter = positionData_.begin();
        while (iter != positionData_.end())
        {
            CThostFtdcInvestorPositionField& f = iter->second;
            wxString str = wxString::Format(format,
                       f.InstrumentID, (f.PosiDirection == THOST_FTDC_PD_Long?"多":"空"),
                       (f.PositionDate == THOST_FTDC_PSD_History?"昨仓":"今仓"), f.Position,
                       (f.PositionCost / f.Position), // / instrumentData_[f.InstrumentID].VolumeMultiple),
                       f.PositionProfit);

            msg += str;
            ++it;
        }

        DisplayMsg(msg);
    }
}

void CTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	DisplayMsg(wxString::Format("--->>> 报单错误:%s", pRspInfo->ErrorMsg));
	// cerr << "--->>> " << "OnRspOrderInsert" << endl;
	// IsErrorRspInfo(pRspInfo);
}

void CTraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    DisplayMsg(wxString::Format("--->>> OnRspOrderAction"));
    if (pInputOrderAction)
    {  
        wxString key = wxString::Format(_T("%s|%d|%d|%s,撤单状态:%s"), pInputOrderAction->InstrumentID, 
            pInputOrderAction->SessionID, pInputOrderAction->FrontID, pInputOrderAction->OrderRef, pRspInfo->ErrorMsg);
        DisplayMsg(key);
    }

    // cerr << "--->>> " << "OnRspOrderAction" << endl;
    // IsErrorRspInfo(pRspInfo);
}

///报单通知
void CTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    // cerr << "--->>> " << "OnRtnOrder"  << endl;
	DisplayMsg(wxString::Format("--->>> OnRtnOrder"));

    string key = wxString::Format("%s|%d|%d|%s", pOrder->InstrumentID, pOrder->SessionID, pOrder->FrontID, pOrder->OrderRef).c_str();
        
    wxString msg = wxString::Format("报单编号:%s@@经纪公司编号:%d@#@状态:%s",
            key.c_str(), pOrder->BrokerOrderSeq, pOrder->StatusMsg);

    DisplayMsg(msg, MAIN_WIN_MSG_ORDER);

    std::map<std::string, OrderEntity>::iterator it = orderSend_.find(key.c_str());
    if (it != orderSend_.end())
    {
        time_t t2 = time(NULL);
        OrderEntity entity = orderSend_[key];

        // TODO calc time span.
        orderSend_.erase(it);
    }
}

///成交通知
void CTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    DisplayMsg(wxString::Format("成交编号:%s, broker编号%d, 成交时间%s", pTrade->TradeID, pTrade->BrokerOrderSeq, pTrade->TradeTime));
}

void CTraderSpi::OnRspQryAccountregister(CThostFtdcAccountregisterField* pAccountregister,
                                    CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if(IsErrorRspInfo(pRspInfo))
    {
        DisplayMsg(wxString::Format(wxString::Format("查签约银行错误:%s", pRspInfo->ErrorMsg)));
    }
    else if(pAccountregister)
    {
        string bank = "";
        if(pAccountregister->BankID[0] == THOST_FTDC_BF_ABC)
            bank = "农业银行";
        else if(pAccountregister->BankID[0] == THOST_FTDC_BF_BC)
            bank = "中国银行";
        else if(pAccountregister->BankID[0] == THOST_FTDC_BF_BOC)
            bank = "交通银行";
        else if(pAccountregister->BankID[0] == THOST_FTDC_BF_CBC)
            bank = "建设银行";
        else if(pAccountregister->BankID[0] == THOST_FTDC_BF_ICBC)
            bank = "工商银行";
        else if(pAccountregister->BankID[0] == THOST_FTDC_BF_Other)
            bank = "其他银行";

        string bankID = string(pAccountregister->BankAccount);
        bankID = bankID.substr(strlen(pAccountregister->BankAccount)-4, 4);
        DisplayMsg(wxString::Format(wxString::Format("%d:%s:%s", pAccountregister->BankID[0], bank, bankID)));
    }
}

void CTraderSpi::OnRtnQueryBankBalanceByFuture(CThostFtdcNotifyQueryAccountField* pNotifyQueryAccount)
{
    DisplayMsg(wxString::Format("可用余额:%s, 可取余额:%s", pNotifyQueryAccount->BankUseAmount,
             pNotifyQueryAccount->BankFetchAmount));
}

void CTraderSpi::OnHeartBeatWarning(int nTimeLapse)
{
    DisplayMsg(wxString::Format("--->>> OnHeartBeatWarning"));
    DisplayMsg(wxString::Format("--->>> nTimerLapse = %d", nTimeLapse));
}

void CTraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    DisplayMsg(wxString::Format("--->>> OnRspError"));
	IsErrorRspInfo(pRspInfo);
}

bool CTraderSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (bResult)
		DisplayMsg(wxString::Format("--->>> ErrorID=%d, ErrorMsg=%s", pRspInfo->ErrorID, pRspInfo->ErrorMsg));
	return bResult;
}

bool CTraderSpi::IsTradingOrder(CThostFtdcOrderField *pOrder)
{
	return ((pOrder->OrderStatus != THOST_FTDC_OST_PartTradedNotQueueing) &&
			(pOrder->OrderStatus != THOST_FTDC_OST_Canceled) &&
			(pOrder->OrderStatus != THOST_FTDC_OST_AllTraded));
}

