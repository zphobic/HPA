//-----------------------------------------------------------------------------
/** @file error.h
    Base exception class.

    $Id: error.h,v 1.3 2002/10/18 17:52:41 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/error.h,v $
*/
//-----------------------------------------------------------------------------

#ifndef PATHFIND_ERROR_H
#define PATHFIND_ERROR_H

#include <exception>
#include <string>
#include <vector>

//-----------------------------------------------------------------------------

namespace PathFind
{
    using namespace std;

    class Error
        : public exception
    {
    public:
        Error();

        Error(const string& what);
        
        ~Error() throw();
        
        void setMessage(const string& message);

        const char* what() const throw();
        
    private:
        string m_message;
    };

    class ReadError
        : public Error
    {
    public:
        ReadError(int line, const string& info);
    };
}

//-----------------------------------------------------------------------------
// Inline methods.
//-----------------------------------------------------------------------------

namespace PathFind
{

    inline Error::Error()
    {
        ;
    }
    
    inline Error::Error(const string& what)
        : m_message(what)
    {
        ;
    }
    
    inline void Error::setMessage(const string& message)
    {
        m_message = message;
    }
    
    inline const char* Error::what() const throw()
    {
        return m_message.c_str();
    }
    
}

//-----------------------------------------------------------------------------

#endif
