#include "netasciiparser.hpp"

void convertToNetascii( std::vector<char> &buffer )
{
    for(size_t i = 0; i < buffer.size(); ++i)
    {
        if(buffer[i] == '\r' && i + 1 < buffer.size() && buffer[i + 1] == '\n')
        {
            buffer[i] = '\n';
            buffer.erase(buffer.begin() + i + 1);
        }
    }

}

/*char *convertFromNetascii( char *buffer )
{
    size_t length = sizeof(buffer);
    for(size_t i = 0; i < length; ++i)
    {
        if(buffer[i] == '\n')
        {
            buffer[i] = '\r';
            memmove(buffer + i + 1, buffer + i, length -i - 1 );
            buffer[i + 1] = '\n';

            ++length;
        }
    }

    return convertedData;
    
}*/