#ifndef ZYC_HAS_ZYC_IPP
#define ZYC_HAS_ZYC_IPP

static inline
uint zyc::hamw (long unsigned int i){
  return uint(__builtin_popcountll (i));
}
static inline
uint zyc::hamw (uint i){
  return uint(__builtin_popcount (i));
}

//end ZYC_HAS_ZYC_IPP
#endif
