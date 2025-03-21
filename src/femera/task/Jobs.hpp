#ifndef FEMERA_HAS_JOBS_HPP
#define FEMERA_HAS_JOBS_HPP

#include "../Task.hpp"

namespace femera { namespace task {
  class Jobs;// Derive a CRTP concrete class from Task.
  class Jobs final: public Task<Jobs> { friend class Task;//Task classes intrfc
  /* This is the base Femera class. 
   * Use a Jobs instance to add Runs, Sims, and Part instances to its task list.
   * The shared Work::proc, data, test objects are managed by this class.
   * 
   * The Jobs class is a concrete implementation of the Task class, which is part of a Curiously Recurring Template Pattern (CRTP) design. This pattern is used to avoid the need for virtual functions and dynamic_casts.
   *
   * The Jobs class is the base class for managing the shared objects (proc, data,
   * and test) in a Femera application. It provides methods to initialize (task_init)
   * and exit (task_exit) the task.
   *
   * Here are the responsibilities of each method:
   *
   * task_init(int* argc, char** argv): This method is called at the start of
   * the task and is responsible for initializing the task with the provided
   * command-line arguments.
   *
   * task_exit(): This method is called at the end of the task and is
   * responsible for cleaning up and exiting the task.
   * 
   * Jobs(femera::Work::Core_ptrs_t) noexcept: This is the constructor of the
   * Jobs class. It initializes the Jobs object with the provided
   * Work::Core_ptrs_t object.
   * 
   * Jobs() noexcept: This is the default constructor of the Jobs class. It
   * initializes the Jobs object with default values.
   * 
   * The class is marked as final to prevent any further inheritance.
   */
  public:
//    Jobs* get_sims();
  private:
    void task_init (int* argc, char** argv);
    void task_exit ();
//    Sims add_sims ();
  public:
    Jobs (femera::Work::Core_ptrs_t) noexcept;
    Jobs () noexcept;
  };
} }//end femera::task:: namespace

#include "Jobs.ipp"

//end FEMERA_HAS_JOBS_HPP
#endif
