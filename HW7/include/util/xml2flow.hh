#ifndef __XML2FLOW_HH
#define __XML2FLOW_HH

#include <set>

#include "flowinfo.hh"

set<FuncFlowInfo*>* xml2flow(const char* filename);

#endif // __XML2FLOW_HH