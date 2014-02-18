// testTraderApi.cpp : 定义控制台应用程序的入口点。
//
#include <wx/wxprec.h>
#ifndef WX_PRECOMP    
#include <wx/wx.h>
#endif

#include "main.h"
#include "Defines.h"
#include "StlUtil.h"
#include "ConfigReader.h"

#include "LogDisplayer.h"

#include <time.h>
#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

DEFINE_EVENT_TYPE(EVENT_MAIN_WIN_SHOW_MSG)

IMPLEMENT_APP(AutoTraderApp);

wxLog* CreateWxLogTarget(const wxString& file_path)
{
    time_t t = time(NULL);
    struct tm* time_info = localtime(&t);
    char buf[128];

    strftime(buf, sizeof(buf), " %Y_%m_%d_%H_%M_%S", time_info);
    wxString name = file_path + buf + ".log";

    FILE* log_file = fopen(name.c_str(), "w+");
    assert(log_file);

    return new wxLogStderr(log_file);
}

AutoTraderApp::AutoTraderApp()
    :logTarget_(NULL)
{

}

AutoTraderApp::~AutoTraderApp()
{
    // if (logTarget_) delete logTarget_;
}

bool AutoTraderApp::OnInit()
{    
    wxString name = "AutoTraderLog_main_";

    logTarget_ = CreateWxLogTarget(name);
    wxLog::SetActiveTarget(logTarget_);
    wxLog::SetVerbose();

    FrameMain *frame = new FrameMain("期货交易", wxPoint(50, 50), wxSize(450, 340));    
    frame->Show( true );    
    return true;
}

enum
{
    ID_TIMER_REFRESH,
    ID_TIMER_ORDER,
    
    ID_BTN_LOGIN,
    ID_BTN_ORDER_INSERT,
    ID_BTN_CONTNUE_ORDER,
    ID_BTN_POSITION_QRY,
    ID_BTN_REQ_QRY_ACCOUNT,
    ID_BTN_QRY_DETAIL,
    ID_BTN_QRY_BANK,
    ID_BTN_FEATURE_BANK,
    ID_BTN_BANK_FEATURE,
    ID_BTN_QRY_BANKS,
    ID_BTN_ORDER_ACTION
};

BEGIN_EVENT_TABLE(FrameMain, wxFrame)    
    EVT_BUTTON(ID_BTN_LOGIN, FrameMain::OnButtonLoginClick)
    EVT_BUTTON(ID_BTN_ORDER_INSERT, FrameMain::OnButtonOrderInsertClick)
    EVT_BUTTON(ID_BTN_CONTNUE_ORDER, FrameMain::OnButtonOrderContinueClick)
    EVT_BUTTON(ID_BTN_POSITION_QRY, FrameMain::OnButtonQryPositionClick)
    EVT_BUTTON(ID_BTN_REQ_QRY_ACCOUNT, FrameMain::OnButtonReqQryAccountClick)
    EVT_BUTTON(ID_BTN_QRY_DETAIL, FrameMain::OnButtonQryDetailClick)
    EVT_BUTTON(ID_BTN_QRY_BANK, FrameMain::OnButtonQryBankClick)
    EVT_BUTTON(ID_BTN_FEATURE_BANK, FrameMain::OnButtonFuture2BankClick)
    EVT_BUTTON(ID_BTN_BANK_FEATURE, FrameMain::OnButtonBank2FutureClick)
    EVT_BUTTON(ID_BTN_QRY_BANKS, FrameMain::OnButtonQryBanksClick)
    EVT_BUTTON(ID_BTN_ORDER_ACTION, FrameMain::OnButtonOrderActionClick)

    EVT_TIMER(ID_TIMER_ORDER, FrameMain::OnOrderTimer)

    EVT_COMBOBOX(wxID_ANY, FrameMain::OnComboBoxInstrumentSelected)

    EVT_COMMAND(wxID_ANY, EVENT_MAIN_WIN_SHOW_MSG, FrameMain::OnFeaturesTradingMsg)
END_EVENT_TABLE()

FrameMain::FrameMain(const wxString& title, const wxPoint& pos, const wxSize& size)        
    : wxFrame(NULL, wxID_ANY, title, pos, size), trader_(this)
{
    SetClientSize(wxSize(710,452));
    wxBoxSizer* top_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(top_sizer);

    // first layer.
    wxPanel* Panel1 = new wxPanel(this, wxID_ANY, wxPoint(24,8), wxSize(624,40), wxNO_BORDER|wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    wxBoxSizer* top1_sizer = new wxBoxSizer(wxHORIZONTAL);
    Panel1->SetSizer(top1_sizer);
    top_sizer->Add(Panel1, 0, wxEXPAND | wxALL, 4);

    wxStaticText* front_static = new wxStaticText(Panel1, wxID_ANY, _("Front"));
    top1_sizer->Add(front_static, 0, wxEXPAND | wxALL, 4);
    ChoiceFront_ = new wxChoice(Panel1, wxID_ANY, wxPoint(8,8), wxSize(134,22), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));

    /*
    ChoiceFront_->SetSelection( ChoiceFront_->Append(_("模拟")) );
    ChoiceFront_->Append(_("上海电信1"));
    ChoiceFront_->Append(_("上海联通1"));
    ChoiceFront_->Append(_("上海电信2"));
    ChoiceFront_->Append(_("上海联通2"));
    ChoiceFront_->Append(_("杭州电信1"));
    ChoiceFront_->Append(_("杭州联通1"));
    ChoiceFront_->Append(_("杭州电信2"));
    ChoiceFront_->Append(_("杭州联通2"));
    ChoiceFront_->Append(_("北京联通1"));
    ChoiceFront_->Append(_("北京联通2"));
    ChoiceFront_->Append(_("深圳电信1"));
    ChoiceFront_->Append(_("深圳电信2"));
    ChoiceFront_->Append(_("盘后查询"));
    ChoiceFront_->Append(_("其它"));
    */

    top1_sizer->Add(ChoiceFront_, 1, wxEXPAND | wxALL, 4);

    /*
    front_static_ = new wxStaticText(Panel1, wxID_ANY, _("Front"));
    TextCtrlOther_ = new wxTextCtrl(Panel1, wxID_ANY, _(""), wxPoint(112,8), wxSize(130,22), 0, wxDefaultValidator, _T("ID_TEXTCTRL5"));
    front_static_->Hide();
    TextCtrlOther_->Hide();

    top1_sizer->Add(front_static_, 0, wxEXPAND | wxALL, 4);
    top1_sizer->Add(TextCtrlOther_, 1, wxEXPAND | wxALL, 4);

    broker_static_ = new wxStaticText(Panel1, wxID_ANY, _("Broker"));
    TextCtrlBroker_ = new wxTextCtrl(Panel1, wxID_ANY, _(""), wxPoint(312,8), wxSize(72,22), 0, wxDefaultValidator, _T("ID_TEXTCTRL6"));
    broker_static_->Hide();
    TextCtrlBroker_->Hide();

    top1_sizer->Add(broker_static_, 0, wxEXPAND | wxALL, 4);
    top1_sizer->Add(TextCtrlBroker_, 1, wxEXPAND | wxALL, 4);
    */

    wxStaticText* investor_static = new wxStaticText(Panel1, wxID_ANY, _("帐号"));
    TextCtrlInvestor_ = new wxTextCtrl(Panel1, wxID_ANY, _(""), wxPoint(384,8), wxSize(88,22), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    wxStaticText* investor_pwd_static = new wxStaticText(Panel1, wxID_ANY, _("密码"));
    TextCtrlPassWord_ = new wxTextCtrl(Panel1, wxID_ANY, wxEmptyString, wxPoint(472,8), wxSize(72,22), wxTE_PASSWORD, wxDefaultValidator, _T("ID_TEXTCTRL2"));

    ButtonLogin_ = new wxButton(Panel1, ID_BTN_LOGIN, _("登录"), wxPoint(544,8), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));

    top1_sizer->Add(investor_static, 0, wxEXPAND | wxALL, 4);
    top1_sizer->Add(TextCtrlInvestor_, 1, wxEXPAND | wxALL, 4);
    top1_sizer->Add(investor_pwd_static, 0, wxEXPAND | wxALL, 4);
    top1_sizer->Add(TextCtrlPassWord_, 1, wxEXPAND | wxALL, 4);
    top1_sizer->Add(ButtonLogin_, 0, wxEXPAND | wxALL, 4);

    // second layer.
    OrderSettingPanel_ = new wxPanel(this, wxID_ANY, wxPoint(24,56), wxSize(624,120), wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    wxBoxSizer* top2_sizer = new wxBoxSizer(wxVERTICAL);
    OrderSettingPanel_->SetSizer(top2_sizer);
    top_sizer->Add(OrderSettingPanel_, 0 , wxEXPAND | wxALL, 4);

    wxBoxSizer* top2_sizer2 = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* instru_static = new wxStaticText(OrderSettingPanel_, wxID_ANY, _("合约"));
    ComboBoxInstrument_ = new wxComboBox(OrderSettingPanel_, wxID_ANY, wxEmptyString, wxPoint(8,8), wxSize(162,22), 0, 0, wxCB_SORT | wxCB_READONLY, wxDefaultValidator, _T("ID_COMBOBOX1"));
    wxStaticText* instru_price_static = new wxStaticText(OrderSettingPanel_, wxID_ANY, _("价格"));
    SpinCtrlPrice_ = new wxSpinCtrl(OrderSettingPanel_, wxID_ANY, _T("0"), wxPoint(128,8), wxSize(88,22), 0, 0, 10000000, 0, _T("ID_SPINCTRL1"));
    SpinCtrlPrice_->SetValue(_T("0"));
    top2_sizer2->Add(instru_static, 0, wxEXPAND | wxALL, 4);
    top2_sizer2->Add(ComboBoxInstrument_, 2, wxEXPAND | wxALL, 4);
    top2_sizer2->Add(instru_price_static, 0, wxEXPAND | wxALL, 4);
    top2_sizer2->Add(SpinCtrlPrice_, 1, wxEXPAND | wxALL, 4);

    CheckBoxPriceType_ = new wxCheckBox(OrderSettingPanel_, wxID_ANY, _("跟盘价"), wxPoint(216,8), wxSize(56,24), 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBoxPriceType_->SetValue(true);
    top2_sizer2->Add(CheckBoxPriceType_, 0, wxEXPAND | wxALL, 4);

    ChoiceDirector_ = new wxChoice(OrderSettingPanel_, wxID_ANY, wxPoint(272,8), wxSize(56,22), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    ChoiceDirector_->SetSelection( ChoiceDirector_->Append(_("买")) );
    ChoiceDirector_->Append(_("卖"));
    top2_sizer2->Add(ChoiceDirector_, 0, wxEXPAND | wxALL, 4);

    ChoiceOffset_ = new wxChoice(OrderSettingPanel_, wxID_ANY, wxPoint(336,8), wxSize(72,22), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
    ChoiceOffset_->SetSelection(ChoiceOffset_->Append(_("开仓")) );
    ChoiceOffset_->Append(_("平仓"));
    ChoiceOffset_->Append(_("平今"));
    top2_sizer2->Add(ChoiceOffset_, 0, wxEXPAND | wxALL, 4);

    SpinCtrlVolume_ = new wxSpinCtrl(OrderSettingPanel_, wxID_ANY, _T("1"), wxPoint(416,8), wxSize(72,22), 0, 1, 10000, 1, _T("ID_SPINCTRL2"));
    SpinCtrlVolume_->SetValue(_T("1"));
    top2_sizer2->Add(SpinCtrlVolume_, 0, wxEXPAND | wxALL, 4);

    wxButton* ButtonOrderInsert = new wxButton(OrderSettingPanel_, ID_BTN_ORDER_INSERT, _("下单"), wxPoint(496,8), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    top2_sizer2->Add(ButtonOrderInsert, 0, wxEXPAND | wxALL, 4);

    top2_sizer->Add(top2_sizer2, 0, wxEXPAND | wxALL, 4);

    wxBoxSizer* top2_sizer3 = new wxBoxSizer(wxHORIZONTAL);

    wxButton* ButtonQryPosition = new wxButton(OrderSettingPanel_, ID_BTN_POSITION_QRY, _("查持仓"), wxPoint(8,40), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    top2_sizer3->Add(ButtonQryPosition, 0, wxEXPAND | wxALL, 4);
    wxButton* ButtonReqQryAccount = new wxButton(OrderSettingPanel_, ID_BTN_REQ_QRY_ACCOUNT, _("查资金"), wxPoint(96,40), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    top2_sizer3->Add(ButtonReqQryAccount, 0, wxEXPAND | wxALL, 4);
    wxButton* ButtonQryDetail = new wxButton(OrderSettingPanel_, ID_BTN_QRY_DETAIL, _("查持仓明细"), wxPoint(184,40), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON11"));
    top2_sizer3->Add(ButtonQryDetail, 0, wxEXPAND | wxALL, 4);

    wxStaticText* StaticText1 = new wxStaticText(OrderSettingPanel_, wxID_ANY, _("每秒发单"), wxPoint(280,48), wxSize(48,14), 0, _T("ID_STATICTEXT1"));
    top2_sizer3->Add(StaticText1, 0, wxEXPAND | wxALL, 4);
    SpinCtrlTicks_ = new wxSpinCtrl(OrderSettingPanel_, wxID_ANY, _T("6"), wxPoint(328,40), wxSize(56,22), 0, 1, 1000, 6, _T("ID_SPINCTRL3"));
    SpinCtrlTicks_->SetValue(_T("6"));
    top2_sizer3->Add(SpinCtrlTicks_, 0, wxEXPAND | wxALL, 4);

    wxStaticText* StaticText2 = new wxStaticText(OrderSettingPanel_, wxID_ANY, _("笔, 持续"), wxPoint(384,48), wxSize(44,16), 0, _T("ID_STATICTEXT2"));
    top2_sizer3->Add(StaticText2, 0, wxEXPAND | wxALL, 4);

    SpinCtrlSeconds_ = new wxSpinCtrl(OrderSettingPanel_, wxID_ANY, _T("3"), wxPoint(432,40), wxSize(48,22), 0, 1, 1000, 3, _T("ID_SPINCTRL4"));
    SpinCtrlSeconds_->SetValue(_T("3"));
    top2_sizer3->Add(SpinCtrlSeconds_, 0, wxEXPAND | wxALL, 4);
    wxStaticText* StaticText3 = new wxStaticText(OrderSettingPanel_, wxID_ANY, _("秒"), wxPoint(480,48), wxSize(16,16), wxALIGN_LEFT, _T("ID_STATICTEXT3"));
    top2_sizer3->Add(StaticText3, 0, wxEXPAND | wxALL, 4);
        
    wxButton* ButtonOrderContinue = new wxButton(OrderSettingPanel_, ID_BTN_CONTNUE_ORDER, _("连续报单"), wxPoint(496,40), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    top2_sizer3->Add(ButtonOrderContinue, 0, wxEXPAND | wxALL, 4);

    top2_sizer->Add(top2_sizer3, 0, wxEXPAND | wxALL, 4);
  
    // third layer.
    wxPanel* Panel4 = new wxPanel(this, wxID_ANY, wxPoint(24,136), wxSize(584,32), wxTAB_TRAVERSAL, _T("ID_PANEL4"));
    wxBoxSizer* top4_sizer = new wxBoxSizer(wxHORIZONTAL);
    Panel4->SetSizer(top4_sizer);
    top_sizer->Add(Panel4, 0, wxEXPAND | wxALL, 4);
 
    ChoiceBrokerSeq_ = new wxChoice(Panel4, wxID_ANY, wxPoint(8,8), wxSize(472,22), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE5"));
    top4_sizer->Add(ChoiceBrokerSeq_, 1, wxEXPAND | wxALL, 4);
    wxButton* ButtonOrderAction = new wxButton(Panel4, ID_BTN_ORDER_ACTION, _("撤单"), wxPoint(496,8), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
    top4_sizer->Add(ButtonOrderAction, 0, wxEXPAND | wxALL, 4);


    // forth layer.
    bankSettingPanel_ = new wxPanel(this, wxID_ANY, wxPoint(24,176), wxSize(584,120), wxTAB_TRAVERSAL, _T("ID_bankSettingPanel_"));
    wxBoxSizer* top3_sizer = new wxBoxSizer(wxVERTICAL);
    bankSettingPanel_->SetSizer(top3_sizer);
    top_sizer->Add(bankSettingPanel_, 0, wxEXPAND | wxALL, 4);

    wxBoxSizer* top3_sizer1 = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* StaticText4 = new wxStaticText(bankSettingPanel_, wxID_ANY, _("银行"), wxPoint(8,16), wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    top3_sizer1->Add(StaticText4, 0, wxEXPAND | wxALL, 4);
    ChoiceBanks_ = new wxChoice(bankSettingPanel_, wxID_ANY, wxPoint(32,8), wxSize(368,22), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
    top3_sizer1->Add(ChoiceBanks_, 0, wxEXPAND | wxALL, 4);
    
    wxButton* ButtonQryBanks = new wxButton(bankSettingPanel_, ID_BTN_QRY_BANKS, _("查银行"), wxPoint(408,8), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON10"));
    top3_sizer1->Add(ButtonQryBanks, 0, wxEXPAND | wxALL, 4);
    wxButton* ButtonFuture2Bank = new wxButton(bankSettingPanel_, ID_BTN_FEATURE_BANK, _("期转银"), wxPoint(496,8), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
    top3_sizer1->Add(ButtonFuture2Bank, 0, wxEXPAND | wxALL, 4);

    top3_sizer->Add(top3_sizer1, 0, wxEXPAND | wxALL, 4);

    wxBoxSizer* top3_sizer2 = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* StaticText5 = new wxStaticText(bankSettingPanel_, wxID_ANY, _("资金密码"), wxPoint(8,48), wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    top3_sizer2->Add(StaticText5, 0, wxEXPAND | wxALL, 4);
    TextCtrlAccountPwd_ = new wxTextCtrl(bankSettingPanel_, wxID_ANY, wxEmptyString, wxPoint(56,40), wxSize(104,22), wxTE_PASSWORD, wxDefaultValidator, _T("ID_TEXTCTRL3"));
    top3_sizer2->Add(TextCtrlAccountPwd_, 0, wxEXPAND | wxALL, 4);
    wxStaticText* StaticText6 = new wxStaticText(bankSettingPanel_, wxID_ANY, _("银行密码"), wxPoint(168,48), wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    top3_sizer2->Add(StaticText6, 0, wxEXPAND | wxALL, 4);
    TextCtrlBankPwd_ = new wxTextCtrl(bankSettingPanel_, wxID_ANY, wxEmptyString, wxPoint(216,40), wxDefaultSize, wxTE_PASSWORD, wxDefaultValidator, _T("ID_TEXTCTRL4"));
    top3_sizer2->Add(TextCtrlBankPwd_, 0, wxEXPAND | wxALL, 4);

    wxButton* ButtonQryBank = new wxButton(bankSettingPanel_, ID_BTN_QRY_BANK, _("查银行帐户"), wxPoint(320,40), wxSize(83,24), 0, wxDefaultValidator, _T("ID_BUTTON6"));
    top3_sizer2->Add(ButtonQryBank, 0, wxEXPAND | wxALL, 4);
    SpinCtrlMoney_ = new wxSpinCtrl(bankSettingPanel_, wxID_ANY, _T("1"), wxPoint(408,40), wxSize(80,22), 0, 1, 100000000, 1, _T("ID_SPINCTRL5"));
    SpinCtrlMoney_->SetValue(_T("1"));
    top3_sizer2->Add(SpinCtrlMoney_, 0, wxEXPAND | wxALL, 4);
    wxButton* ButtonBank2Future = new wxButton(bankSettingPanel_, ID_BTN_BANK_FEATURE, _("银转期"), wxPoint(496,40), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON9"));
    top3_sizer2->Add(ButtonBank2Future, 0, wxEXPAND | wxALL, 4);

    top3_sizer->Add(top3_sizer2, 0, wxEXPAND | wxALL, 4);

    // hide bank setting for the moment, not done yet.
    bankSettingPanel_->Hide();
       
    msgDisplayer_ = new LogDisplayer(this, wxID_ANY);
    top_sizer->Add(msgDisplayer_, 1, wxEXPAND | wxALL, 4);

    Layout();

    continueOrderTimer_.SetOwner(this, ID_TIMER_ORDER);
    StatusBar1_ = new wxStatusBar(this, wxID_ANY, wxST_SIZEGRIP|wxSIMPLE_BORDER, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[1] = { -500 };
    int __wxStatusBarStyles_1[1] = { wxSB_FLAT };
    StatusBar1_->SetFieldsCount(1,__wxStatusBarWidths_1);
    StatusBar1_->SetStatusStyles(1,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1_);

    OrderSettingPanel_->Enable(false);
    bankSettingPanel_->Enable(false);

    SetupFrontAddr();
    TextCtrlInvestor_->SetValue(wxString("00000330", wxConvLibc));
}

void FrameMain::SetupFrontAddr()
{
    front_addr_.clear();
    ConfigReader::GetFrontAddr(front_addr_);
    for (map<string, pair<string, string>>::iterator it = front_addr_.begin(); it != front_addr_.end(); ++it)
    {
        ChoiceFront_->Append(it->first);
    }
}

//登录
void FrameMain::OnButtonLoginClick(wxCommandEvent& event)
{
    trader_.SetInvestorInfo(TextCtrlInvestor_->GetValue().c_str(), TextCtrlPassWord_->GetValue().c_str());

    string front_name = ChoiceFront_->GetStringSelection().c_str();

    map<string, pair<string, string>>::iterator it = front_addr_.find(front_name);
    if (it == front_addr_.end())
    {
        wxMessageBox("Internal error, invalid front address", "Error");
        return;
    }

    pair<string, string> front = it->second;
    trader_.ReqUserConnect(front.second.c_str(), front.first.c_str());
}

//查持仓
void FrameMain::OnButtonQryPositionClick(wxCommandEvent& event)
{
    trader_.ReqQryInvestorPosition();
}

//查资金
void FrameMain::OnButtonReqQryAccountClick(wxCommandEvent& event)
{
    trader_.ReqQryTradingAccount();
}

//查签约银行
void FrameMain::OnButtonQryBanksClick(wxCommandEvent& event)
{
    trader_.ReqQryAccountregister();
}

//查银行帐户
void FrameMain::OnButtonQryBankClick(wxCommandEvent& event)
{
    trader_.ReqQueryBankAccountMoneyByFuture(ChoiceBanks_->GetLabel().substr(0,1).c_str(),
                                     TextCtrlBankPwd_->GetLabel().mb_str(), TextCtrlAccountPwd_->GetLabel().mb_str());
}

//期->银
void FrameMain::OnButtonFuture2BankClick(wxCommandEvent& event)
{
    trader_.ReqTransferByFuture(ChoiceBanks_->GetLabel().substr(0,1).c_str(),
                        TextCtrlBankPwd_->GetLabel().mb_str(), TextCtrlAccountPwd_->GetLabel().mb_str(),
                        SpinCtrlMoney_->GetValue(), true);
}

//银->期
void FrameMain::OnButtonBank2FutureClick(wxCommandEvent& event)
{
    trader_.ReqTransferByFuture(ChoiceBanks_->GetLabel().substr(0,1).c_str(),
                        TextCtrlBankPwd_->GetLabel().mb_str(), TextCtrlAccountPwd_->GetLabel().mb_str(),
                        SpinCtrlMoney_->GetValue(), false);
}

static string ExtractInstrument(const string& str)
{
    vector<string> out;
    StlUtil::Split(str, ',', out);
    if (out.empty()) return "";

    string instrument = out[0];
    out.clear();
    StlUtil::Split(instrument, ':', out);
    if (out.empty()) return "";
    else if (out.size() == 1) return out[0];
    else return out[1];   
}

//下单
void FrameMain::OnButtonOrderInsertClick(wxCommandEvent& event)
{
    int offset = ChoiceOffset_->GetCurrentSelection();

    if(offset == 2)	offset = 3;//平今3

    string instrument = ExtractInstrument(string(ComboBoxInstrument_->GetValue().c_str()));
    if (instrument.empty()) return;

    trader_.ReqOrderInsert(instrument.c_str(), SpinCtrlPrice_->GetValue(),
                   ChoiceDirector_->GetCurrentSelection(), offset, SpinCtrlVolume_->GetValue());
}

void FrameMain::PushContinuousOrder()
{
    static int slp = 0;
    static int index = -1;
    static int ticks = 0,sec = 0;
    static int offset = 0;
    static int director = -1, volume = -1;
    static string instrument;
    static double price = -1;

    if (index == -1)
    {
        index = 0;
        offset = ChoiceOffset_->GetCurrentSelection();
        if(offset == 2)	offset = 3; //平今3

        ticks = SpinCtrlTicks_->GetValue();
        sec   = SpinCtrlSeconds_->GetValue();
        instrument = ExtractInstrument(string(ComboBoxInstrument_->GetValue().c_str()));
        if (instrument.empty()) return;

        price = SpinCtrlPrice_->GetValue();
        director = ChoiceDirector_->GetCurrentSelection();
        volume = SpinCtrlVolume_->GetValue();

        slp = 1000 / ticks;

        continueOrderTimer_.Start(slp);
    }

    if (index == ticks * sec)
    {
        index = -1;
        continueOrderTimer_.Stop();
    }
    else
    {
        index++;
        trader_.ReqOrderInsert(instrument.c_str(), price, director, offset, volume);
    }
}

void FrameMain::OnOrderTimer(wxTimerEvent& event)
{
    PushContinuousOrder();
}

//连续报单
void FrameMain::OnButtonOrderContinueClick(wxCommandEvent& event)
{
    PushContinuousOrder();
}

//撤单
void FrameMain::OnButtonOrderActionClick(wxCommandEvent& event)
{
    vector<string> v;
    string tmp = ChoiceBrokerSeq_->GetStringSelection().c_str();

    StlUtil::Split(tmp, "@@", v);
    if (tmp.empty()) return;

    tmp = v[0];
    v.clear();
    StlUtil::Split(tmp, ':', v);
    if (tmp.empty()) return;

    tmp = v[1];
    v.clear();
    StlUtil::Split(tmp, '|', v);
    if (tmp.empty()) return;

    trader_.ReqOrderAction(v[0].c_str(), atoi(v[1].c_str()), atoi(v[2].c_str()), v.back().c_str());
}

//选择合约：修改最大允许下单
void FrameMain::OnComboBoxInstrumentSelected(wxCommandEvent& event)
{
    // CThostFtdcInstrumentField f = trader_.GetInstrumentByLabel(string(ComboBoxInstrument_->GetValue().c_str()));
    // SpinCtrlVolume_->SetMax(f.MaxLimitOrderVolume);
}

void FrameMain::ShowOrderStatus(const wxString& msg)
{
    vector<string> out;
    StlUtil::Split(string(msg.c_str()), "@#@", out);
    if (out.empty()) return;

    map<string, string>::iterator it = orderStatus_.find(out[0]);
    if (it != orderStatus_.end())
    {
        int pos = ChoiceBrokerSeq_->FindString(it->first + "@#@" + it->second, true);
        if (pos != wxNOT_FOUND) ChoiceBrokerSeq_->Delete(pos);
    }
            
    orderStatus_[out[0]]  = out[1];
    ChoiceBrokerSeq_->Append(msg);
}

//定时刷新界面
void FrameMain::OnFeaturesTradingMsg(wxCommandEvent& event)
{
    wxString msg = event.GetString();

    switch(event.GetInt())
    {
        case MAIN_WIN_MSG_INSTRUMENT:
            {
                if (ComboBoxInstrument_->FindString(msg, true) == wxNOT_FOUND)
                    ComboBoxInstrument_->Append(msg);
            }
            break;
        case MAIN_WIN_MSG_ORDER:
            {
                ShowOrderStatus(msg);
            }
            break;
        case MAIN_WIN_MSG_RUNTIME_LOG:
            {
                msgDisplayer_->AddLog(msg);
            }
            break;
        case MAIN_WIN_MSG_QUOTE_DATA:
            {
            }
            break;
        case MAIN_WIN_MSG_LOGIN:
            {
                ButtonLogin_->Enable(false);
                OrderSettingPanel_->Enable();
                bankSettingPanel_->Enable();
            }
            break;
        default:
            {
            }
    }
}

void FrameMain::OnButtonQryDetailClick(wxCommandEvent& event)
{
    trader_.ReqQryInvestorPositionDetail();
}


