#ifndef FEMERA_DATA_LOGS_HPP
#define FEMERA_DATA_LOGS_HPP

#include "../Data.hpp"
#include "../../fmr/form.hpp"

namespace femera { namespace data {
  class Logs;// Derive a CRTP concrete class from File.
  class Logs : public Data <Logs> { private: friend class Data;
  private:
    Logs (femera::Work::Core_ptrs) noexcept;
    Logs () =delete;//NOTE Use the constructor above.
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
#if 0
    template <typename A, typename ...Args>
    ss data_line (std::tuple<A,Args...> t, ss& line="");// gcc 4.8.5 unsupported
    ss data_page (std::tuple<Args...> t);//tuple of valarray/vector refs (SoA)
#else
  public:
    template <typename ...Args> static
    std::string data_line (Args...);
    static// quoted string types
    std::string csv_string (const std::string&);
    static
    std::string csv_string (const char*);
    static// numeric types
    std::string csv_string (float);
    static
    std::string csv_string (double);
    template <typename I> static
    std::string csv_string (I integer);//NOTE includes char type
  private:
    static
    std::string data_line_p (const std::string line);
    template <typename L> static
    std::string data_line_p (const std::string line, L last);
    template <typename H, typename ...Tail> static
    std::string data_line_p (const std::string line, H head, Tail...);
#endif
  };
} }//end femera::data:: namespace

#include "Logs.ipp"

//end FEMERA_DATA_LOGS_HPP
#endif
