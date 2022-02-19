#ifndef FEMERA_HAS_ERRS_IPP
#define FEMERA_HAS_ERRS_IPP

namespace femera {
  inline
  const char* Errs::what ()
  const noexcept {
    return this->msg.c_str();
  }
}//end femera:: namespace

//end FEMERA_HAS_ERRS_IPP
#endif
