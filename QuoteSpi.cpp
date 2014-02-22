#include "QuoteSpi.h"
using namespace std;

QuoteSpi::QuoteSpi(wxEvtHandler* handler, map<string, CThostFtdcInstrumentField>& validInstrument)
	:pUserApi(NULL)
	,mainWin_(handler)
	,instrumentData_(validInstrument)
{
}

void QuoteSpi::DisplayMsg(const wxString& msg, int type)
{
    if (mainWin_)
    {
        wxCommandEvent evt(EVENT_MAIN_WIN_SHOW_MSG);
        evt.SetInt(type);
        evt.SetString(msg);
        mainWin_->AddPendingEvent(evt);
    }
	else
	{
        wxLogVerbose(msg);
	}
}

void QuoteSpi::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void QuoteSpi::OnFrontConnected()
{
    ReqUserLogin(investor_, password_);
}

void QuoteSpi::OnFrontDisconnected(int nReason)
{
	DisplayMsg("行情断开");
}

void QuoteSpi::OnHeartBeatWarning(int nTimeLapse)
{
}

void QuoteSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                           CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if(IsErrorRspInfo(pRspInfo))
    {
	    DisplayMsg(wxString::Format("登录错误:%s", pRspInfo->ErrorMsg));
	}
    else
    {
		tradingDay_ = pRspUserLogin->TradingDay;
    	DisplayMsg("行情登录成功!"); 	//
    }
}

void QuoteSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

void QuoteSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
}

//深度行情响应
void QuoteSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData)
{
	if (pDepthMarketData) SaveTickInfo(pDepthMarketData);
}

CThostFtdcDepthMarketDataField QuoteSpi::GetMarketDataByInstrumentId(const char* id) const
{
	CThostFtdcDepthMarketDataField data;
	memset(&data, 0, sizeof(data));

	{
        wxMutexLocker lock(tickLocker_);

	    map<string, CThostFtdcDepthMarketDataField>::const_iterator it = tickData_.find(id);
	    if (it == tickData_.end()) return data;

	    return it->second;
	}
}

//tick写入文本
void QuoteSpi::SaveTickInfo(CThostFtdcDepthMarketDataField* f)
{
	wxMutexLocker lock(tickLocker_);

    //合法合约//合法数据
	map<string, CThostFtdcInstrumentField>::const_iterator it = instrumentData_.find(f->InstrumentID);
    if(it != instrumentData_.end() && f->LastPrice < f->UpperLimitPrice)
    {
        CThostFtdcInstrumentField instField;
        memset(&instField, 0, sizeof(instField));
        instField = it->second;	//合约信息

        strcpy(f->TradingDay, tradingDay_.c_str());			//日期
        strcpy(f->ExchangeID, instField.ExchangeID);	//交易所

        if(f->AskPrice1 > f->UpperLimitPrice)	//单边有挂边的情况
            f->AskPrice1 = f->LastPrice;
        if(f->BidPrice1 > f->UpperLimitPrice)
            f->BidPrice1 = f->LastPrice;

        if(instField.ExchangeID == "CZCE") //成交额与均价
            f->Turnover *= instField.VolumeMultiple;
        else
            f->AveragePrice /= instField.VolumeMultiple;

        tickData_[string(f->InstrumentID)] = *f;	//更新Tick

		return;
    }
}
