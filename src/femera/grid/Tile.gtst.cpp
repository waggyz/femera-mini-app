#include "../femera.hpp"

#include "Tile.hpp"
#include "Free.hpp"
#include "elem/Bars.hpp"
#include "tile/Bar1.hpp"
#include "tile/Bar5.hpp"

#include <gtest/gtest.h>
#include <omp.h>
#include <type_traits>

auto  mini = fmr::new_jobs ();

fmr::Exit_int main (int argc, char** argv) {
  mini->init (&argc, argv);
  //
  //
  return mini->exit ();
}
TEST( TileElem, TrivialTest ){
  EXPECT_EQ( 1, 1 );
}

TEST( TileElem, HeaderCanPrecedeBar1 ){
  EXPECT_TRUE( (std::is_empty<femera::grid::tile::Tile<
    femera::grid::tile::Bar1>>::value) );
}

namespace {

template <class Mesh>
void check_shape_records () {
  const double values[] = {0.375, -0.125, 0.75, -0.125, 0.375, 0.75};
  const double gradients[] = {-2, 0, 2, 0, 2, -2};
  const auto value_width = Mesh::get_elem_shpf_size();
  const auto grad_width = Mesh::get_elem_shpg_size();
  ASSERT_EQ(value_width, 3u);
  ASSERT_EQ(grad_width, 3u);
  EXPECT_EQ(Mesh::get_elem_shpf_size(2), 6u);
  EXPECT_EQ(Mesh::get_elem_shpg_size(2), 6u);
  for (fmr::Local_int ip = 0; ip < 2; ++ip) {
    double value_buffer[] = {-9, 0, 0, 0, -9};
    double grad_buffer[] = {-9, 0, 0, 0, -9};
    Mesh::get_elem_shpf(value_buffer + 1, values, value_width, ip);
    Mesh::get_elem_shpg(grad_buffer + 1, gradients, grad_width, ip);
    for (fmr::Local_int i = 0; i < 3; ++i) {
      EXPECT_DOUBLE_EQ(value_buffer[i + 1], values[ip * 3 + i]);
      EXPECT_DOUBLE_EQ(grad_buffer[i + 1], gradients[ip * 3 + i]);
    }
    EXPECT_DOUBLE_EQ(value_buffer[0], -9);
    EXPECT_DOUBLE_EQ(value_buffer[4], -9);
    EXPECT_DOUBLE_EQ(grad_buffer[0], -9);
    EXPECT_DOUBLE_EQ(grad_buffer[4], -9);
  }
  const double per_element[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  double element_gradients[6] = {};
  Mesh::get_elem_shpg(element_gradients, per_element, Mesh::get_elem_shpg_size(2), 1);
  for (int i = 0; i < 6; ++i) {
    EXPECT_DOUBLE_EQ(element_gradients[i], per_element[i + 6]);
  }
  Mesh::template get_elem_shpf<double>(nullptr, nullptr, 0, 1);
  Mesh::template get_elem_shpg<double>(nullptr, nullptr, 0, 1);
}

template <class Mesh>
void check_shared_constant_gradient () {
  const double gradient[] = {-1, 1};
  ASSERT_EQ(Mesh::get_elem_shpg_size(), 2u);
  for (int elem = 0; elem < 3; ++elem) {
    double result[] = {0, 0};
    Mesh::get_elem_shpg(result, gradient, Mesh::get_elem_shpg_size(), 0);
    EXPECT_DOUBLE_EQ(result[0], -1);
    EXPECT_DOUBLE_EQ(result[1], 1);
  }
}

TEST( TileElem, SharedShapeRecords ){
  using namespace femera::grid;
  check_shape_records<free::Free<elem::Bars, 2>>();
  check_shape_records<tile::Tile<tile::Bar1, 2>>();
  check_shape_records<tile::Tile<tile::Bar5, 2>>();
  check_shared_constant_gradient<free::Free<elem::Bars, 1>>();
  check_shared_constant_gradient<tile::Tile<tile::Bar1, 1>>();
}

}//end anonymous namespace

