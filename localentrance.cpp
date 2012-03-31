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

using namespace std;
using namespace PathFind;

//-----------------------------------------------------------------------------

LocalEntrance::LocalEntrance()
{}


LocalEntrance::LocalEntrance(int id, int absNodeId, int localIdx, int centerRow, int centerCol, int length)
    :m_id(id),
     m_absNodeId(absNodeId),
     m_localIdx(localIdx),
     m_centerRow(centerRow),
     m_centerCol(centerCol),
     m_length(length)
{}

LocalEntrance::~LocalEntrance()
{}

//-----------------------------------------------------------------------------
