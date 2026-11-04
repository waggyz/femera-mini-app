#include "../../core.h"
#include "FE1e.hpp"

namespace femera {
  void test::FE1e::task_init (int*, char**) {
    const auto this_cpu_name
      = this->proc->get_task (femera::Task_type::Fcpu)->get_name ();
    if (this_cpu_name.length() > 0) {
      this->set_data_dir(
        "build/" + this_cpu_name + "/femera/test/self/"+ this->abrv + "/");
    }
    this->is_data_dir_present = this->data->in_filesystem(this->get_data_dir());
    if (this->is_data_dir_present) {
      auto csv = this->data->get_task(Task_type::Dlim);
      //auto line = data::Dlim::data_line<int> (1,2,3,4);
      csv->send (this->get_data_dir() + "test-file.csv", "1,2,3,4");
    }
    this->set_init (true);
  }
  std::string test::FE1e::set_data_dir(const std::string& new_data_dir) {
    this->data_dir = new_data_dir;
    if (this->data->in_filesystem(this->data_dir)){
      this->is_data_dir_present = true;
    }
    return this->data_dir;
  }
}//end femera namespace
