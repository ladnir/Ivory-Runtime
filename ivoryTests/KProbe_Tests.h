#pragma once

#define ENCODABLE_KPROBE

void KProbe_Build_Test_Impl();
void KProbe_XORTransitive_Test_Impl();
void KProbe_SaveLoad_Test_Impl();
void KProbe_BlockBitVector_Test_Impl();

#ifdef ENCODABLE_KPROBE
void KProbe_BitVector_Test_Impl();
void KProbe_ZeroLabels_Test_Impl();
void KProbe_Labels_Test_Impl();
#endif