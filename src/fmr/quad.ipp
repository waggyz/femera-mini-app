#ifndef FMR_HAS_MATH_QUAD_IPP
#define FMR_HAS_MATH_QUAD_IPP

//#define FMR_DEBUG

namespace fmr { namespace math {

template <std::uint_fast8_t N, std::uint_fast8_t D, typename F, typename Enable>
constexpr F gaus_lege_quad<N, D, F, Enable>::point_value(const fuint i) {// moved this mess to the end
    return N == 1 ? F(0.0) :
        N == 2 ? (i == 0 ? F(0.57735026918962576451) : F(-0.57735026918962576451)) :
        N == 3 ? (i == 0 ? F(0.0) :
                  i == 1 ? F(0.77459666924148337704) : F(-0.77459666924148337704)) :
        N == 4 ? (i == 0 ? F(0.86113631159405257522) :
                  i == 1 ? F(0.33998104358485626480) :
                  i == 2 ? F(-0.33998104358485626480) : F(-0.86113631159405257522)) :
        (i == 0 ? F(0.0) :
         i == 1 ? F(0.53846931010568309104) :
         i == 2 ? F(-0.53846931010568309104) :
         i == 3 ? F(0.90617984593866399280) : F(-0.90617984593866399280));
  }
template <std::uint_fast8_t N, std::uint_fast8_t D, typename F, typename Enable>
constexpr F gaus_lege_quad<N, D, F, Enable>::weight_value(
    const std::uint_fast8_t i) {
    return N == 1 ? F(2.0) :
        N == 2 ? F(1.0) :
        N == 3 ? (i == 0 ? F(8.0 / 9.0) : F(5.0 / 9.0)) :
        N == 4 ? (i == 0 || i == 3 ? F(0.34785484513745385737) :
                  F(0.65214515486254614263)) :
        (i == 0 ? F(128.0 / 225.0) :
         i == 1 || i == 2 ? F(0.47862867049936646804) :
         F(0.23692688505618908751));
  }

} }//end fmr::math namespace
#undef FMR_DEBUG
//end FMR_HAS_MATH_QUAD_IPP
#endif
