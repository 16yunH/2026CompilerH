#ifndef __FLOW2XML_HH
#define __FLOW2XML_HH

#include <set>

#include "flowinfo.hh"

bool flowinfo2xml(set<FuncFlowInfo*>* allFuncFlow, const char* filename);

#endif // __FLOW2XML_HH