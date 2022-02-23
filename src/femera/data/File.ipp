#ifndef FEMERA_FILE_IPP
#define FEMERA_FILE_IPP

namespace femera {
  inline
  data::File::File (const femera::Work::Core_ptrs_t W) noexcept {
    std::tie (this->proc, this->data, this->test) = W;
    this->name ="file handler";
    this->abrv ="file";
    this->task_type = task_cast (Plug_type::File);
    this->info_d = 2;
  }
  inline
  void data::File::task_exit () {
  }
}//end femera namespace

//end FEMERA_FILE_IPP
#endif
