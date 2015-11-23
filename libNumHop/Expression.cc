#include "Expression.h"
#include "Helpfunctions.h"
#include <cstdlib>
#include <cmath>

namespace numhop {

//! @brief Default constructor
Expression::Expression()
{
    mOperator = UndefinedT;
    mHadOuterParanthesis = false;
    mpRhs = 0;
    mpLhs = 0;
}

//! @brief Constructor
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

//! @brief Copy constructor
Expression::Expression(const Expression &other)
{
    mHadOuterParanthesis = false;
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
    if (isValue())
    {
        rhsOK=true;
        char* pEnd;
        value = strtod(mExpressionString.c_str(), &pEnd);
        lhsOK = (pEnd != mExpressionString.c_str());
        if (!lhsOK)
        {
            value = rVariableStorage.value(mExpressionString, lhsOK);
            //need to avaluate in some other way
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
        fullexp = mExpressionString;
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
            if (optype == AdditionT || optype == SubtractionT)
            {
                // allow +a or -a expressions by adding 0 left hand side
                std::string lhs = exprString.substr(0, i);
                if (lhs.empty())
                {
                    lhs = "0";
                }
                rExp = Expression(exprString, optype, lhs, exprString.substr(i+1) );
                rExp.setHadOuterParanthesis(hadParanthesis);
                return true;
            }
            else if (optype != UndefinedT)
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

//! @brief Process an expression string recurseivly to build an expresssion tree
//! @param[in] exprString The expression string to process
//! @param[out] rExp The reuslting expression tree
bool interpretExpressionStringRecursive(std::string exprString, Expression &rExp)
{
    Expression e;
    branchExpressionOnOperator(exprString, "=", e);
    if (e.empty())
    {
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

}
