#ifndef FMR_API_DATA_HPP
#define FMR_API_DATA_HPP
/** */
#include "flog.hpp"

namespace fmr {namespace data {
  extern int clear ();
  extern int clr ();//TODO or rem() ?
  extern int add_inp_file (const std::string file_name);
  extern int set_out_file (const std::string file_name);
  extern std::deque<std::string> get_sims ();
  extern std::deque<std::string> get_models ();//TODO Remove?
} }//end fmr::data:: namespace

#if 0
namespace fmr {namespace detail {namespace data {
  //TODO fmr::Vals or fmr::data::Vals ?
  //TODO Try to make item lookup constexpr.
  template <typename V>
  V*  get (const std::string name, const fmr::Data item);
  template <typename V>
  V*  get (const int  sim_or_part, const fmr::Data item);
  template <typename V>
  V*  get (const fmr::Sims   sims, const fmr::Data item);
  template <typename V>
  V*  get (const fmr::Part  parts, const fmr::Data item);
  //
  template <typename V>
  int set (const std::string name, const fmr::Data item, V* vals);
  template <typename V>
  int set (const int  sim_or_part, const fmr::Data item, const V* vals);
  template <typename V>
  int set (const fmr::Sims   sims, const fmr::Data item, const V* vals);
  template <typename V>
  int set (const fmr::Part   sims, const fmr::Data item, const V* vals);
  //
  template <typename V>
  int add (const std::string name, const fmr::Data item, const V* vals);
  template <typename V>
  int add (const int  sim_or_part, const fmr::Data item, const V* vals);
  template <typename V>
  int add (const fmr::Sims       , const fmr::Data item, const V* vals);
  template <typename V>
  int add (const fmr::Part       , const fmr::Data item, const V* vals);
} } }
#endif
//end FMR_API_DATA_HPP
#endif
