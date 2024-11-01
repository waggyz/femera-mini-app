#include "Elem.hpp"

namespace femera { namespace grid {

std::string elem::tri2_norm_str
 (const fmr::Geom_float* p1,
  const fmr::Geom_float* p2,
  const fmr::Geom_float* p3) {
  // Use to check correct orientation of element surfaces in 2D.
  const auto Ax = p2[0] - p1[0], Ay =  p2[1] - p1[1];
  const auto Bx = p3[0] - p2[0], By =  p3[1] - p2[1];
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
 (const fmr::Geom_float* p1,
  const fmr::Geom_float* p2,
  const fmr::Geom_float* p3) {
  // Use to check correct orientation of element surfaces in 3D.
  const auto Ax = p2[0] - p1[0], Ay =  p2[1] - p1[1], Az =  p2[2] - p1[2];
  const auto Bx = p3[0] - p2[0], By =  p3[1] - p2[1], Bz =  p3[2] - p2[2];
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
