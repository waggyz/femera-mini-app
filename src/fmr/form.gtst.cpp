#include "form.hpp"

#include <limits>

#include "../femera/main-early.gtst.ipp"

TEST(FmrForm, FormatNoPrefix) {
  EXPECT_EQ(fmr::form:: si_unit_string (12.3456,"m"), "  12  m");
}
TEST(FmrForm, FormatMega) {
  EXPECT_EQ(fmr::form:: si_unit_string (12.3456e6,"m"), "  12 Mm");
}
TEST(FmrForm, FormatMilli) {
  EXPECT_EQ(fmr::form:: si_unit_string (12.3456e-3,"m"), "  12 mm");
}
TEST(FmrForm, FormatMicro) {
#ifdef FMR_MICRO_UCHAR
  EXPECT_NE(fmr::form:: si_unit_string (12.3456e-6,"m"), "  12 um");
#else
  EXPECT_EQ(fmr::form:: si_unit_string (12.3456e-6,"m"), "  12 um");
#endif
}
TEST(FmrForm, FormatMinDig2) {
  EXPECT_EQ(fmr::form:: si_unit_string (1.23456,"m"  ), "1235 mm");
  EXPECT_EQ(fmr::form:: si_unit_string (1.23456,"m",2), "1235 mm");
}
TEST(FmrForm, FormatMinDig1) {
  EXPECT_EQ(fmr::form:: si_unit_string (1.23456,"m",1), "   1  m");
}
TEST(FmrForm, FormatNegative) {
  EXPECT_EQ(fmr::form:: si_unit_string (-1.23456,"m"  ), "-1235 mm");
  EXPECT_EQ(fmr::form:: si_unit_string (-1.23456,"m",1), "-   1  m");
}
TEST(FmrForm, FormatNear10) {
  EXPECT_EQ(fmr::form:: si_unit_string (9.99,"m"  ), "  10  m");
  EXPECT_EQ(fmr::form:: si_unit_string (9.50,"m",2), "  10  m");
  EXPECT_EQ(fmr::form:: si_unit_string (9.40,"m",2), "9400 mm");
  EXPECT_EQ(fmr::form:: si_unit_string (9.50,"m",1), "  10  m");
  EXPECT_EQ(fmr::form:: si_unit_string (9.40,"m",1), "   9  m");
}
TEST(FmrForm, FormatLongName) {
  EXPECT_EQ(fmr::form:: si_unit_string (12.3456,"c23456789"), "  12  c234567");
}
