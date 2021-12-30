#ifndef FEMERA_FLOG_IPP
#define FEMERA_FLOG_IPP

namespace femera {
  inline
  data::Flog::Flog (femera::Work::Make_work_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="log";
    this->info_d = 3;
  }
  inline
  data::Flog::Flog () noexcept {
    this->name ="log";
    this->info_d = 3;
  }
}//end femera namespace

//end FEMERA_FLOG_IPP
#endif
