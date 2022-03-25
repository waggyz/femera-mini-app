#ifndef FEMERA_DATA_FCSV_HPP
#define FEMERA_DATA_FCSV_HPP

#include "../Data.hpp"

#include <limits.h>     // gcc4.8 does not have FLT_DECIMAL_DIG, DBL_DECIMAL_DIG

namespace femera { namespace data {
  class Fcsv;// Derive a CRTP concrete class from Data.//TODO Change to Fcsv?
  class Fcsv : public Data <Fcsv> {// private: friend class Data;
  public:
    template <typename ...Args> static
    std::string data_line (Args...);
#if 0
    template <typename A, typename ...Args>
    ss data_line (std::tuple<A,Args...> t, ss& line="");// gcc 4.8.5 unsupported
    ss data_page (std::tuple<Args...> t);//tuple of valarray/vector refs (SoA)
#else
  private:
    static
    std::string make_data_line (const std::string& line);
    template <typename Last> static
    std::string make_data_line (const std::string& line, Last);
    template <typename Head, typename ...Tail> static
    std::string make_data_line (const std::string& line, Head, Tail...);
  private:// character string types are double-quoted ("item")
    static
    std::string csv_item (const std::string&);
    static
    std::string csv_item (const char*);
  private:// numeric types
    static
    std::string csv_item (float);
    static
    std::string csv_item (double);
    template <typename I> static
    std::string csv_item (I integer,//NOTE includes char type
      typename std::enable_if<std::is_integral<I>::value>::type* = 0);
#endif
  public:
    void task_init (int* argc, char** argv);
    void task_exit ();
  public:
    Fcsv (femera::Work::Core_ptrs_t) noexcept;
    Fcsv () =delete;//NOTE Use the constructor above.
  };
} }//end femera::data:: namespace

#include "Fcsv.ipp"

//end FEMERA_DATA_FCSV_HPP
#endif
