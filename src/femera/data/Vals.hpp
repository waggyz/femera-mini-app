#ifndef FEMERA_DATA_VALS_HPP
#define FEMERA_DATA_VALS_HPP

#include "../Data.hpp"

#include <unordered_map>
#include <immintrin.h>       // __m256d

namespace femera { namespace data {
  //===========================================================================
  class Bulk {//TODO move to Bank.hpp; or class Bank, Bulk, Menu ?
  private:
    template <typename A>
    struct alignas (A) Bulk_vals { std::vector<fmr::Bulk_int> bulk; };
    using Ints_map_t = std::unordered_map <Vals_id, Bulk_vals<FMR_ALIGN_INTS>>;
    using Vals_map_t = std::unordered_map <Vals_id, Bulk_vals<FMR_ALIGN_VALS>>;
  private:
    Ints_map_t name_ints ={};// size_t alignment
    Vals_map_t name_vals ={};//__m256d, SSE, or AVX512 alignment
  public:
    template <typename I>
    I* add (const Vals_id& id, const size_t n=0, const I& init_int=nullptr,
      typename std::enable_if<std::is_integral<I>::value>::type* = 0)
    noexcept {
      if (n == 0) {
        this->name_ints[id].bulk ={};// inserts new or clears existing
        return reinterpret_cast<I*> (this->name_ints[id].bulk.data());
      } else {
        //TODO over-allocate & find first aligned byte?
        const auto sz = n * sizeof (I) / sizeof (fmr::Bulk_int);
        if (init_int == nullptr) {
          this->name_ints[id].bulk.reserve (sz);// Use push_back(val) to fill.
        } else {
          this->name_ints[id].bulk.resize (sz, init_int[0]);
        }
        return reinterpret_cast<I*> (this->name_ints[id].bulk.data());
      }
    }//TODO move to Bulk.ipp
    template <typename I>
    I* get (const Vals_id& id,// includes char type
      typename std::enable_if<std::is_integral<I>::value>::type* = 0) {
      return reinterpret_cast<I*> (name_ints.at(id).bulk.data());
      //TODO catch std::out_of_range
    }//TODO move to Bulk.ipp
    template <typename V>
    V* get (const Vals_id& id,
      typename std::enable_if<std::is_floating_point<V>::value>::type* = 0) {
      return reinterpret_cast<V*> (name_vals.at(id).bulk.data());
      //TODO catch std::out_of_range
    }//TODO move to Bulk.ipp
  };
  //===========================================================================
  class Vals;//            Data has vtable
  class Vals final: public Data <Vals> { private: friend class Data;
  public:
    Bulk bulk = Bulk();
    proc::Team_t team_id = 0;// MPI communicator
#if 1
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
