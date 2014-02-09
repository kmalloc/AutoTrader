#ifndef H_LIBCOMMON_GUI_LOG_DISPLAYER_H_
#define H_LIBCOMMON_GUI_LOG_DISPLAYER_H_

#include <wx/panel.h>
#include <wx/textctrl.h>
#include <wx/log.h>

class  LogPanel: public wxPanel
{
    public:
        LogPanel(wxWindow* parent,
                wxWindowID winid = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize);

        ~LogPanel();

        void LogMessage(const wxString& content, int lines=-1);
        void CleanUp();
        void GetMessage(wxString& content);
        void SelctAll();
    protected:
        void Clear();
        void ConnectRightClickEvent(wxObjectEventFunction function, wxEvtHandler* eventSink);
    private:
        wxTextCtrl*         mw_outputTC;        /* the output text control */
        DECLARE_EVENT_TABLE()
};

class LogDisplayer: public LogPanel
{
    public:
        LogDisplayer(wxWindow*parent,wxWindowID id = wxID_ANY);
        virtual ~LogDisplayer();

        void AddLog(std::string& log);
        void AddLog(const char*log);

        void Clear();
        void SaveAs(const std::string& path);

    protected:
        void EnableWxLogChain(bool enable = true);

        class wxLogCatcher:public wxLog
        {
        public:
            wxLogCatcher(LogDisplayer* Operand);
            virtual ~wxLogCatcher();
        protected:
            //wxLogxxx
            virtual void DoLogString(const wxChar *szString, time_t t);
        private:
            LogDisplayer* m_displayer;
        };

    private:

        void OnSaveAs(wxCommandEvent& evt);
        void OnClear(wxCommandEvent& evt);
        void OnSelectAll(wxCommandEvent& evt);
        void OnRightClicked(wxMouseEvent& evt);
        void OnUpdateGuiLog(wxCommandEvent& evt);
        void InitWxLogChain();

        //disable copying.
        LogDisplayer(const LogDisplayer&);
        LogDisplayer& operator=(const LogDisplayer&);

        wxLogChain *m_pChain;
        bool m_wxChainRedirectOn;

        DECLARE_EVENT_TABLE()
};

#endif

