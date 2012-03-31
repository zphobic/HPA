//-----------------------------------------------------------------------------
// $Id: util.cpp,v 1.1 2002/10/17 22:13:33 emarkus Exp $
// $Source: /usr/cvsroot/project_pathfind/util.cpp,v $
//-----------------------------------------------------------------------------

#include "util.h"

#include <sstream>

using namespace std;
using namespace PathFind;

//-----------------------------------------------------------------------------

LineReader::LineReader(istream& in)
    : m_lineNumber(0),
      m_in(in)
{
}

Error LineReader::createError(const string& message)
{
    ostringstream out;
    out << "Line " << m_lineNumber << ": " << message << ends;
    return Error(out.str());
}

std::string LineReader::readLine()
{
    char buffer[MAX_LINE];
    m_in.getline(buffer, MAX_LINE);
    if (! m_in)
        throw Error("Unexpected end of stream.");
    ++m_lineNumber;
    return string(buffer);
}

//-----------------------------------------------------------------------------
