#ifndef FEMERA_DATA_VALS_HPP
#define FEMERA_DATA_VALS_HPP

#include "../Data.hpp"

#include <unordered_map>
#include <immintrin.h>       // __m256d

namespace femera { namespace data {
  class Vals;//TODO don't derive CRTP concrete class from Data; Data has vtable
  class Vals final: public Data <Vals> { private: friend class Data;
  private:
    struct alignas (size_t) Bulk_ints {
      std::vector <fmr::Bulk_int> bulk_ints;
    };
    struct alignas (__m256d) Bulk_vals {
      std::vector <fmr::Bulk_int> bulk_vals;
    };
    proc::Team_t team_id = 0;// MPI communicator
    std::unordered_map <Vals_id, Bulk_ints> name_ints = {};
    std::unordered_map <Vals_id, Bulk_vals> name_vals = {};
  public:
#if 1
  template <typename I>
  I* add_get (Vals_id& id,//NOTE includes char type
    typename std::enable_if<std::is_integral<I>::value>::type* = 0)
  noexcept {
    if (true) {//TODO check if stored as I
      return reinterpret_cast<I*> (name_ints.at(id).bulk_ints.data());
    } else {//TODO convert to I and store back
      std::vector<I> bulk_ints(0);
      bulk_ints.reserve(name_ints.at(id).bulk_ints.capacity());
      return bulk_ints.data();
  } }//TODO move to Vals.ipp
  template <typename I>
  I* get (Vals_id& id,//NOTE includes char type
    typename std::enable_if<std::is_integral<I>::value>::type* = 0) {
    return reinterpret_cast<I*> (name_ints.at(id).bulk_ints.data());
    //TODO catch std::out_of_range
  }//TODO move to Vals.ipp
#endif
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
  private:
    Vals (femera::Work::Core_ptrs_t) noexcept;
    Vals () =delete;//NOTE Use the constructor above.
  };
} }//end femera::data:: namespace

#include "Vals.ipp"

//end FEMERA_DATA_VALS_HPP
#endif
