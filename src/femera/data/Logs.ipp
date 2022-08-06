#ifndef FEMERA_DATA_LOGS_IPP
#define FEMERA_DATA_LOGS_IPP

namespace femera {
  inline
  fmr::Dim_int data::Logs::get_verb ()
  noexcept {
    return this->verb_d;
  }
  inline
  bool data::Logs::does_file (const fmr::Data_name_t& file)
  noexcept {
    return this->out_name_list.find (file) != this->out_name_list.end ();
  }//
  //
}//end femera namespace
//end FEMERA_DATA_LOGS_IPP
#endif
