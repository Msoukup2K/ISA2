/*
*      Autor: Martin Soukup
*      Login: xsouku15
*
*/
#include <vector>
#include <string>
#include <string.h>


//Sending netascii will always be done with a buffer as a vector
// Receiving netascii is an array of chars, cause of the structures

//Convert to or from netascii
void convertToNetascii( std::vector<char> &buffer );

//Convert to or from netascii
void convertFromNetascii( char *buffer );
