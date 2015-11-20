#include "Expression.h"
#include "Helpfunctions.h"
#include <cstdlib>
#include <cmath>

namespace numhop {

Expression::Expression()
{
    mOperator = UndefinedT;
    mHadOuterParanthesis = false;
    mpRhs = 0;
    mpLhs = 0;
}

Expression::Expression(const std::string &exprString, ExpressionOperatorT op, const std::string &lhs, const std::string &rhs)
{
    mHadOuterParanthesis = false;
    mpRhs = 0;
    mpLhs = 0;
    mExpressionString = exprString;
    stripLeadingTrailingWhitespaces(mExpressionString);
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

Expression::Expression(const Expression &other)
{
    mHadOuterParanthesis = false;
    mpRhs = 0;
    mpLhs = 0;
    copyFromOther(other);
}

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

bool Expression::empty() const
{
    return mExpressionString.empty();
}

bool Expression::isValue() const
{
    return mOperator == ValueT;
}

const std::string &Expression::exprString() const
{
    return mExpressionString;
}

ExpressionOperatorT Expression::operatorType() const
{
    return mOperator;
}

double Expression::evaluate(VariableStorage &variableStorage, bool &rEvalOK)
{
    bool rhsOK, lhsOK;
    double value;
    if (isValue())
    {
        rhsOK=true;
        char* pEnd;
        value = strtod(mExpressionString.c_str(), &pEnd);
        lhsOK = (pEnd != mExpressionString.c_str());
        if (!lhsOK)
        {
            value = variableStorage.value(mExpressionString, lhsOK);
            //need to avaluate in some other way
        }
    }
    else if (mOperator == AdditionT)
    {
        value = mpLhs->evaluate(variableStorage, rhsOK) + mpRhs->evaluate(variableStorage, lhsOK);
    }
    else if (mOperator == SubtractionT)
    {
        value = mpLhs->evaluate(variableStorage, rhsOK) - mpRhs->evaluate(variableStorage, lhsOK);
    }
    else if (mOperator == MultiplicationT)
    {
        value = mpLhs->evaluate(variableStorage, rhsOK) * mpRhs->evaluate(variableStorage, lhsOK);
    }
    else if (mOperator == DivisionT)
    {
        value = mpLhs->evaluate(variableStorage, rhsOK) / mpRhs->evaluate(variableStorage, lhsOK);
    }
    else if (mOperator == PowerT)
    {
        value = pow(mpLhs->evaluate(variableStorage, rhsOK), mpRhs->evaluate(variableStorage, lhsOK));
    }
    else if (mOperator == AssignmentT)
    {
        // Try to assign variable
        bool dummy;
        value = mpRhs->evaluate(variableStorage, lhsOK);
        if (lhsOK)
        {
           rhsOK = variableStorage.setVariable(mpLhs->exprString(), value, dummy);
        }
    }

    rEvalOK = (rhsOK & lhsOK);
    return value;
}

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
        fullexp = mExpressionString;
    }

    if (mHadOuterParanthesis)
    {
        fullexp = "("+fullexp+")";
    }

    return fullexp;
}

void Expression::copyFromOther(const Expression &other)
{
    mExpressionString = other.mExpressionString;
    mOperator = other.mOperator;
    mHadOuterParanthesis = other.mHadOuterParanthesis;
    if (other.mpLhs)
    {
        mpLhs = new Expression(*other.mpLhs);
    }
    if (other.mpRhs)
    {
        mpRhs = new Expression(*other.mpRhs);
    }
}

bool branchExpressionOnOperator(std::string exprString, const std::string &evalOperators, Expression &rExp)
{
    stripLeadingTrailingWhitespaces(exprString);
    bool hadParanthesis;
    stripLeadingTrailingParanthesis(exprString, hadParanthesis);

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
            }
            else if ( (c == '-') && contains(evalOperators, '-') )
            {
                optype = SubtractionT;
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
                rExp.mHadOuterParanthesis = hadParanthesis;
                return true;
            }
        }
    }

    // Nothing found
    rExp = Expression();
    return true;
}

bool interpretExpressionStringRecursive(std::string exprString, Expression &rExp)
{
    Expression e;
    branchExpressionOnOperator(exprString, "=", e);
    if (e.empty())
    {
        branchExpressionOnOperator(exprString, "+-", e);
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

    rExp = e;
    return true;
}

}
