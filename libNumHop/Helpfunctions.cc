#include "Helpfunctions.h"

using namespace std;
namespace numhop {

//! @brief Split a string into multiple rows based on ; or \n characters
//! @param[in] expr The expression as a string
//! @param[in] comment The comment character (ignore those lines)
//! @param[out] rExpressions The list of expression lines
void splitExprRows(const string &expr, const char &comment, list<string> &rExpressions)
{
    size_t s=0;
    do
    {
        size_t e = expr.find_first_of(";",s);
        size_t en = expr.find_first_of("\n",s);
        size_t ec = expr.find_first_of(comment,s);
        e = std::min(std::min(e,en), ec);
        if (s<expr.size() && (e-s)>0)
        {
            string part = expr.substr(s,e-s);
            stripLeadingTrailingWhitespaces(part);
            if (!part.empty())
            {
                rExpressions.push_back(part);
            }
        }
        if (e == string::npos)
        {
            break;
        }
        // If comment skip to after next newline
        if (expr[e] == comment)
        {
            s = expr.find_first_of('\n', e);
            if (s != string::npos)
            {
                s = s+1;
            }
        }
        // Else advance one step before the found ; or \n character
        else
        {
            s = e+1;
        }
    }while(true);
}

//! @brief Strip leading and trailing spaces from a string
//! @param[in,out] rString The string to process
void stripLeadingTrailingWhitespaces(string &rString)
{
    while (!rString.empty() && rString[0] == ' ')
    {
        rString.erase(0,1);
    }
    while (!rString.empty() && rString[rString.size()-1] == ' ')
    {
        rString.erase(rString.size()-1);
    }
}

//! @brief Strip leading and trailing parenthesis ( ) from a string
//! @param[in,out] rString The string to process
//! @param[out] rDidStrip Indicates  whether parenthesis were removed or not
//! @returns false if there is an error in the number of parenthesis, else true
bool stripLeadingTrailingParanthesis(string &rString, bool &rDidStrip)
{
    rDidStrip = false;
    if (!rString.empty() && rString[0] == '(' && rString[rString.size()-1] == ')')
    {
        // Need to count parenthesis so that we only clear if the closing one closes the one first opened
        size_t numOpen=1;
        bool doClear=false;
        for (size_t i=1; i<rString.size(); ++i)
        {
            char &c = rString[i];
            if (c == '(')
            {
                numOpen++;
            }
            else if (c == ')')
            {
                numOpen--;
            }

            // Break if we finally close the first parenthesis
            if (numOpen == 0)
            {
                if (i == rString.size()-1)
                {
                    doClear=true;
                }
                break;
            }
        }

        if (doClear)
        {
            rString.erase(0,1);
            rString.erase(rString.size()-1, 1);
        }
        rDidStrip = doClear;
    }

    return true;
}

//! @brief Convert multiple consecutive divisions into multiplications of fractions
//! @details Example: 4/3/6/7 becomes 4/3*1/6*1/7
//! @param[in,out] rString The string to process
void fixMultiDivision(string &rString)
{
    size_t i=0;
    int ld=-1;
    while(i<rString.size())
    {
        char &c = rString[i];
        if (c == '/')
        {
            if (ld < 0)
            {
                ld = i;
            }
            else
            {
                size_t insertAt = i;
                rString.insert(insertAt, "*1");
                ld = i+2;
                i=ld;
            }
        }
        else if (c == '*')
        {
            ld = -1;
        }
        ++i;
    }
}

}
