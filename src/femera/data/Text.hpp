#ifndef FEMERA_HAS_DATA_TEXT_HPP
#define FEMERA_HAS_DATA_TEXT_HPP

#include "../Data.hpp"

#include <unordered_map>
#include <valarray>// needed in Text.ipp

namespace femera { namespace data {
  class Text;// Derive a CRTP concrete class from Data.
  class Text final: public Data <Text> {// private: friend class Data;
  public:
    void task_init (int* argc, char** argv);
    void task_exit ();
    bool does_file (const fmr::Data_name_NEW_t&){return false;}//TODO to .?pp
  public:
    Text (femera::Work::Core_ptrs_t) noexcept;
    Text () =delete;//NOTE Use the constructor above.
  };
} }//end femera::data namespace

#include "Text.ipp"

//end FEMERA_HAS_DATA_TEXT_HPP
#endif
