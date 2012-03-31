//-----------------------------------------------------------------------------
// $Id: tiling.cpp,v 1.37 2002/11/26 23:39:05 emarkus Exp $
// $Source: /usr/cvsroot/project_pathfind/tiling.cpp,v $
//-----------------------------------------------------------------------------
#include <assert.h>
#include "cluster.h"
#include <memory>
#include <ctype.h>
#include <iostream>
#include <sstream>
#include "util.h"
#include <algorithm>

using namespace std;
using namespace PathFind;

//-----------------------------------------------------------------------------

HCluster::HCluster(int id, 
                   int row, int col,
                   int horizOrigin, int vertOrigin, int width, int height)
    :m_id(id),
     m_row(row),
     m_column(col),
     m_horizOrigin(horizOrigin),
     m_vertOrigin(vertOrigin),
     m_width(width),
     m_height(height)
{
}

HCluster::~HCluster()
{}


//-----------------------------------------------------------------------------
