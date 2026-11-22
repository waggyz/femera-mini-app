#ifndef FEMERA_HAS_BEDS_IPP
#define FEMERA_HAS_BEDS_IPP

namespace femera {
  inline constexpr
  test::Beds::Rand_int test::Beds::get_rand_max () noexcept {
    return std::mt19937::max ();
  }
  inline constexpr
  unsigned int test::Beds::get_rand_seed
    (const fmr::Local_int proc_id, const unsigned int base_seed) noexcept {
    constexpr auto mod = std::uintmax_t
      (std::numeric_limits<unsigned int>::max ());
    const auto a = std::uintmax_t (base_seed) % mod;
    const auto b = std::uintmax_t (proc_id) % mod;
    // Reduce before adding, including when Local_int is as wide as uintmax_t.
    return static_cast<unsigned int>
      (a >= mod - b ? a - (mod - b) : a + b);
  }
  inline
  test::Beds::Beds (const femera::Work::Core_ptrs_t W)
  noexcept : Test (W) {
    this->name      ="Femera testbeds";
    this->abrv      ="beds";
    this->task_type = task_cast (Task_type::Beds);
  }
  inline
  void test::Beds::task_exit () {
    this->rand_ready = false;
    this->rand_states.clear ();
  }
}//end femera namespace

//end FEMERA_HAS_BEDS_IPP
#endif
