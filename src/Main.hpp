#ifndef FMR_HAS_MAIN_HPP
#define FMR_HAS_MAIN_HPP

namespace Femera {
class Main : public Work {// singleton: only one instance on each MPI thread
  // member variables ---------------------------------------------------------
  private:
  protected:
    // queue to run on this MPI thread or
    // list of all models (depends on sims->plan)
  public:
  // methods ------------------------------------------------------------------
  public:
    virtual ~Main ();
    Main ()=default;
    Main (Main const&)=delete;// not copyable
    Main operator= (const Main&)=delete;
    //
    virtual int add_new_task (Femera::Base_type type, Work* add_to);
    virtual int add_new_task (Femera::Plug_type type, Work* add_to);
    virtual int add_new_task (Femera::Work_type type, Work* add_to);
#if 0
    int run (const std::deque<std::string> models);
#endif
    int add_model_name (const std::string);
    int run ();
    int clear ();
  protected:
    int prep () override;
    int init_task (int* argc, char** argv) override;
    int exit_task (int err) override;
  private:
};
}//end Femera namespace
#endif
