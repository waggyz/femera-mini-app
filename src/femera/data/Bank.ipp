#ifndef FEMERA_DATA_BANK_IPP
#define FEMERA_DATA_BANK_IPP

namespace femera {
  inline
  data::Bank::Bank (const Work::Core_ptrs_t core)
  noexcept : Data (core) {
    this->name      ="Femera data bank";
    this->abrv      ="bank";
    this->task_type = task_cast (Plug_type::Bank);
    this->info_d    = 3;
  }
#ifdef FMR_BANK_LOCAL
  inline
  data::Vals* data::Bank::vals_ptr () {
    return & vals;
  }
#endif
#ifdef FMR_VALS_LOCAL
  inline
  data::Vals* data::Bank::vals_ptr (fmr::Local_int ix) {
    return & vals [ix];
  }
  inline
  data::Vals* data::Bank::vals_ptr () {
    return & vals [this->proc->get_proc_ix (Plug_type::Fomp)];
  }
#endif
  inline
  void data::Bank::task_init (int*, char**) {
#ifdef FMR_VALS_LOCAL
    const auto o = this->proc->get_proc_n (Plug_type::Fomp);
    this->vals.clear ();
    this->vals.shrink_to_fit ();
    this->vals.reserve (o);// allocate on main OpenMP thread
    FMR_PRAGMA_OMP(omp parallel for schedule(static) ordered num_threads(o))
    for (fmr::Local_int i=0; i<o; i++) {// Make & add thread-local data::Bank
      FMR_PRAGMA_OMP(omp ordered) {     // in order.
        this->vals.push_back (data::Vals ());
    } }
    this->name += " with "+std::to_string (o)+" thread-local vals";
# endif
  }
  inline
  void data::Bank::task_exit () {
  }
}//end femera namespace

//end FEMERA_DATA_BANK_IPP
#endif
