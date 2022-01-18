#ifndef FEMERA_FILE_IPP
#define FEMERA_FILE_IPP

namespace femera {
  inline
  data::File::File (femera::Work::Core_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="file handler";
    this->info_d = 2;
  }
  inline
  void data::File::task_exit () {
//    this->log =nullptr;
  }
}//end femera namespace

//end FEMERA_FILE_IPP
#endif
