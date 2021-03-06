#include "numhop/Expression.h"
#include "numhop/Helpfunctions.h"
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>

namespace numhop {

const std::string allOperators="=+-*/^<>&|";
const std::string operatorsNotAllowedAfterEqualSign="=*/^<>&|";
const std::string operatorsNotPME="*/^<>&|";

// Internal help functions
inline double boolify(const double v)
{
    if (v>0.5) {return 1.;} return 0.;
}

inline bool checkOperatorsNexttoEqualSign(size_t e, const std::string &expr)
{
    if ( (int(e) < int(expr.size())-1) && contains(operatorsNotAllowedAfterEqualSign, expr[e+1]) )
    {
        return false;
    }
    if ( e==0 || (e>0 && contains(allOperators, expr[e-1])) )
    {
        return false;
    }
    return true;
}

bool fixMultiOperators(size_t e, std::string &expr)
{
    char op = expr[e];
    const size_t s=e;

    // Now check + and minus signs
    if (op == '+' || op == '-')
    {
        bool isPositive=true;
        // Search forward and compress multiple signs into one
        for (; e<expr.size(); ++e)
        {
            if (expr[e] == '-')
            {
                isPositive = !isPositive;
            }
            else if (expr[e] != '+')
            {
                break;
            }
        }
        // Now replace the sequence of operators with one sign character
        if (isPositive)
        {
            expr.replace(s, e-s, 1, '+');
        }
        else
        {
            expr.replace(s, e-s, 1, '-');
        }
    }
    else if (contains(operatorsNotPME, op))
    {
        if (e<expr.size()-1 && contains(allOperators, expr[e+1]))
        {
             return false;
        }
    }
    return true;
}

double decideIfNumericConstantOrNamedValue(const std::string &expr, bool &rIsNumericConstant, bool &rIsNamedValue)
{
    char* pEnd;
    double value = strtod(expr.c_str(), &pEnd);
    rIsNumericConstant = (pEnd != expr.c_str()) && (pEnd == expr.c_str()+expr.size());
    rIsNamedValue = !rIsNumericConstant;
    return value;
}

//! @brief Check if a char is fisrt part of an exponential notation
//! @param[in] expr The expression string
//! @param[in] i The index of the first char efter the e or E
//! @returns True or False
bool isExpNot(const std::string &expr, const size_t i)
{
    // For i to be first part of exponential notation,
    // prev. char should be 'e' or 'E'
    // prev.prev. char should be a digit
    if (i>1)
    {
        //const char &c = expr[i];
        //! @todo maybe should check that c is +, - or digit
        const char &pc = expr[i-1];
        const char &ppc = expr[i-2];
        return ((pc == 'e') || (pc == 'E')) && isdigit(ppc);
    }
    return false;
}

bool notAlphaNum(const char c)
{
    return !isalnum(c);
}

//! @brief Check if expression matches to form abc123(something)
bool expressionIsFunctionCall(std::string& expr)
{
    if (expr.empty()) {
        return false;
    }

    size_t last = expr.size()-1;

    if (isalpha(expr[0]) && expr[last]==')') {
        std::string::iterator it = std::find_if(expr.begin(), expr.end(), notAlphaNum);
        if ( (it != expr.end()) && (*it == '(') ) {
            return true;
        }
    }
    return false;
}

bool splitFunctionCallExpression(const std::string& expr, std::string& funcName, std::list<Expression>& args)
{
    std::string::size_type arg_start = expr.find_first_of('(');
    std::string::size_type args_end = expr.find_last_of(')');

    funcName = expr.substr(0, arg_start);

    ++arg_start;
    while (arg_start < args_end) {
        std::string::size_type arg_end = expr.find_first_of(',', arg_start);
        if (arg_end == std::string::npos) {
            arg_end = args_end;
        }
        args.push_back(Expression(expr.substr(arg_start, arg_end-arg_start), AdditionT));
        arg_start = arg_end+1;
    }

    return true;
}

template <typename T>
T min(T a, T b)
{
    return std::min(a,b);
}

template <typename T>
T max(T a, T b)
{
    return std::max(a,b);
}

class FunctionHandler
{
public:
    typedef double(*onearg_function)(double);
    typedef double(*twoarg_function)(double, double);

    FunctionHandler() : mIdCounter(0)
    {
        // register single argument built-in math functions
        registerFunction("cos", static_cast<onearg_function>(&cos));
        registerFunction("sin", static_cast<onearg_function>(&sin));
        registerFunction("tan", static_cast<onearg_function>(&tan));
        registerFunction("acos", static_cast<onearg_function>(&acos));
        registerFunction("asin", static_cast<onearg_function>(&asin));
        registerFunction("atan", static_cast<onearg_function>(&atan));

        registerFunction("cosh", static_cast<onearg_function>(&cosh));
        registerFunction("sinh", static_cast<onearg_function>(&sinh));
        registerFunction("tanh", static_cast<onearg_function>(&tanh));

        registerFunction("exp", static_cast<onearg_function>(&exp));
        registerFunction("log", static_cast<onearg_function>(&log));
        registerFunction("log10", static_cast<onearg_function>(&log10));

        registerFunction("sqrt", static_cast<onearg_function>(&sqrt));

        registerFunction("ceil", static_cast<onearg_function>(&ceil));
        registerFunction("floor", static_cast<onearg_function>(&floor));
        registerFunction("abs", static_cast<onearg_function>(&fabs));

        // register two argument built-in math functions
        registerFunction("atan2", static_cast<twoarg_function>(&atan2));
        registerFunction("pow", static_cast<twoarg_function>(&pow));
        registerFunction("fmod", static_cast<twoarg_function>(&fmod));
        registerFunction("min", static_cast<twoarg_function>(&min<double>));
        registerFunction("max", static_cast<twoarg_function>(&max<double>));

    }

    int registerFunction(const std::string& name, onearg_function funcPointer)
    {
        int id = registerName(name);
        mOneArgFuncs.insert(std::pair<size_t, onearg_function>(id, funcPointer));
        return id;
    }

    int registerFunction(const std::string& name, twoarg_function funcPointer)
    {
        int id = registerName(name);
        mTwoArgFuncs.insert(std::pair<size_t, twoarg_function>(id, funcPointer));
        return id;
    }

    int lookupFunctionId(const std::string& name, const size_t numArgs) const
    {
        std::map<std::string, int>::const_iterator it = mNameIdMap.find(name);
        int id = (it != mNameIdMap.end()) ? it->second : -1;
        switch (numArgs) {
        case 1 :
            return contains(mOneArgFuncs, id) ? id : -1;
        case 2 :
            return contains(mTwoArgFuncs, id) ? id : -1;
        default:
            return -1;
        }
    }

    double callFunction(const int id, const std::list<Expression>& args, VariableStorage &rVariableStorage, bool &rEvalOK)
    {
        if (id >= 0) {
            const size_t numArgs = args.size();
            if (numArgs == 1) {
                double arg1 = args.front().evaluate(rVariableStorage, rEvalOK);
                return mOneArgFuncs[id](arg1);
            }
            else if (numArgs == 2) {
                bool ok1,ok2;
                double arg1 = args.front().evaluate(rVariableStorage, ok1);
                double arg2 = (++args.begin())->evaluate(rVariableStorage, ok2);
                rEvalOK = ok1 && ok2;
                return mTwoArgFuncs[id](arg1, arg2);
            }
        }
        rEvalOK=false;
        return -1;
    }

    std::vector<std::string> registeredFunctionNames() const
    {
        std::vector<std::string> names;
        names.reserve(mNameIdMap.size());
        std::map<std::string, int>::const_iterator it;
        for (it=mNameIdMap.begin(); it!=mNameIdMap.end(); ++it) {
            names.push_back(it->first);
        }
        return names;
    }

protected:
    int registerName(const std::string& name)
    {
        mNameIdMap.insert(std::pair<std::string, size_t>(name, mIdCounter));
        return mIdCounter++;
    }

    int mIdCounter;
    std::map<std::string, int> mNameIdMap;
    std::map<int, onearg_function> mOneArgFuncs;
    std::map<int, twoarg_function> mTwoArgFuncs;
    //! @todo Maybe better to use vectors for faster "constant" lookup
};

static FunctionHandler gFunctionHandler;


//! @brief Find an operator and branch the expression tree at this point
//! @param[in] exprString The expression string to process
//! @param[in] evalOperators A string with the operators to search for
//! @param[out] rExprList The resulting expression list (Empty if nothing found)
//! @returns False if some error occurred else true
bool branchExpressionOnOperator(std::string exprString, const std::string &evalOperators, std::list<Expression> &rExprList)
{
    bool hadParanthesis;
    removeAllWhitespaces(exprString);
    stripLeadingTrailingParanthesis(exprString, hadParanthesis);

    size_t nOpenParanthesis=0, e=0;
    bool foundOperator=false;
    std::vector<size_t> breakpts;
    breakpts.push_back(e);
    for (e=0; e<exprString.size(); ++e)
    {
        const char &c = exprString[e];

        // Count parenthesis
        if (c == '(')
        {
            nOpenParanthesis++;
        }
        else if (c == ')')
        {
            nOpenParanthesis--;
        }

        if (nOpenParanthesis == 0)
        {
            //! @todo it might be a good idea to compare the char with a range of asci values, to quickly decide if a char is an operator, instead of comparing every char multiple times
            bool foundOperatorAtThisLocation=false;
            // Check for assignment, (can only have one assignment in expression)
            if ( (c == '=') && contains(evalOperators, '=') )
            {
                foundOperator=true;
                breakpts.push_back(e);
                if (!checkOperatorsNexttoEqualSign(e, exprString))
                {
                    return false;
                }
            }
            else if ( (c == '+') && !isExpNot(exprString, e) && contains(evalOperators, '+') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                if (e!=0)
                {
                    breakpts.push_back(e);
                }
            }
            else if ( (c == '-') && !isExpNot(exprString, e) && contains(evalOperators, '-') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                if (e!=0)
                {
                    breakpts.push_back(e);
                }
            }
            else if ( (c == '|') && contains(evalOperators, '|') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                if (e!=0)
                {
                    breakpts.push_back(e);
                }
            }
            else if ( (c == '*') && contains(evalOperators, '*') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                breakpts.push_back(e);
            }
            else if ( (c == '/') && contains(evalOperators, '/') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                breakpts.push_back(e);
            }
            else if ( (c == '&') && contains(evalOperators, '&') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                breakpts.push_back(e);
            }
            else if ( (c == '^') && contains(evalOperators, '^') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                breakpts.push_back(e);
            }
            else if ( (c == '<') && contains(evalOperators, '<') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                breakpts.push_back(e);
            }
            else if ( (c == '>') && contains(evalOperators, '>') )
            {
                foundOperator=true;
                foundOperatorAtThisLocation=true;
                breakpts.push_back(e);
            }
            // Make sure that next character is not also an operator (not allowed right now)
            if (foundOperatorAtThisLocation && !fixMultiOperators(e, exprString))
            {
                // Error in parsing
                return false;
            }
        }
    }
    breakpts.push_back(e);

    // Add expression
    if (foundOperator)
    {
        if (containsAnyof(evalOperators, "+-|"))
        {
            for (size_t bp=0; bp<breakpts.size()-1; ++bp)
            {
                std::string left = exprString.substr(breakpts[bp], breakpts[bp+1]-breakpts[bp]);
                //char op = stripInitialSign(left);
                char op = left[0];
                if (op == '-')
                {
                    rExprList.push_back(Expression(left.substr(1), SubtractionT));
                }
                else if (op == '+')
                {
                    rExprList.push_back(Expression(left.substr(1), AdditionT));
                }
                else if (op == '|')
                {
                    rExprList.push_back(Expression(left.substr(1), OrT));
                }
                else
                {
                    rExprList.push_back(Expression(left, AdditionT));
                }
            }
        }
        else if (containsAnyof(evalOperators, "*/&"))
        {
            for (size_t bp=0; bp<breakpts.size()-1; ++bp)
            {
                std::string left = exprString.substr(breakpts[bp], breakpts[bp+1]-breakpts[bp]);
                char op = left[0];
                if (op == '/')
                {
                    rExprList.push_back(Expression(left.substr(1), DivisionT));
                }
                else if (op == '*')
                {
                    rExprList.push_back(Expression(left.substr(1), MultiplicationT));
                }
                else if (op == '&')
                {
                    rExprList.push_back(Expression(left.substr(1), AndT));
                }
                else
                {
                    rExprList.push_back(Expression(left, AdditionT));
                }
            }
        }
        else if (containsAnyof(evalOperators, "=^<>"))
        {
            if (breakpts.size() == 3)
            {
                std::string left = exprString.substr(breakpts[0], breakpts[1]-breakpts[0]);
                std::string right = exprString.substr(breakpts[1], breakpts[2]-breakpts[1]);
                char op = right[0];
                if (op == '=')
                {
                    rExprList.push_back(Expression(left, right.substr(1), AssignmentT));
                }
                else if (op == '^')
                {
                    rExprList.push_back(Expression(left, right.substr(1), PowerT));
                }
                else if (op == '<')
                {
                    rExprList.push_back(Expression(left, right.substr(1), LessThenT));
                }
                else
                {
                    rExprList.push_back(Expression(left, right.substr(1), GreaterThenT));
                }
            }
            else
            {
                // Error in parsing
                return false;
            }
        }
    }

    // No error in parsing (even if we did not find anything)
    return true;
}

//! @brief Process an expression string recursively to build an expression tree
//! @param[in] exprString The expression string to process
//! @param[out] rExprList A list of the resulting expression branches
bool interpretExpressionStringRecursive(std::string exprString, std::list<Expression> &rExprList)
{
    bool branchOK;
    branchOK = branchExpressionOnOperator(exprString, "=", rExprList);
    if (branchOK && rExprList.empty())
    {
        branchOK = branchExpressionOnOperator(exprString, "+-|", rExprList);
        if (branchOK && rExprList.empty())
        {
            branchOK = branchExpressionOnOperator(exprString, "*/&", rExprList);
            if (branchOK && rExprList.empty())
            {
                branchOK = branchExpressionOnOperator(exprString, "^<>", rExprList);
                if (branchOK && rExprList.empty())
                {
                    // This must be a value or a function call
                    const bool isFuncCall = expressionIsFunctionCall(exprString);
                    if (isFuncCall) {
                        rExprList.push_back(Expression(exprString, FunctionCallT));
                    }
                    else {
                        rExprList.push_back(Expression(exprString, ValueT));
                    }
                }
            }
        }
    }
    return branchOK;
}

//! @brief Process an expression string recursively to build an expression tree
//! @param[in] exprString The expression string to process
//! @param[out] rExpr The resulting expression tree
bool interpretExpressionStringRecursive(std::string exprString, Expression &rExpr)
{
    rExpr = Expression(exprString, AdditionT);
    return rExpr.isValid();
}

//! @brief Default constructor
Expression::Expression()
{
    commonConstructorCode();
}

//! @brief Copy constructor
Expression::Expression(const Expression &other)
{
    copyFromOther(other);
}

//! @brief Constructor taking one expression string (rhs)
Expression::Expression(const std::string &exprString, ExpressionOperatorT op)
{
    commonConstructorCode();
    mRightExpressionString = exprString;
    removeAllWhitespaces(mRightExpressionString);
    stripLeadingTrailingParanthesis(mRightExpressionString, mHadRightOuterParanthesis);
    mOperator = op;

    if (mOperator == ValueT)
    {
        if (!mRightExpressionString.empty())
        {
            mNumericConstantValue = decideIfNumericConstantOrNamedValue(mRightExpressionString, mIsNumericConstant, mIsNamedValue);
            mIsValid = true;
        }
    }
    else if (mOperator == FunctionCallT)
    {
        mIsValid = splitFunctionCallExpression(mRightExpressionString, mLeftExpressionString, mRightChildExpressions);
        if (mIsValid) {
            mFunctionId = gFunctionHandler.lookupFunctionId(mLeftExpressionString, mRightChildExpressions.size());
            mIsValid = (mFunctionId >= 0);
        }
    }
    else
    {
        mIsValid = interpretExpressionStringRecursive(mRightExpressionString, mRightChildExpressions);
        // If child expression is a value, then move the value into this expression
        if (mRightChildExpressions.size() == 1 && mRightChildExpressions.front().operatorType()==ValueT)
        {
            mRightExpressionString = mRightChildExpressions.front().rightExprString();
            mRightChildExpressions.clear();
            if (!mRightExpressionString.empty())
            {
                mNumericConstantValue = decideIfNumericConstantOrNamedValue(mRightExpressionString, mIsNumericConstant, mIsNamedValue);
                mIsValid = true;
            }
            else
            {
                mIsValid = false;
            }
        }
    }
}

//! @brief Constructor taking two expression strings (lhs and rhs)
Expression::Expression(const std::string &leftExprString, const std::string &rightExprString, ExpressionOperatorT op)
{
    commonConstructorCode();
    mLeftExpressionString = leftExprString;
    removeAllWhitespaces(mLeftExpressionString);
    stripLeadingTrailingParanthesis(mLeftExpressionString, mHadLeftOuterParanthesis);
    mRightExpressionString = rightExprString;
    removeAllWhitespaces(mRightExpressionString);
    stripLeadingTrailingParanthesis(mRightExpressionString, mHadRightOuterParanthesis);
    mOperator = op;

    if (mOperator == AssignmentT)
    {
        mRightChildExpressions.push_back(Expression(mRightExpressionString, AdditionT));
    }
    else
    {
        mLeftChildExpressions.push_back(Expression(mLeftExpressionString, AdditionT));
        mRightChildExpressions.push_back(Expression(mRightExpressionString, AdditionT));
    }
    mIsValid = true;
}

//! @brief The assignment operator
Expression &Expression::operator=(const Expression &other)
{
    // Check for self-assignment
    if (this == &other)
    {
        return *this;
    }

    // Copy
    copyFromOther(other);

    // Return this
    return *this;
}

//! @brief Check if this expression is empty
bool Expression::empty() const
{
    if (isValue())
    {
        return mRightExpressionString.empty();
    }
    else
    {
        return mRightChildExpressions.empty();
    }
}

//! @brief Check if this expression is a value, a numeric constant or named value in its right expression string
bool Expression::isValue() const
{
    return mIsNumericConstant || mIsNamedValue;
}

//! @brief Check if this expression represents a numeric constant value
bool Expression::isNumericConstant() const
{
    return mIsNumericConstant;
}

//! @brief Check if this expression represents a named value
bool Expression::isNamedValue() const
{
    return mIsNamedValue;
}

//! @brief Recursively check if an expression tree is valid after interpretation
bool Expression::isValid() const
{
    if (!mIsValid)
    {
        return false;
    }

    std::list<Expression>::const_iterator it;
    for (it=mLeftChildExpressions.begin(); it!=mLeftChildExpressions.end(); ++it)
    {
        if (!it->isValid())
        {
            return false;
        }
    }

    for (it=mRightChildExpressions.begin(); it!=mRightChildExpressions.end(); ++it)
    {
        if (!it->isValid())
        {
            return false;
        }
    }

    return true;
}

//! @brief Returns the (right hand side) expression string (without outer parenthesis)
const std::string &Expression::exprString() const
{
    return rightExprString();
}

//! @brief Returns the left hand side expression string (without outer parenthesis)
const std::string &Expression::leftExprString() const
{
    return mLeftExpressionString;
}

//! @brief Returns the right hand side expression string (without outer parenthesis)
const std::string &Expression::rightExprString() const
{
    return mRightExpressionString;
}

//! @brief Returns the operator type
ExpressionOperatorT Expression::operatorType() const
{
    return mOperator;
}

//! @brief Evaluate the expression
//! @param[in,out] rVariableStorage The variable storage to use for setting or getting variables or named values
//! @param[out] rEvalOK Indicates whether evaluation was successful or not
//! @return The value of the evaluated expression
double Expression::evaluate(VariableStorage &rVariableStorage, bool &rEvalOK) const
{
    bool lhsOK=false,rhsOK=false;
    double value=0;

    // If this is a numeric constant value, then return it
    if (mIsNumericConstant)
    {
        rEvalOK = true;
        return mNumericConstantValue;
    }
    // The expression seems to be a named value or variable name
    else if (mIsNamedValue)
    {
        lhsOK=true;
        // Lookup named value or variable in the variable storage instead
        value = rVariableStorage.value(mRightExpressionString, rhsOK);
    }
    else if (mOperator == AssignmentT)
    {
        // Try to assign variable
        bool dummy;
        value = mRightChildExpressions.front().evaluate(rVariableStorage, rhsOK);
        if (rhsOK)
        {
           lhsOK = rVariableStorage.setVariable(mLeftExpressionString, value, dummy);
        }
    }
    else if (mOperator == PowerT)
    {
        // Evaluate both sides
        double base = mLeftChildExpressions.front().evaluate(rVariableStorage, lhsOK);
        double exp = mRightChildExpressions.front().evaluate(rVariableStorage, rhsOK);
        value = pow(base,exp);
    }
    else if (mOperator == LessThenT)
    {
        // Evaluate both sides
        double l = mLeftChildExpressions.front().evaluate(rVariableStorage, lhsOK);
        double r = mRightChildExpressions.front().evaluate(rVariableStorage, rhsOK);
        value = double(l<r);
    }
    else if (mOperator == GreaterThenT)
    {
        // Evaluate both sides
        double l = mLeftChildExpressions.front().evaluate(rVariableStorage, lhsOK);
        double r = mRightChildExpressions.front().evaluate(rVariableStorage, rhsOK);
        value = double(l>r);
    }
    else if (mOperator == FunctionCallT)
    {
        lhsOK=true;
        value = gFunctionHandler.callFunction(mFunctionId, mRightChildExpressions, rVariableStorage, rhsOK);
    }
    else
    {
        lhsOK=true;
        std::list<Expression>::const_iterator it;
        for (it=mRightChildExpressions.begin(); it!=mRightChildExpressions.end(); ++it)
        {
            ExpressionOperatorT optype = it->operatorType();
            double newValue = it->evaluate(rVariableStorage, rhsOK);
            if (optype == AdditionT )
            {
                value += newValue;
            }
            else if (optype == SubtractionT)
            {
                value -= newValue;
            }
            else if (optype == MultiplicationT)
            {
                value *= newValue;
            }
            else if (optype == DivisionT)
            {
                value /= newValue;
            }
            else if (optype == OrT)
            {
                value = boolify(boolify(value)+boolify(newValue));
            }
            else if (optype == AndT)
            {
                value = boolify(value)*boolify(newValue);
            }
            else if (optype != UndefinedT)
            {
                value = newValue;
            }
            else
            {
                rEvalOK=false;
                return value;
            }
            // If evaluation error in child expression, abort and return false
            if (!rhsOK)
            {
                rEvalOK=false;
                return value;
            }
        }
    }

    rEvalOK = (lhsOK && rhsOK);
    return value;
}

//! @brief Extract all named values from expression
//! @param[out] rNamedValues All named values (including constants such as pi and invalid variable names)
void Expression::extractNamedValues(std::set<std::string> &rNamedValues) const
{
    std::list<Expression>::const_iterator it;
    for (it=mRightChildExpressions.begin(); it!=mRightChildExpressions.end(); ++it) {
        if(it->isNamedValue()) {
            rNamedValues.insert(it->exprString());
        }
        it->extractNamedValues(rNamedValues);
    }
    for (it=mLeftChildExpressions.begin(); it!=mLeftChildExpressions.end(); ++it) {
        if(it->isNamedValue()) {
            rNamedValues.insert(it->exprString());
        }
        it->extractNamedValues(rNamedValues);
    }
    if (mOperator == AssignmentT) {
        rNamedValues.insert(mLeftExpressionString);
    }
    if (mIsNamedValue) {
        rNamedValues.insert(mRightExpressionString);
    }
}

//! @brief Extract named values that have a valid match in the VariableStorage
//! @param[in] variableStorage The variable storage in which to look for variable names
//! @param[out] rVariableNames A set of unique variable names from the expression, named values (constants are not included)
void Expression::extractValidVariableNames(const VariableStorage &variableStorage, std::set<std::string> &rVariableNames) const
{
    extractNamedValues(rVariableNames);
    std::set<std::string>::iterator it;
    for (it=rVariableNames.begin(); it!=rVariableNames.end(); ) {
        if (!variableStorage.hasVariableName(*it)) {
            rVariableNames.erase(it++);
        } else {
            ++it;
        }
    }
}

//! @brief Replace named values (rename them)
//! @param[in] oldName The current name of a value
//! @param[out] newName The new name of that value
void Expression::replaceNamedValue(const std::string &oldName, const std::string &newName)
{
    if ((mOperator == AssignmentT) && (mLeftExpressionString == oldName)) {
        mLeftExpressionString = newName;
    }
    if (mIsNamedValue && mRightExpressionString == oldName) {
        mRightExpressionString = newName;
    }
    // Recursively search for any occurance of old value and replace it
    std::list<Expression>::iterator it;
    for (it=mRightChildExpressions.begin(); it!=mRightChildExpressions.end(); ++it) {
        it->replaceNamedValue(oldName, newName);
    }
    for (it=mLeftChildExpressions.begin(); it!=mLeftChildExpressions.end(); ++it) {
        it->replaceNamedValue(oldName, newName);
    }
}

//! @brief Prints the expression (as it will be evaluated) to a string
std::string Expression::print()
{
    std::string fullexp;

    if (mOperator == AssignmentT)
    {
        std::string r = mRightChildExpressions.front().print();
        if (mHadRightOuterParanthesis)
        {
            r="("+r+")";
        }
        fullexp = mLeftExpressionString+"="+r;
    }
    else if (mOperator == PowerT)
    {
        std::string l = mLeftChildExpressions.front().print();
        std::string r = mRightChildExpressions.front().print();
        if (mHadLeftOuterParanthesis)
        {
            l="("+l+")";
        }
        if (mHadRightOuterParanthesis)
        {
            r="("+r+")";
        }
        fullexp = l+"^"+r;
    }
    else if (mOperator == LessThenT)
    {
        std::string l = mLeftChildExpressions.front().print();
        std::string r = mRightChildExpressions.front().print();
        if (mHadLeftOuterParanthesis)
        {
            l="("+l+")";
        }
        if (mHadRightOuterParanthesis)
        {
            r="("+r+")";
        }
        fullexp = l+"<"+r;
    }
    else if (mOperator == GreaterThenT)
    {
        std::string l = mLeftChildExpressions.front().print();
        std::string r = mRightChildExpressions.front().print();
        if (mHadLeftOuterParanthesis)
        {
            l="("+l+")";
        }
        if (mHadRightOuterParanthesis)
        {
            r="("+r+")";
        }
        fullexp = l+">"+r;
    }
    else if (mOperator == FunctionCallT)
    {
        fullexp = mLeftExpressionString+'(';
        std::list<Expression>::iterator it;
        for (it=mRightChildExpressions.begin(); it!=mRightChildExpressions.end(); ++it) {
            fullexp += it->print();
            fullexp += ',';
        }
        fullexp[fullexp.size()-1] = ')';
    }
    else if (isValue())
    {
        fullexp = mRightExpressionString;
        if (mHadRightOuterParanthesis)
        {
            fullexp = "("+fullexp+")";
        }
    }
    else
    {
        std::list<Expression>::iterator it;
        for (it=mRightChildExpressions.begin(); it!=mRightChildExpressions.end(); ++it)
        {
            ExpressionOperatorT optype = it->operatorType();
            if (optype == AdditionT)
            {
                fullexp += "+";
            }
            else if (optype == SubtractionT)
            {
                fullexp += "-";
            }
            else if (optype == MultiplicationT)
            {
                fullexp += "*";
            }
            else if (optype == DivisionT)
            {
                fullexp += "/";
            }
            else if (optype == OrT)
            {
                fullexp += "|";
            }
            else if (optype == AndT)
            {
                fullexp += "&";
            }
            fullexp += it->print();
        }
        stripInitialPlus(fullexp);
        if (mHadRightOuterParanthesis)
        {
            fullexp = "("+fullexp+")";
        }
    }
    return fullexp;
}

void Expression::commonConstructorCode()
{
    mOperator = UndefinedT;
    mHadRightOuterParanthesis = false;
    mHadLeftOuterParanthesis = false;
    mIsNumericConstant = false;
    mIsNamedValue = false;
    mIsValid = false;
    mFunctionId = -1;
    mNumericConstantValue = 0;
}

//! @brief Copy from other expression (help function for assignment and copy constructor)
//! @param[in] other The expression to copy from
void Expression::copyFromOther(const Expression &other)
{
    mOperator = other.mOperator;
    mHadLeftOuterParanthesis = other.mHadLeftOuterParanthesis;
    mHadRightOuterParanthesis = other.mHadRightOuterParanthesis;
    mLeftChildExpressions = other.mLeftChildExpressions;
    mRightChildExpressions = other.mRightChildExpressions;
    mLeftExpressionString = other.mLeftExpressionString;
    mRightExpressionString = other.mRightExpressionString;
    mIsNumericConstant = other.mIsNumericConstant;
    mIsNamedValue = other.mIsNamedValue;
    mNumericConstantValue = other.mNumericConstantValue;
    mFunctionId = other.mFunctionId;
    mIsValid = other.mIsValid;
}

std::vector<std::string> getRegisteredFunctionNames()
{
    return gFunctionHandler.registeredFunctionNames();
}

}
