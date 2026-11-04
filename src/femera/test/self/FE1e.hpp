#ifndef FEMERA_HAS_TEST_FE1E_HPP
#define FEMERA_HAS_TEST_FE1E_HPP

#include "../../Test.hpp"

namespace femera { namespace test {
  class FE1e;// Single-element tests
  class FE1e final: public Test<FE1e> { friend class Test;
  //TODO register this as a Data source providing node connectivity & coordinates.
  //     This class only tests geometric aspects of single elements:
  //     volume, area, length, element quality, jacs & dets, etc.
  //     It also provides reference data for single, double, and mixed precision
  //     verification, and tests data write/read.
  public: 
    std::string get_data_dir();
    std::string set_data_dir(const std::string& new_data_dir);
  private:
    bool is_data_dir_present = false;// set by set_data_dir during task_init to avoid repeated checks.
    std::string data_dir = "build/femera/test/self/FE1e/";// Default folder.
    //NOTE data_dir is changed during task_init ().
    //NOTE if folder is not present, skips write/read tests.
  private:
    void task_init (int* argc, char** argv);// scan for -T, -P args?
    // Remember to use timers.
    // 1. Queue all elements for processing.
    // 2. Load elements in mem one per thread.
    // 3. Write elements to the data_dir (if dir exists).
    // 4. Read elements into mem on same thread(s) (if dir exists).
    // 5. Compare the read elements with the originally loaded elements.
    // 6. Calculate element quality metrics (volume, area, length, jacs & dets, etc.)
    // 7. Compare calculated metrics to reference data and log results.
    // 8. Check that data is cleared from memory correctly.
    void task_exit ();
  private:
    FE1e (femera::Work::Core_ptrs_t) noexcept;
    FE1e () =delete;
  };
} }//end femera::test:: namespace

#include "FE1e.ipp"

#endif
