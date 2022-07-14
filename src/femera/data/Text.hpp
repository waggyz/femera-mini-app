#ifndef FEMERA_HAS_DATA_TEXT_HPP
#define FEMERA_HAS_DATA_TEXT_HPP

#include "File.hpp"
//TODO refactor to include "../Data.hpp" instead of "File.hpp"
//#include "../Data.hpp"

#include <unordered_map>
#include <valarray>// needed in Text.ipp

namespace femera { namespace data {
  class Text;// Derive a CRTP concrete class from Data.
  class Text final: public Data <Text> {// private: friend class Data;
  private:
    using ss = std::string;
#if 1
  public:
#endif
  public:
    template <typename ...Args>
    ss text_line (const File_ptrs_t&,                 const ss& form, Args...);
    template <typename ...Args>
    ss text_line (                                    const ss& form, Args...);
    template <typename ...Args>
    ss name_line (const File_ptrs_t&, const ss& name, const ss& form, Args...);
    template <typename ...Args>
    ss name_line (                    const ss& name, const ss& form, Args...);
    template <typename ...Args>
    ss time_line (const File_ptrs_t&, const ss& name, const ss& form, Args...);
    template <typename ...Args>
    ss time_line (                    const ss& name, const ss& form, Args...);
#if 0
    ss wrap_line (File_ptrs_t, ss& name, ss& form,...);//TODO?
    ss data_line (File_ptrs_t, ss& name, ss& form,...);
    ss perf_line (File_ptrs_t, ss& name, ss& form,...);
    ss perf_line (ss name, ss& form,...);
#endif
  public:
    void task_init (int* argc, char** argv);
    void task_exit ();
  public:
    Text (femera::Work::Core_ptrs_t) noexcept;
    Text () =delete;//NOTE Use the constructor above.
  };
} }//end femera::data namespace

#include "Text.ipp"

//end FEMERA_HAS_DATA_TEXT_HPP
#endif
