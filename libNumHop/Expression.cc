#include "Expression.h"
#include "Helpfunctions.h"
#include <cstdlib>
#include <cmath>
#include <vector>

namespace numhop {

//! @brief Default constructor
Expression::Expression()
{
    mOperator = UndefinedT;
    mHadOuterParanthesis = false;
    mIsNegative = false;
    mpRhs = 0;
    mpLhs = 0;
}

//! @brief Constructor
Expression::Expression(const std::string &exprString, ExpressionOperatorT op, const std::string &lhs, const std::string &rhs)
{
    mHadOuterParanthesis = false;
    mIsNegative = false;
    mpRhs = 0;
    mpLhs = 0;
    mExpressionString = exprString;
    stripLeadingTrailingWhitespaces(mExpressionString);
    mIsNegative = stripInitialSign(mExpressionString) == '-';
    mOperator = op;
    if (op != ValueT)
    {
        mpLhs = new Expression();
        interpretExpressionStringRecursive(lhs, *mpLhs);
        if (!rhs.empty())
        {
            mpRhs = new Expression();
            interpretExpressionStringRecursive(rhs, *mpRhs);
        }
    }
}

//! @brief Copy constructor
Expression::Expression(const Expression &other)
{
    mHadOuterParanthesis = false;
    mIsNegative = false;
    mpRhs = 0;
    mpLhs = 0;
    copyFromOther(other);
}

//! @brief Destructor
Expression::~Expression()
{
    if (mpLhs)
    {
        delete mpLhs;
    }
    if (mpRhs)
    {
        delete mpRhs;
    }
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
    return mExpressionString.empty();
}

//! @brief Check if this expression represents a value or variable
bool Expression::isValue() const
{
    return mOperator == ValueT;
}

//! @brief Returns the expression string (without outer parenthesis)
const std::string &Expression::exprString() const
{
    return mExpressionString;
}

//! @brief Returns the operator type
ExpressionOperatorT Expression::operatorType() const
{
    return mOperator;
}

//! @brief Evaluate the expression
//! @param[in,out] rVariableStorage The variable storage to use for setting or getting variable values
//! @param[out] rEvalOK Indicates wheter evaluation was successfull or not
//! @return The value of the evaluated expression
double Expression::evaluate(VariableStorage &rVariableStorage, bool &rEvalOK)
{
    bool rhsOK, lhsOK;
    double value;
    if (mOperator == ValueT)
    {
        rhsOK=true;
        char* pEnd;
        value = strtod(mExpressionString.c_str(), &pEnd);
        lhsOK = (pEnd != mExpressionString.c_str());
        if (!lhsOK)
        {
            value = rVariableStorage.value(mExpressionString, lhsOK);
        }
        if(mIsNegative)
        {
            value = -value;
        }
    }
    else if (mOperator == AdditionT)
    {
        value = mpLhs->evaluate(rVariableStorage, rhsOK) + mpRhs->evaluate(rVariableStorage, lhsOK);
    }
    else if (mOperator == SubtractionT)
    {
        value = mpLhs->evaluate(rVariableStorage, rhsOK) - mpRhs->evaluate(rVariableStorage, lhsOK);
    }
    else if (mOperator == MultiplicationT)
    {
        value = mpLhs->evaluate(rVariableStorage, rhsOK) * mpRhs->evaluate(rVariableStorage, lhsOK);
    }
    else if (mOperator == DivisionT)
    {
        value = mpLhs->evaluate(rVariableStorage, rhsOK) / mpRhs->evaluate(rVariableStorage, lhsOK);
    }
    else if (mOperator == PowerT)
    {
        value = pow(mpLhs->evaluate(rVariableStorage, rhsOK), mpRhs->evaluate(rVariableStorage, lhsOK));
    }
    else if (mOperator == AssignmentT)
    {
        // Try to assign variable
        bool dummy;
        value = mpRhs->evaluate(rVariableStorage, lhsOK);
        if (lhsOK)
        {
           rhsOK = rVariableStorage.setVariable(mpLhs->exprString(), value, dummy);
        }
    }

    rEvalOK = (rhsOK & lhsOK);
    return value;
}

//! @brief Prints the expression (as it will be evaluated) to a string
std::string Expression::print()
{
    std::string fullexp;
    if (mpLhs)
    {
        fullexp = mpLhs->print();
    }
    if (mOperator == AssignmentT)
    {
        fullexp += "=";
    }
    else if (mOperator == AdditionT)
    {
        fullexp += "+";
    }
    else if (mOperator == SubtractionT)
    {
        fullexp += "-";
    }
    else if (mOperator == MultiplicationT)
    {
        fullexp += "*";
    }
    else if (mOperator == DivisionT)
    {
        fullexp += "/";
    }
    else if (mOperator == PowerT)
    {
        fullexp += "^";
    }
    if (mpRhs)
    {
        fullexp += mpRhs->print();
    }
    if(isValue())
    {
        if (mIsNegative)
        {
            fullexp = "-"+mExpressionString;
        }
        else
        {
            fullexp = mExpressionString;
        }
    }

    if (mHadOuterParanthesis)
    {
        fullexp = "("+fullexp+")";
    }

    return fullexp;
}

//! @brief Set wheter the expression had outer parenthesis (usefull to know when printing)
void Expression::setHadOuterParanthesis(bool tf)
{
    mHadOuterParanthesis = tf;
}

//! @brief Copy from other expression (helpfunction for assignment and copy constructor)
//! @param[in] other The expression to copy from
void Expression::copyFromOther(const Expression &other)
{
    mExpressionString = other.mExpressionString;
    mOperator = other.mOperator;
    mHadOuterParanthesis = other.mHadOuterParanthesis;
    mIsNegative = other.mIsNegative;
    if (other.mpLhs)
    {
        mpLhs = new Expression(*other.mpLhs);
    }
    if (other.mpRhs)
    {
        mpRhs = new Expression(*other.mpRhs);
    }
}

//! @brief Find an operator and brach the expression tree at this point
//! @param[in] exprString The expression string to process
//! @param[in] evalOperators A string with the operators to search for
//! @param[out] The resulting expresssion (Empty expression if nothing found)
//! @returns False if some error occured else true
bool branchExpressionOnOperator(std::string exprString, const std::string &evalOperators, Expression &rExp)
{
    removeWhitespaces(exprString);
    bool hadParanthesis;
    stripLeadingTrailingParanthesis(exprString, hadParanthesis);
    //fixInitialSign(exprString);

    size_t nOpenParanthesis=0;
    for (size_t i=0; i<exprString.size(); ++i)
    {
        const char &c = exprString[i];

        // Count paranthesis
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
            // Check for assignment, (can only have one assignment in expression)
            ExpressionOperatorT optype=UndefinedT;
            if ( (c == '=') && contains(evalOperators, '=') )
            {
                optype = AssignmentT;
            }
            else if ( (c == '+') && contains(evalOperators, '+') )
            {
                optype = AdditionT;
                if (i == 0)
                {
                    optype = UndefinedT;
                }

            }
            else if ( (c == '-') && contains(evalOperators, '-') )
            {
                optype = SubtractionT;
                if (i == 0)
                {
                    optype = UndefinedT;
                }
            }
            else if ( (c == '*') && contains(evalOperators, '*') )
            {
                optype = MultiplicationT;
            }
            else if ( (c == '/') && contains(evalOperators, '/') )
            {
                optype = DivisionT;
            }
            else if ( (c == '^') && contains(evalOperators, '^') )
            {
                optype = PowerT;
            }

            // Add expression
            if (optype != UndefinedT)
            {
                rExp = Expression(exprString, optype, exprString.substr(0, i), exprString.substr(i+1) );
                rExp.setHadOuterParanthesis(hadParanthesis);
                return true;
            }
        }
    }

    // Nothing found
    rExp = Expression();
    return true;
}

bool branchExpressionOnOperator2(std::string exprString, const std::string &evalOperators, std::list<Expression2> &rExp)
{
    removeWhitespaces(exprString);
    bool hadParanthesis;
    stripLeadingTrailingParanthesis(exprString, hadParanthesis);
    //fixInitialSign(exprString);

    size_t nOpenParanthesis=0;
    size_t s=0;
    std::vector<size_t> breakpts;
    ExpressionOperatorT optype=UndefinedT;

    size_t e=0;
    breakpts.push_back(e);
    for (e=0; e<exprString.size(); ++e)
    {
        const char &c = exprString[e];

        // Count paranthesis
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
            // Check for assignment, (can only have one assignment in expression)
            //ExpressionOperatorT optype=UndefinedT;
            if ( (c == '=') && contains(evalOperators, '=') )
            {
                optype = AssignmentT;
                breakpts.push_back(e);
            }
            else if ( (c == '+') && contains(evalOperators, '+') )
            {
                optype = AdditionT;
                if (e!=0)
                {
                    breakpts.push_back(e);
                }
            }
            else if ( (c == '-') && contains(evalOperators, '-') )
            {
                //! @todo found bool will be enough
                optype = SubtractionT;
                if (e!=0)
                {
                    breakpts.push_back(e);
                }
            }
            else if ( (c == '*') && contains(evalOperators, '*') )
            {
                optype = MultiplicationT;
                breakpts.push_back(e);
            }
            else if ( (c == '/') && contains(evalOperators, '/') )
            {
                optype = DivisionT;
                breakpts.push_back(e);
            }
            else if ( (c == '^') && contains(evalOperators, '^') )
            {
                optype = PowerT;
                breakpts.push_back(e);
            }
        }
    }
    breakpts.push_back(e);

    // Add expression
    if (optype != UndefinedT)
    {
        if (contains(evalOperators, "+-"))
        {
            for (size_t bp=0; bp<breakpts.size()-1; ++bp)
            {
                std::string left = exprString.substr(breakpts[bp], breakpts[bp+1]-breakpts[bp]);
                char sign = stripInitialSign(left);
                if (sign == '-')
                {
                    rExp.push_back(Expression2(left, SubtractionT));
                }
                else if (sign == '+')
                {
                    rExp.push_back(Expression2(left, AdditionT));
                }
            }
        }
        else if (contains(evalOperators, "*/"))
        {
            for (size_t bp=0; bp<breakpts.size()-1; ++bp)
            {
                std::string left = exprString.substr(breakpts[bp], breakpts[bp+1]-breakpts[bp]);
                char op = left[0];
                if (op == '/')
                {
                    rExp.push_back(Expression2(left.substr(1), DivisionT));
                }
                else if (op == '*')
                {
                    rExp.push_back(Expression2(left.substr(1), MultiplicationT));
                }
                else
                {
                    rExp.push_back(Expression2(left, AdditionT));
                }
            }
        }
        else if (contains(evalOperators, "=^"))
        {
//            for (size_t bp=0; bp<breakpts.size()-1; ++bp)
            //! @todo check num breakpoints
            {
                std::string left = exprString.substr(breakpts[0], breakpts[1]-breakpts[0]);
                std::string right = exprString.substr(breakpts[1], breakpts[2]-breakpts[1]);
                char op = right[0];
                if (op == '=')
                {
                    rExp.push_back(Expression2(left, right.substr(1), AssignmentT));
                }
                else
                {
                    rExp.push_back(Expression2(left, right.substr(1), PowerT));
                }

            }
        }
    }


    // Nothing found
    //rExp.push_back(Expression2());
    return true;

}

//! @brief Process an expression string recurseivly to build an expresssion tree
//! @param[in] exprString The expression string to process
//! @param[out] rExp The reuslting expression tree
bool interpretExpressionStringRecursive(std::string exprString, Expression &rExp)
{
    Expression e;
    branchExpressionOnOperator(exprString, "=", e);
    if (e.empty())
    {
        fixMultiSubtraction(exprString);
        branchExpressionOnOperator(exprString, "+", e);
        if (e.empty())
        {
            branchExpressionOnOperator(exprString, "-", e);
            if (e.empty())
            {
                fixMultiDivision(exprString);
                branchExpressionOnOperator(exprString, "*", e);
                if (e.empty())
                {
                    branchExpressionOnOperator(exprString, "/", e);
                    if (e.empty())
                    {
                        branchExpressionOnOperator(exprString, "^", e);
                        if (e.empty())
                        {
                            // This must be a value
                            e = Expression(exprString, ValueT);
                        }
                    }
                }
            }
        }
    }

    rExp = e;
    return true;
}

bool interpretExpressionStringRecursive2(std::string exprString, std::list<Expression2> &rExp)
{
    branchExpressionOnOperator2(exprString, "=", rExp);
    if (rExp.empty())
    {
        branchExpressionOnOperator2(exprString, "+-", rExp);
        if (rExp.empty())
        {
            branchExpressionOnOperator2(exprString, "*/", rExp);
            if (rExp.empty())
            {
                branchExpressionOnOperator2(exprString, "^", rExp);
                if (rExp.empty())
                {
                    // This must be a value
                    rExp.push_back(Expression2(exprString, ValueT));
                }
            }

        }
    }
    return true;
}

Expression2::Expression2()
{
    mOperator = UndefinedT;
    mHadRightOuterParanthesis = false;
    mHadLeftOuterParanthesis = false;
}

Expression2::Expression2(const Expression &other)
{
    copyFromOther(other);
}

Expression2::Expression2(const std::string &exprString, ExpressionOperatorT op)
{
    mRightExpressionString = exprString;
    removeWhitespaces(mRightExpressionString);
    stripLeadingTrailingParanthesis(mRightExpressionString, mHadRightOuterParanthesis);
    mOperator = op;

    if (mOperator != ValueT)
    {
        std::list<Expression2> right;
        interpretExpressionStringRecursive2(mRightExpressionString, right);
        mRightChildExpressions.swap(right);
    }
}

Expression2::Expression2(const std::string &leftExprString, const std::string &rightExprString, ExpressionOperatorT op)
{
    mLeftExpressionString = leftExprString;
    removeWhitespaces(mLeftExpressionString);
    stripLeadingTrailingParanthesis(mLeftExpressionString, mHadLeftOuterParanthesis);
    mRightExpressionString = rightExprString;
    removeWhitespaces(mRightExpressionString);
    stripLeadingTrailingParanthesis(mRightExpressionString, mHadRightOuterParanthesis);
    mOperator = op;

    if (mOperator == AssignmentT)
    {
        mRightChildExpressions.push_back(Expression2(mRightExpressionString, AdditionT));
    }
    else
    {
        mLeftChildExpressions.push_back(Expression2(mLeftExpressionString, AdditionT));
        mRightChildExpressions.push_back(Expression2(mRightExpressionString, AdditionT));
    }
}

Expression2::~Expression2()
{

}

Expression2 &Expression2::operator=(const Expression2 &other)
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

bool Expression2::empty() const
{
    return mRightExpressionString.empty();
}

bool Expression2::isValue() const
{
    return mOperator == ValueT;
}

const std::string &Expression2::exprString() const
{
    return rightExprString();
}

const std::string &Expression2::leftExprString() const
{
    return mLeftExpressionString;
}

const std::string &Expression2::rightExprString() const
{
    return mRightExpressionString;
}

ExpressionOperatorT Expression2::operatorType() const
{
    return mOperator;
}

double Expression2::evaluate(VariableStorage &rVariableStorage, bool &rEvalOK)
{
    bool lhsOK,rhsOK;
    double value=0;

    if (mOperator == ValueT)
    {
        lhsOK=true;
        char* pEnd;
        value = strtod(mRightExpressionString.c_str(), &pEnd);
        rhsOK = (pEnd != mRightExpressionString.c_str());
        if (!rhsOK)
        {
            value = rVariableStorage.value(mRightExpressionString, rhsOK);
        }
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
    else
    {
        lhsOK=true;
        std::list<Expression2>::iterator it;
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
            else if (optype == ValueT || optype == AssignmentT || optype == PowerT)
            {
                value = newValue;
            }
        }
    }

    rEvalOK = (lhsOK && rhsOK);
    return value;
}

std::string Expression2::print()
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
    else if (mOperator == ValueT)
    {
        fullexp = mRightExpressionString;
        if (mHadRightOuterParanthesis)
        {
            fullexp = "("+fullexp+")";
        }
    }
    else
    {
        std::list<Expression2>::iterator it;
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

void Expression2::copyFromOther(const Expression2 &other)
{
    mOperator = other.mOperator;
    mHadLeftOuterParanthesis = other.mHadLeftOuterParanthesis;
    mHadRightOuterParanthesis = other.mHadRightOuterParanthesis;
    mLeftChildExpressions = other.mLeftChildExpressions;
    mRightChildExpressions = other.mRightChildExpressions;
    mLeftExpressionString = other.mLeftExpressionString;
    mRightExpressionString = other.mRightExpressionString;
}

bool interpretExpressionStringRecursive2(std::string exprString, Expression2 &rExp)
{
    rExp = Expression2(exprString, AdditionT);
    return true;
}



}
