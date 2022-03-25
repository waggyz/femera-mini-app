#ifndef FEMERA_DATA_FCSV_IPP
#define FEMERA_DATA_FCSV_IPP

namespace femera {
  inline
  data::Fcsv::Fcsv (const femera::Work::Core_ptrs_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name      ="Femera CSV file handler";
    this->abrv      ="csv";
    this->task_type = task_cast (Plug_type::Fcsv);
    this->info_d    = 3;
  }
  inline
  void data::Fcsv::task_init (int*, char**) {
  }
  inline
  void data::Fcsv::task_exit () {
  }
  template <typename ...Args> inline
  std::string data::Fcsv::data_line (Args... args) {
    return make_data_line (std::string(""), args...);
  }
  inline
  std::string data::Fcsv::csv_item (const std::string& str) {
    return +"\""+str+"\"";
  }
  inline
  std::string data::Fcsv::csv_item (const char* str) {
    return "\""+std::string(str)+"\"";
  }
  inline
  std::string data::Fcsv::csv_item (const float f) {
    std::valarray<char> buf (15 + 1);
    std::snprintf (&buf[0],buf.size(),"%1.*e",
      std::numeric_limits<float>::max_digits10 - 1, double(f));
    return std::string(&buf[0]);
  }
 /* Floats have 7 digits of accuracy. Doubles have 15. See:
  * www.educative.io/edpresso/what-is-the-difference-between-float-and-double
  * But,  "DBL_DECIMAL_DIG is the minimum number of significant digits to print
  * to insure the round-trip of double to string to the same exact double for
  * all double." See:
  * https://stackoverflow.com/questions/26183735
  * /print-all-significant-digits-in-sprintf-scientific-notation
  */
  inline
  std::string data::Fcsv::csv_item (const double f) {
    std::valarray<char> buf (23 + 1);
    std::snprintf (&buf[0], buf.size(),"%1.*E",
      std::numeric_limits<double>::max_digits10 - 1,  f);
    return std::string(&buf[0]);
  }
  template <typename I> inline
  std::string data::Fcsv::csv_item (const I integer,
    typename std::enable_if<std::is_integral<I>::value >::type*) {
    return std::to_string (integer);
  }
  inline
  std::string data::Fcsv::make_data_line (const std::string& line) {
    return line;
  }
  template <typename L> inline
  std::string data::Fcsv::make_data_line (const std::string& line, const L last) {
    return line + data::Fcsv::csv_item (last);
  }
  template <typename F, typename ...R> inline
  std::string data::Fcsv::make_data_line
  (const std::string& line, const F first, R... rest) {
    return make_data_line (line + data::Fcsv::csv_item (first)+",", rest...);
  }
}//end femera namespace

//end FEMERA_DATA_FCSV_IPP
#endif
