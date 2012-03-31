//-----------------------------------------------------------------------------
// $Id: util.h,v 1.4 2002/11/26 23:29:43 emarkus Exp $
// $Source: /usr/cvsroot/project_pathfind/util.h,v $
//-----------------------------------------------------------------------------

#ifndef PATHFIND_UTIL_H
#define PATHFIND_UTIL_H

#include <iostream>
#include <string>
#include "error.h"
#include "math.h"

//-----------------------------------------------------------------------------

namespace PathFind
{
    /** Wrapper class around std::istream for reading line by line.
        Allows keeping track of the line number for generating meaningful
        error messages.
    */
    class LineReader
    {
    public:
        static const int MAX_LINE = 2048;

        LineReader(std::istream& in);

        /** Creates a new error with a message and the current line number */
        Error createError(const string& message);

        int getLineNumber()
        {
            return m_lineNumber;
        }

        std::string readLine();

    private:
        int m_lineNumber;
        std::istream& m_in;
    };
}

//-----------------------------------------------------------------------------

#endif
