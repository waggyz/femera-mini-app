#ifndef FMR_HAS_FMRDATA_HPP
#define FMR_HAS_FMRDATA_HPP

#define FMR_ALIGN_INTS alignof(size_t)
#define FMR_ALIGN_VALS alignof(__m256d)

#ifndef FMR_RESTRICT
#define FMR_RESTRICT __restrict
#endif
#define FMR_ARRAY_PTR auto* FMR_RESTRICT
#define FMR_CONST_PTR const auto* FMR_RESTRICT
#define FMR_ALIGN_PTR __attribute__((aligned(FMR_ALIGN_VALS))) auto* FMR_RESTRICT

namespace zyc { namespace data {
  enum class Layout      : int8_t { Unknown =0,// for arrays of Zomplex
    Vector=1,// AoS (array of structs, interleaved real & imaginary parts)
    Block =2,// SoA (struct of arrays, arrays of real & each imaginary part)
    Native=3 // Native is for real & built-in complex type
  };
} }// end zyc::data namespace

//end FMR_HAS_FMRDATA_HPP
#endif
