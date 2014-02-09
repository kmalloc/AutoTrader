#include "LogDisplayer.h"

#include <wx/menu.h>
#include <wx/window.h>
#include <wx/file.h>
#include <wx/filedlg.h>
#include <wx/sizer.h>

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EXPORTED_EVENT_TYPE("C", wxEVT_UPDATE_GUI_LOG, -1)
END_DECLARE_EVENT_TYPES()

DEFINE_EVENT_TYPE(wxEVT_UPDATE_GUI_LOG)

enum
{
    ID_SAVE_AS = 2013,
    ID_CLEAR_AS,
    ID_SELECT_ALL
};


BEGIN_EVENT_TABLE(LogDisplayer,LogPanel)
EVT_RIGHT_UP(LogDisplayer::OnRightClicked)
EVT_MENU(ID_SAVE_AS, LogDisplayer::OnSaveAs)
EVT_MENU(ID_CLEAR_AS, LogDisplayer::OnClear)
EVT_MENU(ID_SELECT_ALL, LogDisplayer::OnSelectAll)
EVT_COMMAND(-1,wxEVT_UPDATE_GUI_LOG,LogDisplayer::OnUpdateGuiLog)
END_EVENT_TABLE()


LogDisplayer::LogDisplayer(wxWindow*parent,wxWindowID id /* = wxID_ANY */)
:LogPanel(parent)
,m_wxChainRedirectOn(false)
,m_pChain(NULL)
{
    InitWxLogChain();
    EnableWxLogChain();
    // Connect Right Click Event
    ConnectRightClickEvent(wxMouseEventHandler(LogDisplayer::OnRightClicked), this->GetEventHandler());
}

LogDisplayer::~LogDisplayer()
{
    EnableWxLogChain(false);
}

void LogDisplayer::OnRightClicked(wxMouseEvent&evt)
{
    wxPoint pos = evt.GetPosition();
    wxMenu right_click_menu;
    right_click_menu.Append(ID_SAVE_AS, "Save As");
    right_click_menu.Append(ID_CLEAR_AS, "Clear");
    right_click_menu.AppendSeparator();
    right_click_menu.Append(wxID_COPY, _("&Copy"));
    right_click_menu.AppendSeparator();
    right_click_menu.Append(ID_SELECT_ALL, _("Select &All"));
    wxWindow::PopupMenu(&right_click_menu, pos.x, pos.y);
}

void LogDisplayer::OnUpdateGuiLog(wxCommandEvent&evt)
{
    LogMessage(evt.GetString());
}

void LogDisplayer::AddLog(const char*log)
{
    wxDateTime now = wxDateTime::Now();
    wxString msg = wxT(" [") + now.FormatISODate() + wxT("_") + now.FormatISOTime() + wxT("]: ") + log;

    wxCommandEvent evt(wxEVT_UPDATE_GUI_LOG);
    evt.SetId(this->GetId());
    evt.SetString(msg);

    this->AddPendingEvent(evt);
}

void LogDisplayer::AddLog(std::string& log)
{
   AddLog(log.c_str());
}

void LogDisplayer::Clear()
{
    CleanUp();
}

void LogDisplayer::SaveAs(const std::string& path)
{
    wxFile file(path.c_str(),wxFile::write);
    wxString msg;
    GetMessage(msg);
    file.Write(msg);
}

void LogDisplayer::OnSaveAs(wxCommandEvent&evt)
{
    wxFileDialog fd(this, "Save As...","", "", 
        "TXT(*.txt)|*.txt|All files (*.*)|*.*", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

    if (fd.ShowModal() != wxID_OK) return;

    SaveAs(std::string(fd.GetPath()));
}

void LogDisplayer::EnableWxLogChain(bool enable /* = true */)
{
    if(enable == true)
    {
        if(m_wxChainRedirectOn == false && m_pChain == NULL)
        {
            m_wxChainRedirectOn = true;
            m_pChain = new wxLogChain(new wxLogCatcher(this));
        }
        else
        {
            m_wxChainRedirectOn = true;
            wxLog::SetActiveTarget(m_pChain);
        }
    }
    else
    {
        if(m_wxChainRedirectOn == true)
        {
            m_wxChainRedirectOn = false;
            wxLog::SetActiveTarget(NULL);
        }
    }
}

void LogDisplayer::InitWxLogChain()
{
    //tricky here.
    //for linux gui, default wx log target is null after the application start up.
    //calling wxLogWarning(and other wxLogXXX function) will force wxLog to  create a log target automatically in runtime.
    //so ,if we want to maintain the expected behavior(wxLogXXX output to a messagebox) we must ensure there is an log target
    // before installing our LogDisplayer to intercept log.

    wxLog::GetActiveTarget();
}

void LogDisplayer::OnClear( wxCommandEvent& evt )
{
    Clear();
}

void LogDisplayer::OnSelectAll( wxCommandEvent& evt )
{
    SelctAll();
}

//wxLogCatcher
//
LogDisplayer::wxLogCatcher::wxLogCatcher(LogDisplayer* Operand):wxLog()
,m_displayer(Operand)
{

}

LogDisplayer::wxLogCatcher::~wxLogCatcher()
{

}

//wxLogxx,chain.
void LogDisplayer::wxLogCatcher::DoLogString(const wxChar *szString, time_t t)
{
    m_displayer->AddLog(wxString("<wxLog>")+szString);
}


BEGIN_EVENT_TABLE(LogPanel, wxPanel)

END_EVENT_TABLE()

LogPanel::LogPanel( wxWindow* parent, wxWindowID winid,
        const wxPoint& pos, const wxSize& size )
        :wxPanel(parent,winid, pos, size)
        ,mw_outputTC(NULL)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mw_outputTC = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_RICH | wxTE_LEFT | wxTE_BESTWRAP | wxTE_READONLY);

    mainSizer->Add(mw_outputTC, 1, wxEXPAND | wxALL, 2);

    SetSizer(mainSizer);
    Layout();
}

LogPanel::~LogPanel()
{
    Clear();
}

void LogPanel::Clear()
{

}

void LogPanel::LogMessage( const wxString& data, int lines/*=-1*/ )
{
    int old_lines = mw_outputTC->GetNumberOfLines();
    mw_outputTC->AppendText( data + "\n" );
    int new_lines = mw_outputTC->GetNumberOfLines();
    if (lines>=0 && lines<=new_lines)
    {
        wxString lastStr;
        for ( int i = new_lines - lines; i < new_lines; ++i )
        {
            lastStr += mw_outputTC->GetLineText( i );
            lastStr += "\n";
        }
        mw_outputTC->SetValue(lastStr);
        mw_outputTC->ScrollLines(lines - 1);
    }
    else
    {
        mw_outputTC->ScrollLines( new_lines - old_lines );
    }

    Refresh();
}

void LogPanel::CleanUp()
{
    mw_outputTC->Clear();
}

void LogPanel::GetMessage(wxString& content)
{
    content = mw_outputTC->GetValue();
}

void LogPanel::ConnectRightClickEvent(wxObjectEventFunction function, wxEvtHandler* eventSink)
{
    mw_outputTC->Connect(wxEVT_RIGHT_DOWN, function, 0, eventSink);
}

void LogPanel::SelctAll()
{
    mw_outputTC->SetSelection(0, mw_outputTC->GetLastPosition());
}

