#ifndef FEMERA_FLOG_HPP
#define FEMERA_FLOG_HPP

#include "../Data.hpp"
#include "../../fmr/form.hpp"

namespace femera { namespace data {
  class Logs;// Derive a CRTP concrete class from File.
  class Logs : public Data <Logs> { private: friend class Data;
  private:
//FIXME remove?    fmr::Line_size_int line_sz = 80;
//FIXME remove?    fmr::Line_size_int head_sz = 14;
  private:
    Logs (femera::Work::Core_ptrs) noexcept;
    Logs () =delete;//NOTE Use the constructor above.
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
#if 0
    template <typename A, typename ...Args>  
    ss data_line (std::tuple<A,Args...> t, ss& line="");
#else
  public:
    template <typename ...Args> static
    std::string data_line (Args...);
    static
    std::string to_string (float);
    static
    std::string to_string (double);
    template <typename I> static
    std::string to_string (I integer);
  private:
    static
    std::string data_line_p (std::string line);
    // quoted string types
    static
    std::string data_line_p (std::string line, std::string last);
    template <typename ...Tail> static
    std::string data_line_p (std::string line, std::string head, Tail...);
    static
    std::string data_line_p (std::string line, const char* last);
    template <typename ...Tail> static
    std::string data_line_p (std::string line, const char* head, Tail...);
    // numeric types
    template <typename L> static
    std::string data_line_p (std::string line, L last);
    template <typename H, typename ...Tail> static
    std::string data_line_p (std::string line, H head, Tail...);
#endif
//TODO    ss data_page (std::tuple<Args...> t);//tuple of valarray/vector refs (SOA)
  };
} }//end femera::file:: namespace

#include "Logs.ipp"

//end FEMERA_FLOG_HPP
#endif
