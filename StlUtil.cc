#include "StlUtil.h"

#include <stdlib.h>

using namespace std;

namespace StlUtil
{
    int Split(const string& str, char delim, vector<string>& out)
    {
        int c = 0;
        stringstream ss(str);
        string item;

        while (getline(ss, item, delim))
        {
            ++c;
            out.push_back(item);
        }

        return c;
    }

    int Split(const string& str, const string& delim, vector<string>& out)
    {
         int co = 0;
         size_t pos = 0;
         size_t np = str.find(delim, 0);

         while (np != string::npos && pos < str.size())
         {
             ++co;
             out.push_back(str.substr(pos, np - pos));

             pos = np + delim.size();
             np  = str.find(delim, pos);
         }

         if (pos < str.size())
         {
             ++co;
             out.push_back(str.substr(pos));
         }

         return co;
    }

    string StringTrim(string& str, char ch)
    {
        string::size_type pos = str.find_last_not_of(ch);
        if(pos != string::npos)
        {
            str.erase(pos + 1);
            pos = str.find_first_not_of(ch);
            if(pos != string::npos) str.erase(0, pos);
        }
        else
        {
            str = "";
        }

        return str;
    }
}

