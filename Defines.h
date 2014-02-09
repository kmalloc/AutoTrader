#ifndef AUTO_TRADE_DEFINE_H_
#define AUTO_TRADE_DEFINE_H_

#include <wx/event.h>
#include <vector>
#include <string>

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(EVENT_MAIN_WIN_SHOW_MSG, -1)
END_DECLARE_EVENT_TYPES()

enum
{
    MAIN_WIN_MSG_INSTRUMENT, // 合约信息
    MAIN_WIN_MSG_RUNTIME_LOG,// 运行日记
    MAIN_WIN_MSG_ORDER, // 报单信息
    MAIN_WIN_MSG_QUOTE_DATA, // 行情信息
    MAIN_WIN_MSG_LOGIN
};

int Split(const std::string& str, char delim, std::vector<std::string>& out);

#endif

