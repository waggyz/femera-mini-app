#ifndef FEMERA_GTST_IPP
#define FEMERA_GTST_IPP

namespace femera {
  inline
  test::Gtst::Gtst (const femera::Work::Core_ptrs_t W)
  noexcept : Test (W) {
    this->name      ="GoogleTest";
    this->abrv      ="gtst";
    this->version   = MAKESTR(FMR_GTEST_VERSION) ;
    this->task_type = task_cast (Task_type::Gtst);
    this->info_d    = 3;
  }
}//end femera namespace

//end FEMERA_GTST_IPP
#endif
