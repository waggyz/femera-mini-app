#include "../femera.hpp"
#include "Free.hpp"
#include "elem/Bars.hpp"

#include <gtest/gtest.h>

#include <array>

auto  mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  mini->init (& argc, argv);
  //
  return mini->exit ();
}

namespace femera { namespace grid { namespace free {
namespace {

template <fmr::Dim_int P>
void check_connectivity_copy () {
  using Mesh = Free<elem::Bars, P>;
  constexpr auto conn_n = Mesh::get_elem_node_n();
  const fmr::Local_int master[] = {
    19, 7, 31, 42, 8, 53, 24, 61, 9, 37, 68, 15
  };
  std::array<fmr::Local_int, conn_n + 2> buffer;
  for (fmr::Local_int elem_i = 0; elem_i < 3; ++elem_i) {
    buffer.fill(99);
    Mesh::get_elem_conn(&buffer[1], master, elem_i);
    for (fmr::Local_int i = 0; i < conn_n; ++i) {
      EXPECT_EQ(buffer[std::size_t(i) + 1], master[elem_i * conn_n + i]);
    }
    EXPECT_EQ(buffer.front(), 99u);
    EXPECT_EQ(buffer.back(), 99u);
  }
}

template <fmr::Dim_int P, typename Y>
void check_geometric_copy () {
  using Mesh = Free<elem::Bars, P>;
  for (fmr::Dim_int sims_d = 1; sims_d <= 3; ++sims_d) {
    const auto jacd_size = Mesh::get_elem_jacd_size(sims_d);
    const Y master[] = {
      Y(2), Y(3), Y(5), Y(7), Y(11), Y(13),
      Y(17), Y(19), Y(23), Y(29), Y(31), Y(37)
    };
    std::array<Y, 6> buffer;
    for (fmr::Local_int elem_i = 0; elem_i < 3; ++elem_i) {
      buffer.fill(Y(-1));
      Mesh::get_elem_jacd(&buffer[1], master, jacd_size, elem_i);
      for (fmr::Local_int i = 0; i < jacd_size; ++i) {
        EXPECT_EQ(buffer[std::size_t(i) + 1], master[elem_i * jacd_size + i]);
      }
      EXPECT_EQ(buffer.front(), Y(-1));
      for (std::size_t i = std::size_t(jacd_size) + 1; i < buffer.size(); ++i) {
        EXPECT_EQ(buffer[i], Y(-1));
      }
    }
    const auto previous = buffer;
    Mesh::get_elem_jacd(buffer.data(), master, 0, 2);
    EXPECT_EQ(buffer, previous);
    Mesh::template get_elem_jacd<Y>(nullptr, nullptr, 0, 2);
  }
}

TEST(FreeBars, CopiesFullOrderConnectivity) {
  check_connectivity_copy<1>();
  check_connectivity_copy<2>();
  check_connectivity_copy<3>();
}

TEST(FreeBars, CopiesOneGeometricRecordPerElement) {
  check_geometric_copy<1, float>();
  check_geometric_copy<3, double>();
}

}//end anonymous namespace
} } }//end femera::grid::free:: namespace
