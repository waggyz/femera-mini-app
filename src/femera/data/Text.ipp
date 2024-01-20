#ifndef FEMERA_DATA_TEXT_IPP
#define FEMERA_DATA_TEXT_IPP

namespace femera {
  inline
  data::Text::Text (const Work::Core_ptrs_t W)
  noexcept : Data (W) {
    this->name      ="Femera text handler";
    this->abrv      ="text";
    this->task_type = task_cast (Task_type::Text);
  }
  inline
  void data::Text::task_init (int*, char**) {
    this->set_init (true);
  }
  inline
  void data::Text::task_exit () {
  }
  //
}//end femera namespace

//end FEMERA_DATA_TEXT_IPP
#endif
