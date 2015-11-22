#include <iostream>
#include <string>
#include <list>
#include <map>


#include "numhop.h"

using namespace std;
using namespace numhop;

class MyVariableStorage : public ExternalVariableStorage
{
public:
    MyVariableStorage(map<string, double> *pExtVars)
    {
        mpVars = pExtVars;
    }

    double externalValue(string name, bool &rFound) const
    {
        if (mpVars->count(name) > 0)
        {
            rFound = true;
            return mpVars->at(name);
        }
        rFound = false;
        return 0;
    }

    bool setExternalValue(string name, double value)
    {
        map<string, double>::iterator it = mpVars->find(name);
        if (it != mpVars->end())
        {
            //cout << "external found" << endl;
            it->second = value;
            return true;
        }
        return false;
    }

private:
    map<string, double> *mpVars;
};


int main()
{
    map<string, double> externalVars;
    MyVariableStorage extVariableStorage(&externalVars);
    VariableStorage variableStorage;

    variableStorage.setExternalStorage(&extVariableStorage);
    cout << "Hello World!" << endl;

    //std::string expr = "a=5;a=8;a;";
    //std::string expr = "a=5;\n a=8\n a;";
    //std::string expr = "a=1;b=2;c=3;d=a+b*c;d";
    //std::string expr = "a=1;b=2;c=3;d=4;a=(3+b)+4^2*c^(2+d)-7*(d-1)";
    //std::string expr = "a=1;b=2;c=3;d=4;a=(3+b)+4^2*c^(2+d)-7/6/5*(d-1)";
    //std::string expr = "7/3/4/5";
    //std::string expr = "(4/3*14*7/3/4/5*5/(4*3/2))";
    std::string expr = " \t #   \n    a=5;\n #   a=8\n a+1; \r\n a+2 \r a+3 \r\n #Some comment ";


    externalVars.insert(pair<string,double>("dog", 55));
    externalVars.insert(pair<string,double>("cat", 66));
    //std::string expr = "cat \n dog \r dog=5;cat=2;a=3;b=dog*cat*a;b";

    list<string> exprlist;
    extractExpressionRows(expr, '#', exprlist);

    for (list<string>::iterator it = exprlist.begin(); it!=exprlist.end(); ++it)
    {
        Expression e;
        interpretExpressionStringRecursive(*it, e);
        bool evalOK;
        double value = e.evaluate(variableStorage, evalOK);
        cout << "Evaluating: "  << *it << " --> " << e.print() << "     Value: " << value << " evalOK:" << evalOK << endl;
    }

    return 0;
}

