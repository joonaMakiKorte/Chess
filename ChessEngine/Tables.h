#ifndef TABLES_H
#define TABLES_H

#include "BitboardConstants.h"

// Pre-compute all attack rays between squares
extern uint64_t BETWEEN[64][64];
extern uint64_t LINE[64][64];
extern Direction DIR[64][64];

// Generate all precomputed tables 
void initTables();

#endif