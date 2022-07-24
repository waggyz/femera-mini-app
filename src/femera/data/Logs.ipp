#ifndef FEMERA_DATA_LOGS_IPP
#define FEMERA_DATA_LOGS_IPP

namespace femera {
  inline
  fmr::Dim_int data::Logs::set_verb (const fmr::Dim_int v)
  noexcept {
    if (v > FMR_VERBMAX) {//TODO print warning
      this->verb_d = FMR_VERBMAX;
    } else {
      this->verb_d = v;
    }
    return this->verb_d;
  }
  inline
  bool data::Logs::do_log (const fmr::Dim_int v)
  noexcept {
    return (v <= this->verb_d) && this->did_init ();
  }
  inline
  bool data::Logs::did_init ()
  noexcept {
    return this->did_init_tf;
  }
  inline
  bool data::Logs::set_init (const bool tf)
  noexcept {
    this->did_init_tf = tf;
    return this->did_init_tf;
  }
  inline
  bool data::Logs::does_file (const fmr::Data_name_t& file)
  noexcept {
    return this->out_name_list.find(file) != this->out_name_list.end();
  }
}//end femera namespace

//end FEMERA_DATA_LOGS_IPP
#endif
