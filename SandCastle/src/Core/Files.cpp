#include "pch.h"
#include "SandCastle/Core/Files.h"

namespace SandCastle
{
    namespace Files
    {
        std::string IfstreamToString(std::ifstream& file)
        {
            std::ostringstream stream;
            stream << file.rdbuf();
            return stream.str();
        }
    }
}