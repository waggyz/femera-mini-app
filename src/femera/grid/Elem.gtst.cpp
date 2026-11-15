#include "../femera.hpp"

#include "Elem.test.hpp"

#if 1
// These included temporaily for testing.
// The public API should be able to include only a generic interface to access
// all element types.
#include "elem/Bars.hpp"
#include "elem/Tris.hpp"
#include "elem/Quad.hpp"
#include "elem/Tets.hpp"
#include "elem/Cube.hpp"
#include "elem/Prmd.hpp"
#include "elem/Wdge.hpp"
#include "elem/Itri.hpp"
#include "elem/Ibar.hpp"
#include "elem/Iqud.hpp"
#endif

#include <gtest/gtest.h>
#include <omp.h>

auto  mini_ptr = fmr::new_jobs ();
auto &mini = *mini_ptr;

fmr::Local_int jacs_size=0, jac1_size=0, jac2_size=0, jac3_size=0, jac4_size=0;
fmr::Local_int elem_tet2_node_n=0;

constexpr auto elem_tet1 = femera::grid::elem::Elem<femera::grid::elem::Tets,1> ();
constexpr auto elem_tet2 = femera::grid::elem::Elem<femera::grid::elem::Tets,2> ();
constexpr auto elem_tet3 = femera::grid::elem::Elem<femera::grid::elem::Tets,3> ();

constexpr auto elem_tetf
  = femera::grid::elem::Elem<femera::grid::elem::Tets,2,float> ();
constexpr auto elem_tetd
  = femera::grid::elem::Elem<femera::grid::elem::Tets,2,double> ();
//const auto size_coor_f = sizeof(elem_tetf.get_elem_vert_coor ()[0]);
//const auto size_coor_d = sizeof(elem_tet1.get_elem_vert_coor ()[0]);

const float a_float = float(1.0/6.0);
const float an_array_f[2] = {float(1.0/6.0), float(1.0/6.0)};

// Elem_test *****************************************************************
// Supported element examples
static constexpr femera::grid::elem::Elem_args tri6_2d_args = { 6, 3, 2};
static constexpr femera::grid::elem::Elem_args tri6_3d_args = { 6, 3, 3};
static constexpr femera::grid::elem::Elem_args tet10_args   = {10, 4, 3};

uintptr_t val0_adress=0, val1_adress=0;

constexpr auto test_tet_3d = femera::grid::elem::Elem_test
  <femera::grid::elem::Tets, tet10_args> ();
//****************************************************************************

fmr::Exit_int main (int argc, char** argv) {
  mini.init (&argc, argv);
  //
  const auto elem_quad = femera::grid::elem::Elem<femera::grid::elem::Quad> ();
  const auto elem_bars = femera::grid::elem::Elem<femera::grid::elem::Bars,1> ();
  const auto elem_tris = femera::grid::elem::Elem<femera::grid::elem::Tris,1> ();
  //
  jacs_size = elem_quad.jacs_dets_size ();
  jac1_size = elem_bars.jacs_dets_size (1, 1);// 1D sims space
  jac2_size = elem_tris.jacs_dets_size (1, 2);// 2D sims space
  jac3_size = elem_tet1.jacs_dets_size (1);
  jac4_size = elem_tet1.jacs_dets_size (4);
  //
  elem_tet2_node_n = elem_tet2.get_node_n ();
  //
#pragma omp parallel
  {// Check that variables assigned from constexpr values are thread-local.
    const auto elem = femera::grid::elem::Elem<femera::grid::elem::Tets,2> ();
    const auto val_local = elem.get_node_n ();
    if (omp_get_thread_num () == 0){
      val0_adress = reinterpret_cast<uintptr_t>(&val_local);
    }
    if (omp_get_thread_num () == 1){
      val1_adress = reinterpret_cast<uintptr_t>(&val_local);
    }
  }//end parallel region
  //
  return mini.exit ();
}
TEST( GridElem, TrivialTest ){
  EXPECT_EQ( 1, 1 );
  EXPECT_FLOAT_EQ (a_float, an_array_f[0]);
  EXPECT_FLOAT_EQ (a_float, an_array_f[1]);
}
namespace femera { namespace grid { namespace elem {
TEST( GridElem, SerendipityElementNodeCounts ){
  EXPECT_EQ( (Elem<Bars,1>::get_node_n ()), 2 );
  EXPECT_EQ( (Elem<Bars,2>::get_node_n ()), 3 );
  EXPECT_EQ( (Elem<Bars,3>::get_node_n ()), 4 );
  //
  EXPECT_EQ( (Elem<Tris,1>::get_node_n ()), 3 );
  EXPECT_EQ( (Elem<Tris,2>::get_node_n ()), 6 );
  EXPECT_EQ( (Elem<Tris,3>::get_node_n ()), 10 );
  //
  EXPECT_EQ( (Elem<Quad,1>::get_node_n ()), 4 );
  EXPECT_EQ( (Elem<Quad,2>::get_node_n ()), 8 );
  EXPECT_EQ( (Elem<Quad,3>::get_node_n ()), 12 );
  //
  EXPECT_EQ( (Elem<Tets,1>::get_node_n ()), 4 );
  EXPECT_EQ( (Elem<Tets,2>::get_node_n ()), 10 );
  EXPECT_EQ( (Elem<Tets,3>::get_node_n ()), 20 );
  //
  EXPECT_EQ( (Elem<Cube,1>::get_node_n ()), 8 );
  EXPECT_EQ( (Elem<Cube,2>::get_node_n ()), 20 );
  EXPECT_EQ( (Elem<Cube,3>::get_node_n ()), 32 );
  //
  EXPECT_EQ( (Elem<Prmd,1>::get_node_n ()), 5 );
  EXPECT_EQ( (Elem<Prmd,2>::get_node_n ()), 13 );
  EXPECT_EQ( (Elem<Prmd,3>::get_node_n ()), 21 );
  //
  EXPECT_EQ( (Elem<Wdge,1>::get_node_n ()), 6 );
  EXPECT_EQ( (Elem<Wdge,2>::get_node_n ()), 15 );
  EXPECT_EQ( (Elem<Wdge,3>::get_node_n ()), 26 );
  EXPECT_EQ( (Elem<Ibar,3>::get_node_n ()), 8 );
  EXPECT_EQ( (Elem<Itri,3>::get_node_n ()), 20 );
  EXPECT_EQ( (Elem<Iqud,3>::get_node_n ()), 24 );
}

template <class T, std::size_t N>
void check_solid_topology(const fmr::Local_int (&expected_edges)[N]) {
  using E = Elem<T>;
  const auto* edges = E::get_bars_vert_conn();
  const auto* vertices = T::vert_coor;
  const auto* transposed = T::coor_vert;
  ASSERT_EQ(N, 2u*E::get_edge_n());
  ASSERT_NE(edges, nullptr);
  EXPECT_EQ(E::get_tris_vert_conn() == nullptr, E::get_tris_n() == 0);
  EXPECT_EQ(E::get_quad_vert_conn() == nullptr, E::get_quad_n() == 0);
  EXPECT_EQ(int(E::get_vert_n())-int(E::get_edge_n())+int(E::get_face_n()), 2);
  double center[3] = {};
  for (fmr::Local_int v=0; v<E::get_vert_n(); ++v) {
    for (uint d=0; d<3; ++d) {
      EXPECT_DOUBLE_EQ(vertices[3*v+d], transposed[d*E::get_vert_n()+v]);
      center[d] += vertices[3*v+d]/E::get_vert_n();
    }
  }
  for (std::size_t i=0; i<N; ++i) EXPECT_EQ(edges[i], expected_edges[i]);
  for (fmr::Local_int e=0; e<E::get_edge_n(); ++e) {
    ASSERT_LT(edges[2*e], E::get_vert_n());
    ASSERT_LT(edges[2*e+1], E::get_vert_n());
    ASSERT_NE(edges[2*e], edges[2*e+1]);
    for (fmr::Local_int f=0; f<e; ++f) {
      EXPECT_FALSE((edges[2*e] == edges[2*f] && edges[2*e+1] == edges[2*f+1])
        || (edges[2*e] == edges[2*f+1] && edges[2*e+1] == edges[2*f]));
    }
  }
  int incidence[12] = {};
  int orientation[12] = {};
  for (int kind=0; kind<2; ++kind) {
    const uint size = kind == 0 ? 3 : 4;
    const auto count = kind == 0 ? E::get_tris_n() : E::get_quad_n();
    const auto* faces = kind == 0 ? E::get_tris_vert_conn() : E::get_quad_vert_conn();
    for (fmr::Local_int f=0; f<count; ++f) {
      const auto* face = faces+f*size;
      for (fmr::Local_int previous=0; previous<f; ++previous) {
        uint shared = 0;
        for (uint i=0; i<size; ++i) for (uint j=0; j<size; ++j) {
          if (face[i] == faces[previous*size+j]) ++shared;
        }
        EXPECT_NE(shared, size);
      }
      double centroid[3] = {};
      for (uint i=0; i<size; ++i) {
        ASSERT_LT(face[i], E::get_vert_n());
        for (uint j=0; j<i; ++j) ASSERT_NE(face[i], face[j]);
        for (uint d=0; d<3; ++d) centroid[d] += vertices[3*face[i]+d]/size;
        const auto a = face[i], b = face[(i+1)%size];
        int found = -1;
        for (fmr::Local_int e=0; e<E::get_edge_n(); ++e) {
          if ((edges[2*e] == a && edges[2*e+1] == b)
            || (edges[2*e] == b && edges[2*e+1] == a)) found = int(e);
        }
        ASSERT_GE(found, 0);
        ++incidence[found];
        orientation[found] += edges[2*found] == a ? 1 : -1;
      }
      double first_normal[3] = {};
      for (uint triangle=0; triangle<size-2; ++triangle) {
        double normal[3] = {};
        ASSERT_TRUE(fmr::math::tri3_norm(normal, vertices+3*face[0],
          vertices+3*face[triangle+1], vertices+3*face[triangle+2]));
        double outward = 0, alignment = 0;
        for (uint d=0; d<3; ++d) {
          outward += normal[d]*(centroid[d]-center[d]);
          alignment += normal[d]*first_normal[d];
          if (triangle == 0) first_normal[d] = normal[d];
        }
        EXPECT_GT(outward, 1e-12);
        if (triangle > 0) {
          EXPECT_NEAR(alignment, 1.0, 1e-12);
        }
      }
    }
  }
  for (fmr::Local_int e=0; e<E::get_edge_n(); ++e) {
    EXPECT_EQ(incidence[e], 2);
    EXPECT_EQ(orientation[e], 0);
  }
}

TEST( GridElem, SolidTopologyAndOutwardFaceNormals ){
  const fmr::Local_int tets[] = {0,1, 1,2, 2,0, 3,0, 3,2, 3,1};
  const fmr::Local_int prmd[] = {0,1, 1,2, 2,3, 3,0, 0,4, 1,4, 2,4, 3,4};
  const fmr::Local_int wdge[] = {0,1, 1,2, 2,0, 3,4, 4,5, 5,3, 0,3, 1,4, 2,5};
  const fmr::Local_int cube[] = {0,1, 1,2, 2,3, 3,0, 4,5, 5,6, 6,7, 7,4,
    0,4, 1,5, 2,6, 3,7};
  check_solid_topology<Tets>(tets);
  check_solid_topology<Prmd>(prmd);
  check_solid_topology<Wdge>(wdge);
  check_solid_topology<Cube>(cube);
}

template <class T, std::size_t N>
void check_planar_topology(const fmr::Local_int (&expected_edges)[N],
  const fmr::Local_int* face, fmr::Local_int face_size) {
  using E = Elem<T>;
  const auto* edges = E::get_bars_vert_conn();
  ASSERT_EQ(N, 2u*E::get_edge_n());
  ASSERT_EQ(face_size, E::get_vert_n());
  ASSERT_NE(face, nullptr);
  for (std::size_t i=0; i<N; ++i) EXPECT_EQ(edges[i], expected_edges[i]);
  for (fmr::Local_int v=0; v<E::get_vert_n(); ++v) {
    EXPECT_EQ(face[v], v);
    for (uint d=0; d<3; ++d) {
      EXPECT_DOUBLE_EQ(T::vert_coor[3*v+d], T::coor_vert[d*E::get_vert_n()+v]);
    }
  }
  for (fmr::Local_int edge=0; edge<E::get_edge_n(); ++edge) {
    EXPECT_EQ(edges[2*edge], face[edge]);
    EXPECT_EQ(edges[2*edge+1], face[(edge+1)%face_size]);
  }
  double normal[3] = {};
  ASSERT_TRUE(fmr::math::tri3_norm(normal, T::vert_coor+3*face[0],
    T::vert_coor+3*face[1], T::vert_coor+3*face[2]));
  EXPECT_NEAR(normal[0], 0, 1e-14);
  EXPECT_NEAR(normal[1], 0, 1e-14);
  EXPECT_NEAR(normal[2], 1, 1e-14);
  if (face_size == 4) {
    ASSERT_TRUE(fmr::math::tri3_norm(normal, T::vert_coor+3*face[0],
      T::vert_coor+3*face[2], T::vert_coor+3*face[3]));
    EXPECT_NEAR(normal[2], 1, 1e-14);
  }
}
TEST( GridElem, PlanarSolidTopology ){
  const fmr::Local_int tris[] = {0,1, 1,2, 2,0};
  const fmr::Local_int quad[] = {0,1, 1,2, 2,3, 3,0};
  EXPECT_EQ((Elem<Tris>::get_quad_vert_conn()), nullptr);
  EXPECT_EQ((Elem<Quad>::get_tris_vert_conn()), nullptr);
  check_planar_topology<Tris>(tris, Tris::tris_vert_conn, 3);
  check_planar_topology<Quad>(quad, Quad::quad_vert_conn, 4);
}

template <class T>
void check_cubic_coordinates(const double* vertices, uint face_nodes) {
  using E = Elem<T,3>;
  double storage[3*32+2] = {};
  const int count = E::get_node_n();
  storage[0] = 123;
  storage[3*count+1] = 456;
  ASSERT_TRUE(E::get_elem_node_coor(storage+1, vertices));
  EXPECT_DOUBLE_EQ(storage[0], 123);
  EXPECT_DOUBLE_EQ(storage[3*count+1], 456);
  for (uint v=0; v<int(E::get_vert_n()); ++v) {
    for (uint d=0; d<3; ++d) EXPECT_DOUBLE_EQ(storage[1+3*v+d], vertices[3*v+d]);
  }
  for (uint e=0; e<int(E::get_edge_n()); ++e) {
    const auto a = E::get_bars_vert_conn()[2*e];
    const auto b = E::get_bars_vert_conn()[2*e+1];
    for (uint i=0; i<2; ++i) for (uint d=0; d<3; ++d) {
      const double expected = ((2-i)*vertices[3*a+d] + (i+1)*vertices[3*b+d])/3;
      EXPECT_NEAR(storage[1+3*(E::get_vert_n()+2*e+i)+d], expected, 1e-14);
    }
  }
  ASSERT_EQ(count, int(E::get_vert_n()+2*E::get_edge_n())+face_nodes);
  for (uint face=0; face<face_nodes; ++face) for (uint d=0; d<3; ++d) {
    const auto conn = E::get_tris_vert_conn()+3*face;
    const double expected = (vertices[3*conn[0]+d] + vertices[3*conn[1]+d]
      + vertices[3*conn[2]+d])/3;
    EXPECT_NEAR(storage[1+3*(E::get_vert_n()+2*E::get_edge_n()+face)+d],
      expected, 1e-14);
  }
}

TEST( GridElem, CubicNodeCoordinateBoundsAndOrdering ){
  check_cubic_coordinates<Tris>(Tris::vert_coor, 1);
  check_cubic_coordinates<Quad>(Quad::vert_coor, 0);
  check_cubic_coordinates<Tets>(Tets::vert_coor, 4);
  check_cubic_coordinates<Cube>(Cube::vert_coor, 0);
  check_cubic_coordinates<Prmd>(Prmd::vert_coor, 0);
  check_cubic_coordinates<Wdge>(Wdge::vert_coor, 2);
  check_cubic_coordinates<Itri>(Itri::vert_coor, 2);
}
template <class T>
void check_quadratic_coordinates(const double* vertices) {
  using E = Elem<T,2>;
  double storage[3*32+2] = {};
  const auto count = E::get_node_n();
  storage[0] = 123;
  storage[3*count+1] = 456;
  ASSERT_TRUE(E::get_elem_node_coor(storage+1, vertices));
  EXPECT_DOUBLE_EQ(storage[0], 123);
  EXPECT_DOUBLE_EQ(storage[3*count+1], 456);
  for (fmr::Local_int e=0; e<E::get_edge_n(); ++e) {
    const auto a = E::get_bars_vert_conn()[2*e];
    const auto b = E::get_bars_vert_conn()[2*e+1];
    for (uint d=0; d<3; ++d) {
      EXPECT_NEAR(storage[1+3*(E::get_vert_n()+e)+d],
        (vertices[3*a+d]+vertices[3*b+d])/2, 1e-14);
    }
  }
}
TEST( GridElem, QuadraticSolidNodeCoordinates ){
  check_quadratic_coordinates<Tets>(Tets::vert_coor);
  check_quadratic_coordinates<Prmd>(Prmd::vert_coor);
  check_quadratic_coordinates<Wdge>(Wdge::vert_coor);
  check_quadratic_coordinates<Cube>(Cube::vert_coor);
}
template <class T>
void check_linear_coordinates(const double* vertices) {
  using E = Elem<T,1>;
  double storage[3*8+2] = {};
  const auto count = E::get_node_n();
  storage[0] = 123;
  storage[3*count+1] = 456;
  ASSERT_EQ(count, E::get_vert_n());
  ASSERT_TRUE(E::get_elem_node_coor(storage+1, vertices));
  EXPECT_DOUBLE_EQ(storage[0], 123);
  EXPECT_DOUBLE_EQ(storage[3*count+1], 456);
  for (fmr::Local_int i=0; i<3*count; ++i) {
    EXPECT_DOUBLE_EQ(storage[1+i], vertices[i]);
  }
}
TEST( GridElem, LinearSolidNodeCoordinates ){
  check_linear_coordinates<Tets>(Tets::vert_coor);
  check_linear_coordinates<Prmd>(Prmd::vert_coor);
  check_linear_coordinates<Wdge>(Wdge::vert_coor);
  check_linear_coordinates<Cube>(Cube::vert_coor);
}
TEST( GridElem, InterfaceBarReferenceCoordinateStride ){
  double storage[26] = {};
  storage[0] = 123;
  storage[25] = 456;
  ASSERT_EQ((Elem<Ibar,3>::get_node_n()), 8);
  ASSERT_TRUE((Elem<Ibar,3>::get_elem_node_coor(storage+1)));
  EXPECT_DOUBLE_EQ(storage[0], 123);
  EXPECT_DOUBLE_EQ(storage[25], 456);
  const double expected[24] = {
    0,0,0, 1,0,0, 0,0,0, 1,0,0,
    1.0/3,0,0, 2.0/3,0,0, 1.0/3,0,0, 2.0/3,0,0
  };
  for (int i=0; i<24; ++i) EXPECT_NEAR(storage[1+i], expected[i], 1e-14);
}
}}}//end namespace femera::grid::elem
TEST( GridElem, JacdSizeTest ){
  EXPECT_EQ( jacs_size, 7 );// 2D reference quad in 3D ambient space
  EXPECT_EQ( jac1_size,  2 );// 1D, 1 point rule, 1D space
  EXPECT_EQ( jac2_size,  5 );// 2D, 1 point rule, 2D space
  EXPECT_EQ( jac3_size, 10 );// 3D, 1 point rule
  EXPECT_EQ( jac4_size, 40 );// 3D, 4 points
  EXPECT_EQ( femera::grid::elem::Elem<femera::grid::elem::Bars>::jacs_size(1, 1), 1 );
  EXPECT_EQ( femera::grid::elem::Elem<femera::grid::elem::Bars>::jacs_size(1, 2), 2 );
  EXPECT_EQ( femera::grid::elem::Elem<femera::grid::elem::Bars>::jacs_size(1, 3), 3 );
  EXPECT_EQ( femera::grid::elem::Elem<femera::grid::elem::Bars>::jacs_size(4, 3), 12 );
  EXPECT_EQ( femera::grid::elem::Elem<femera::grid::elem::Bars>::jacs_dets_size(1, 3), 4 );
  EXPECT_EQ( femera::grid::elem::Elem<femera::grid::elem::Bars>::jacs_dets_size(4, 3), 16 );
}
TEST( GridElem, TetsVertN ){
  EXPECT_EQ( elem_tet1.get_vert_n (), 4 );
}
TEST( GridElem, FaceN ){
  EXPECT_EQ( elem_tet1.get_face_n (), 4 );
}
TEST( GridElem, TetsConnN ){
  EXPECT_EQ( elem_tet1.get_node_n (), 4 );
  EXPECT_EQ( elem_tet2.get_node_n (),10 );
  EXPECT_EQ( elem_tet3.get_node_n (),20 );
  EXPECT_EQ( elem_tet2_node_n       ,10 );
}
TEST( GridElem, Measures ){
  EXPECT_FLOAT_EQ( float(elem_tet1.get_elem_v ()), float(1.0/6.0) );
  EXPECT_FLOAT_EQ( float(elem_tet1.get_elem_d ()), 3 );
}
TEST( GridElem, PolynomialShapeFunction ){
  EXPECT_EQ( elem_tet1.get_poly_func_size ( ), 4 );
  EXPECT_EQ( elem_tet2.get_poly_func_size ( ), 10 );
  EXPECT_EQ( elem_tet3.get_poly_func_size ( ), 20 );
  EXPECT_EQ( elem_tet2.get_poly_func_size (4), 40 );
}
TEST( GridElem, PolynomialShapeGradient ){
  EXPECT_EQ( elem_tet1.get_poly_grad_size ( ), 12 );
  EXPECT_EQ( elem_tet2.get_poly_grad_size ( ), 30 );
  EXPECT_EQ( elem_tet3.get_poly_grad_size ( ), 60 );
  EXPECT_EQ( elem_tet2.get_poly_grad_size (4),120 );
}
#if 0
TEST( GridElem, DoubleFloatSpecialization ){
  EXPECT_EQ( size_coor_d, 2 * size_coor_f );
}
#endif
TEST( GridElem, ThreadLocalAssignedFromConstexpr ){
  EXPECT_NE( val0_adress, val1_adress );
}
// Elem_test *****************************************************************
TEST( ElemStructTemplateArg, Works ){
  EXPECT_EQ( test_tet_3d.test_vert_n (), 4);
}
