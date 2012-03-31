//-----------------------------------------------------------------------------
// $Id: tiling.cpp,v 1.37 2002/11/26 23:39:05 emarkus Exp $
// $Source: /usr/cvsroot/project_pathfind/tiling.cpp,v $
//-----------------------------------------------------------------------------
#include <assert.h>
#include "absnode.h"

#include <ctype.h>
#include <iostream>
#include <sstream>
#include "util.h"

using namespace std;
using namespace PathFind;


//-----------------------------------------------------------------------------

AbsNode::AbsNode(int id, int clusterId, int row, int column, int center)
    :m_id(id),
     m_level(-1),
     m_clusterId(clusterId),
     m_row(row),
     m_column(column),
     m_center(center)
{}

AbsNode::AbsNode()
{}

AbsNode::~AbsNode()
{}


//-----------------------------------------------------------------------------

