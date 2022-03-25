#ifndef FEMERA_HAS_DATA_TEXT_HPP
#define FEMERA_HAS_DATA_TEXT_HPP

#include "../Data.hpp"
#include <unordered_map>

#include "../../fmr/form.hpp"// needed in Text.ipp

namespace femera { namespace data {
  class Text;// Derive a CRTP concrete class from Data.
  class Text : public Data <Text> {// private: friend class Data;
#if 0
  public:
    // TODO Make fmrlog, fmrout, fmrerr, fmrall into public functions?
    File_ptrs_t fmrlog = {};// main proc to stdout set by Logs::task_init (..)
    File_ptrs_t fmrout = {::stdout};
    File_ptrs_t fmrerr = {::stderr};
    File_ptrs_t fmrall = {::stdout};
#endif
  private:
    using ss = std::string;
#if 1
  public:
    // TODO Replace below with femera::data::Page_dims struct.
    std::unordered_map <FILE*, fmr::Line_size_int> file_line_sz
      = {{nullptr, 80}, {::stdout, 80}, {::stderr, 250}};
    std::unordered_map <FILE*, fmr::Line_size_int> line_name_sz
      = {{nullptr, 14}, {::stdout, 14}, {::stderr, 0}};
#endif
  public:
    //TODO? Move to femera::data::Text and/or femera::data::Logs
    template <typename ...Args>
    ss text_line (const File_ptrs_t&,                 const ss& form, Args...);
    template <typename ...Args>
    ss text_line (                                    const ss& form, Args...);
    template <typename ...Args>
    ss name_line (const File_ptrs_t&, const ss& name, const ss& form, Args...);
    template <typename ...Args>
    ss name_line (                    const ss& name, const ss& form, Args...);
    template <typename ...Args>
    ss name_time (const File_ptrs_t&, const ss& name, const ss& form, Args...);
    template <typename ...Args>
    ss name_time (                    const ss& name, const ss& form, Args...);
#if 0
    ss wrap_line (File_ptrs_t, ss& name, ss& form,...);//TODO
    ss time_line (File_ptrs_t, ss& name, ss& form,...);//TODO
    ss data_line (File_ptrs_t, ss& name, ss& form,...);//TODO
    ss perf_line (File_ptrs_t, ss& name, ss& form,...);//TODO
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
