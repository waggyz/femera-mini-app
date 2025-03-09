#include "Elem.hpp"

#include <cmath>// std::sqrt () needed to calculate edge lengths

namespace femera { namespace grid {

std::string elem::tri2_norm_str
 (const fmr::Geom_float* pt1,
  const fmr::Geom_float* pt2,
  const fmr::Geom_float* pt3) {
  // Use to check correct orientation of element surfaces in 2D.
  const auto Ax = pt2[0] - pt1[0], Ay =  pt2[1] - pt1[1];
  const auto Bx = pt3[0] - pt2[0], By =  pt3[1] - pt2[1];
  auto Nz = float(Ax * By - Ay * Bx);
  const auto len = std::sqrt (Nz*Nz);
  Nz /= len;
  if (std::abs (Nz - float(1.0)) < float(1.0e-6)) {return std::string ("+z");}
  if (std::abs (Nz + float(1.0)) < float(1.0e-6)) {return std::string ("-z");}
  return std::string ("["
    + std::to_string (0.0) + ", "
    + std::to_string (0.0) + ", "
    + std::to_string (Nz) + "]");
}
std::string elem::tri3_norm_str
 (const fmr::Geom_float* pt1,
  const fmr::Geom_float* pt2,
  const fmr::Geom_float* pt3) {
  // Use to check correct orientation of element surfaces in 3D.
  const auto Ax = pt2[0] - pt1[0], Ay =  pt2[1] - pt1[1], Az =  pt2[2] - pt1[2];
  const auto Bx = pt3[0] - pt2[0], By =  pt3[1] - pt2[1], Bz =  pt3[2] - pt2[2];
  auto Nx = float(Ay * Bz - Az * By);
  auto Ny = float(Az * Bx - Ax * Bz);
  auto Nz = float(Ax * By - Ay * Bx);
  const auto len = std::sqrt (Nx*Nx + Ny*Ny + Nz*Nz);
  Nx /= len; Ny /= len; Nz /= len;
  if (std::abs (Nx - float(1.0)) < float(1.0e-6)) {return std::string ("+x");}
  if (std::abs (Nx + float(1.0)) < float(1.0e-6)) {return std::string ("-x");}
  if (std::abs (Ny - float(1.0)) < float(1.0e-6)) {return std::string ("+y");}
  if (std::abs (Ny + float(1.0)) < float(1.0e-6)) {return std::string ("-y");}
  if (std::abs (Nz - float(1.0)) < float(1.0e-6)) {return std::string ("+z");}
  if (std::abs (Nz + float(1.0)) < float(1.0e-6)) {return std::string ("-z");}
#if 0
  if (std::abs (Nx) < float(1.0e-6)) {return std::string ("xy");}
  if (std::abs (Ny) < float(1.0e-6)) {return std::string ("xz");}
  if (std::abs (Nz) < float(1.0e-6)) {return std::string ("yz");}
  return std::string ("xyz");
#else
  return std::string ("["
    + std::to_string (Nx) + ", "
    + std::to_string (Ny) + ", "
    + std::to_string (Nz) + "]");
  #endif
}

} }//end femera::grid:: namespace
