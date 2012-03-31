//-----------------------------------------------------------------------------
// $Id: error.cpp,v 1.2 2002/07/24 00:38:05 emarkus Exp $
// $Source: /usr/cvsroot/project_pathfind/error.cpp,v $
//-----------------------------------------------------------------------------

#include "error.h"

#include <sstream>

using namespace PathFind;

//-----------------------------------------------------------------------------

Error::~Error() throw()
{
    ;
}

//-----------------------------------------------------------------------------

ReadError::ReadError(int line, const string& info)
{
    ostringstream s;
    s << "Read error line " << line << ": " << info << ends;
    setMessage(s.str());
}

//-----------------------------------------------------------------------------

