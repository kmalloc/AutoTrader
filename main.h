#ifndef _TRADER_MAIN_H_
#define _TRADER_MAIN_H_

#include <wx/log.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/button.h>
#include <wx/frame.h>
#include <wx/timer.h>
#include <wx/combobox.h>
#include <wx/statusbr.h>

#include "TraderSpi.h"

class LogDisplayer;

class AutoTraderApp: public wxApp
{
    public:    
        AutoTraderApp();
        virtual ~AutoTraderApp();
        virtual bool OnInit();
    private:
        wxLog* logTarget_;
};

class FrameMain: public wxFrame
{
    public:   
        FrameMain(const wxString& title, const wxPoint& pos, const wxSize& size);
    private:   
        void OnHello(wxCommandEvent& event);    
        void OnExit(wxCommandEvent& event);   
        void OnAbout(wxCommandEvent& event);   
        void OnButtonLoginClick(wxCommandEvent& event);
		void OnButtonOrderInsertClick(wxCommandEvent& event);
		void OnTimer1Trigger(wxTimerEvent& event);
		void OnCheckBoxPriceTypeClick(wxCommandEvent& event);
		void OnComboBoxInstrumentSelected(wxCommandEvent& event);
		void OnButtonOrderContinueClick(wxCommandEvent& event);
		void OnButtonQryPositionClick(wxCommandEvent& event);
		void OnButtonReqQryAccountClick(wxCommandEvent& event);
		void OnButtonQryBanksClick(wxCommandEvent& event);
		void OnButtonQryFutureClick(wxCommandEvent& event);
		void OnButtonQryBankClick(wxCommandEvent& event);
		void OnButtonFuture2BankClick(wxCommandEvent& event);
		void OnButtonBank2FutureClick(wxCommandEvent& event);
		void OnClose(wxCloseEvent& event);
		void OnButtonOrderActionClick(wxCommandEvent& event);
		void OnButtonQryDetailClick(wxCommandEvent& event);

        void OnOrderTimer(wxTimerEvent& event);
        void OnFeaturesTradingMsg(wxCommandEvent&);
        void PushContinuousOrder();

        void SetupFrontAddr();
        void ShowOrderStatus(const wxString& msg);
    private:

        CTraderSpi  trader_;

        wxTextCtrl* TextCtrlInvestor_;
        wxTextCtrl* TextCtrlPassWord_;
        wxTextCtrl* TextCtrlOther_;
        wxTextCtrl* TextCtrlBroker_;
		wxTextCtrl* TextCtrlBankPwd_;
		wxTextCtrl* TextCtrlAccountPwd_;

        wxChoice* ChoiceFront_;
		wxChoice* ChoiceDirector_;
		wxChoice* ChoiceBrokerSeq_;
		wxChoice* ChoiceOffset_;
		wxChoice* ChoiceBanks_;

		wxSpinCtrl* SpinCtrlSeconds_;
		wxSpinCtrl* SpinCtrlPrice_;
		wxSpinCtrl* SpinCtrlMoney_;
		wxSpinCtrl* SpinCtrlTicks_;
		wxSpinCtrl* SpinCtrlVolume_;

		wxCheckBox* CheckBoxPriceType_;
		wxStatusBar* StatusBar1_;
        wxTimer continueOrderTimer_;
		wxComboBox* ComboBoxInstrument_;

        LogDisplayer* msgDisplayer_;

        wxButton* ButtonLogin_;

        wxStaticText* front_static_;
        wxStaticText* broker_static_;
        wxPanel* OrderSettingPanel_;
        wxPanel* bankSettingPanel_;

        std::map<std::string, std::pair<std::string, std::string>> front_addr_;
        std::map<std::string, std::string> orderStatus_;

        DECLARE_EVENT_TABLE();
};

#endif
