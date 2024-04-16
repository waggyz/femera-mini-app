#ifndef FEMERA_DATA_DLIM_HPP
#define FEMERA_DATA_DLIM_HPP

#include "../Data.hpp"

#include <limits.h>     // gcc4.8 does not have FLT_DECIMAL_DIG, DBL_DECIMAL_DIG

namespace femera { namespace data {
  class Dlim;// Derive a CRTP concrete class from Data.//TODO Change to Dlim?
  class Dlim final: public Data <Dlim> {// friend class Data;
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
  private:// numeric types
    static
    std::string csv_item (float);
    static
    std::string csv_item (double);/*
    static //FIXME handle char type
    std::string csv_item (char);*/
    template <typename I> static
    std::string csv_item (I integer,//NOTE includes char type
      typename std::enable_if<std::is_integral<I>::value>::type* = 0);
  private:// character string types are double-quoted ("item")
    static
    std::string csv_item (const std::string&);
    static
    std::string csv_item (const char*);
#endif
  public:
    void task_init (int* argc, char** argv);
    void task_exit ();
    bool does_file (const fmr::Data_name_t&){return false;}//TODO to .?pp
  public:
    Dlim (femera::Work::Core_ptrs_t) noexcept;
    Dlim () =delete;//NOTE Use the constructor above.
  };
} }//end femera::data:: namespace

#include "Dlim.ipp"

//end FEMERA_DATA_DLIM_HPP
#endif