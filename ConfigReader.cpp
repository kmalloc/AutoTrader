#include <fstream>
#include <iostream>

#include "StlUtil.h"
#include "ConfigReader.h"

using namespace std;

static const char* gs_config_file = "TraderFrontConfig.cfg";
static const char  gs_config_comment_char = '#';
static const char* gs_config_field_delim = "$$";

namespace ConfigReader
{
    string ReadOneValidLine(istream& fin)
    {
        std::string line;
        std::getline(fin, line);

        StlUtil::StringTrim(line);

        while ((line.empty() && fin.good()) || (!line.empty() && line[0] == gs_config_comment_char))
        {
            // null line or comment line.
            line = "";
            std::getline(fin, line);
            StlUtil::StringTrim(line);
        }

        return line;
    }

    int GetFrontAddr(map<string, pair<string, string>>& output)
    {
        int co = 0;
        std::ifstream fin(gs_config_file);
        std::string line = ReadOneValidLine(fin);

        while (!line.empty())
        {
            vector<string> out;
            StlUtil::Split(line, gs_config_field_delim, out);

            size_t sz = out.size();
            if (sz > 2)
            {
                ++co;
                output[out[0]] = pair<string, string>(out[1], out[2]);
            }

            line = ReadOneValidLine(fin);
        }

        return co;
    }
}

