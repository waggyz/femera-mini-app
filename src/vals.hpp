#ifndef FMR_HAS_VALS_HPP
#define FMR_HAS_VALS_HPP

#include "data-type.hpp"

#include <array>

namespace fmr {
  template <typename T> struct Vals {//TODO class?
    std::valarray<T>    data ={}                   ;
    //TODO std::valarray<Dim_int> dims = {1};// array dimensions?
    math::Zomplex    algebra = math::Real          ;
    Data                type = Data::Unknown       ;
    data::Layout      layout = data::Layout::Block ;
    data::State stored_state = data::State()       ;
    data::State memory_state = data::State()       ;
    bool isok [8*sizeof(T)];// state of heterogeneous data type items
    //TODO data.size = prod (dims) << order; // native complex is <<(order-1)
#if 0
    void clear (){
      this->data.resize(0);
           algebra = math::Real          ;
              type = Data::Unknown       ;
            layout = data::Layout::Block ;
      stored_state = data::State()       ;
      memory_state = data::State()       ;
    }
#endif
    // constructors
    Vals (){}
    Vals (const Data);
    Vals (const Data, T init_val);
    Vals (const Data, const Global_int sz, T init_val);
    Vals (const Data, const math::Zomplex);
    Vals (const Data, const math::Zomplex, const Global_int sz);
    Vals (const Data, const math::Zomplex, const Global_int sz, const T init);
  };
  typedef struct Vals<Dim_int>       Dim_int_vals;
  typedef struct Vals<Enum_int>     Enum_int_vals;
  typedef struct Vals<Local_int>   Local_int_vals;
  typedef struct Vals<Global_int> Global_int_vals;
  //
  typedef struct Vals<Geom_float> Geom_float_vals;
  typedef struct Vals<Phys_float> Phys_float_vals;
  typedef struct Vals<Solv_float> Solv_float_vals;
  typedef struct Vals<Cond_float> Cond_float_vals;
  /* e.g.
   * auto dims
   *  = fmr::Local_int_vals (fmr::Data::Geom_info, fmr::math::Natural);
   * dims.data [enum2val(fmr::Geom_info::Geom_d)]
   *  = fmr::Local_int (this->geom_d);
   */
  // Implementation ==========================================================
  template <typename T> Vals<T>::Vals (const Data t)
  : type(t) {
    const bool was_found = Data_size.count(t) > 0;
    algebra       = was_found ? Data_size.at(t).algebra : algebra;
    const auto sz = was_found ? Data_size.at(t).size : 0;
    if (sz>0) {
      const auto o = algebra.order
        -( algebra.algebra == fmr::math::Algebra::Complex
        && algebra.layout  == fmr::data::Layout::Native
        && algebra.order > 0 ) ? 1:0;
      data.resize (sz << o);//TODO handle OTI sizing?
#ifdef FMR_TOUCH_VALS_FIRST
      data[0]=T(0);
#endif
  } }
  template <typename T> Vals<T>::Vals (const Data t, T init_val)
  : type(t) {
    const bool was_found = Data_size.count(t) > 0;
    algebra       = was_found ? Data_size.at(t).algebra : algebra;
    const auto sz = was_found ? Data_size.at(t).size : 0;
    if (sz>0) {
      const auto o = algebra.order
        -( algebra.algebra == fmr::math::Algebra::Complex
        && algebra.layout  == fmr::data::Layout::Native
        && algebra.order > 0 ) ? 1:0;
      data.resize (sz << o, init_val);
  } }
  template <typename T> Vals<T>::Vals (
  const Data t, const Global_int sz, T init_val)
  : type(t) {
    const bool was_found = Data_size.count(t) > 0;
    algebra       = was_found ? Data_size.at(t).algebra : algebra;
    if (sz==0) {
      data.resize (0);
    }else{
      const auto o = algebra.order
        -( algebra.algebra == fmr::math::Algebra::Complex
        && algebra.layout  == fmr::data::Layout::Native
        && algebra.order > 0 ) ? 1:0;
      data.resize (sz << o, init_val);
  } }
  template <typename T> Vals<T>::Vals (const Data t, const math::Zomplex z)
  : algebra (z), type(t) {
    const bool was_found = Data_size.count(t) > 0;
    const auto sz = was_found ? Data_size.at(t).size : 0;
    if (sz>0) {
      const auto o = algebra.order
        -( algebra.algebra == fmr::math::Algebra::Complex
        && algebra.layout  == fmr::data::Layout::Native
        && algebra.order > 0 ) ? 1:0;
      data.resize (sz << o);
#ifdef FMR_TOUCH_VALS_FIRST
      data[0]=T(0);
#endif
  } }
  template <typename T> Vals<T>::Vals (
    const Data t, const math::Zomplex z, const Global_int sz) {
    if (sz==0) {
      data.resize(0);
    }else{
      const auto o = algebra.order
        -( algebra.algebra == fmr::math::Algebra::Complex
        && algebra.layout  == fmr::data::Layout::Native
        && algebra.order > 0 ) ? 1:0;
      data.resize (sz << o);
#ifdef FMR_TOUCH_VALS_FIRST
      data[0]=T(0);
#endif
  } }
  template <typename T> Vals<T>::Vals (
  const Data t, const math::Zomplex z, const Global_int sz, const T init_val)
  : algebra (z), type(t) {
    if (sz==0) {
      data.resize(0);
    }else{
      const auto o = algebra.order
        -( algebra.algebra == fmr::math::Algebra::Complex
        && algebra.layout  == fmr::data::Layout::Native
        && algebra.order > 0 ) ? 1:0;
      data.resize (sz << o, init_val);
  } }
}//end fmr:: namespace
//end FMR_HAS_VALS_HPP
#endif
