#ifndef _PRINT_HPP_
#define _PRINT_HPP_

#include "seal/seal.h"

void PrintFrame(const SealFrame * frame, const SealCore * core);
void PrintComponent(const SealFrameCompInst * ci, const SealCore * core);

#endif
