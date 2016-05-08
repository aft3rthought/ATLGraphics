

#pragma once

#include <string>

class NomFiles
{
private:
    std::string pm_rootDirectory;
    
public:
    const std::string & rootDirectory() const { return pm_rootDirectory; }
    
    NomFiles(const std::string & in_rootDirectory);
};