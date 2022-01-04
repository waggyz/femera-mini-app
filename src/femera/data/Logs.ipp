#ifndef FEMERA_FLOG_IPP
#define FEMERA_FLOG_IPP

namespace femera {
  inline
  data::Logs::Logs (femera::Work::Core_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="log";
    this->info_d = 3;
  }
}//end femera namespace

//end FEMERA_FLOG_IPP
#endif
