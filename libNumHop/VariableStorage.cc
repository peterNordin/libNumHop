#include "VariableStorage.h"
#include "Helpfunctions.h"

namespace numhop {


VariableStorage::VariableStorage()
{
    mpExternalStorage = 0;
    mpParentStorage = 0;
}

bool VariableStorage::setVariable(const std::string &name, double value, bool &rDidSetExternally)
{
    // First try to set it externally
    rDidSetExternally = false;
    if (mpExternalStorage)
    {
        rDidSetExternally = mpExternalStorage->setExternalValue(name, value);
    }
    // If we could not set externally, then set it internally
    if (isNameInternalValid(name))
    {
        std::map<std::string,double>::iterator it = mVariableMap.find(name);
        if (it == mVariableMap.end())
        {
            mVariableMap.insert(std::pair<std::string,double>(name, value));
            return true;
        }
        else
        {
            it->second = value;
            return true;
        }
    }
    return rDidSetExternally;
}

bool VariableStorage::isNameInternalValid(const std::string &name) const
{
    for (size_t i=0; i<mDissallowedInternalNameChars.size(); ++i)
    {
        if (contains(name, mDissallowedInternalNameChars[i]))
        {
            return false;
        }
    }
    return true;
}

void VariableStorage::setDissallowedInternalNameCharacters(const std::string &dissallowed)
{
    mDissallowedInternalNameChars = dissallowed;
}

double VariableStorage::value(const std::string &name, bool &rFound) const
{
    // First try to find variable internally
    rFound=false;
    std::map<std::string, double>::const_iterator it = mVariableMap.find(name);
    if (it != mVariableMap.end())
    {
        rFound=true;
        return it->second;
    }
    // Else try to find it externally
    if (mpExternalStorage)
    {
        double value = mpExternalStorage->externalValue(name, rFound);
        if (rFound)
        {
            return value;
        }
    }
    return 0;
}

void VariableStorage::setExternalStorage(ExternalVariableStorage *pExternalStorage)
{
    mpExternalStorage = pExternalStorage;
}

void VariableStorage::setParentStorage(VariableStorage *pParentStorage)
{
    mpParentStorage = pParentStorage;
}

}
