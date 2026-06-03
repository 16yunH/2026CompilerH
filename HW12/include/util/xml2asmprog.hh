#ifndef __XML2ASMPROG_HH
#define __XML2ASMPROG_HH

#include "asmprog.hh"

namespace instr {

// Returns nullptr on error
AsmProg* xml2asmprog(const char* filename);

} // namespace instr

#endif // __XML2ASMPROG_HH
