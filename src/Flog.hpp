#ifndef FMR_HAS_WORK_FLOG_HPP
#define FMR_HAS_WORK_FLOG_HPP
/** */
#include "Proc.hpp"

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#include <cstdio>     // std::printf
#endif

namespace Femera{
class Flog {
  private:
    Proc* proc=nullptr;
  public:
    const std::string task_name ="Log";
    fmr::perf::Meter       time = fmr::perf::Meter ();
    //
    FILE*    fmrout = ::stdout;// can redirect stdout
    FILE*    fmrerr = ::stderr;// can redirect stderr
    std::vector <FILE*> cpuout ={};//TODO could be ofstream ?
    //
    int   verbosity =  7;
    int      timing =  7;
    int      detail =  0;
    //
    int  line_width = 80;
    int label_width = 15;
    int   pid_width =  2;
  private:
    int   verblevel =  1;
  public:
    ~Flog ()noexcept=default;
    Flog (Proc*);
    Flog ()=delete;
    Flog (const Flog&)=delete;// not copyable
    Flog operator= (const Flog&)=delete;
    //
    int init (int*, char**);
    //
    int print_log_time (std::string suffix);
    std::string print_heading (const char* text);
    std::string print_center  (const char* text);
    //int print_center  (const char* text, int width);
    int printf (const char* format);
    std::string format_label_line (const std::string label,
      const std::string info);
    std::string print_label_line (const std::string label,
      const std::string info);
    int print_label_meter (const std::string label, std::string unit,
      fmr::perf::Meter M);
    //
    template<typename ...Args>// prints from all processes
    int fprintf (FILE* out, const char* format, Args ...args) {
      if (out) {return std::fprintf (out, format, args...);}
      return 0;
    }
    template<typename ...Args>// prints from all processes
    int label_fprintf (FILE* out, const char* label, const char* format,
      Args ...args){
      if (out) {
        if (this->detail){
          std::string f = std::string("%") +std::to_string(this->pid_width)
            + std::string("i %") +std::to_string(this->label_width)
            + std::string("s : ") + std::string(format);
          return this->fprintf (out, f.c_str(),
            this->proc->get_proc_id(), label, args...);
        }else{
          std::string f = std::string("%") +std::to_string(this->label_width)
            + std::string("s : ") + std::string(format);
          return this->fprintf (out, f.c_str(), label, args...);
        } }
      return 0;
    }
    template<typename ...Args>// only prints from master
    int printf_err (const char* format, Args ...args) {
      if (this->proc->is_master()) {if (this->fmrerr) {
      return this->fprintf (this->fmrerr, format, args...); } }
      return 0;
    }
    template<typename ...Args>// only prints from master
    int printf (const char* format, Args ...args){
      if (this->proc->is_master()) {if(this->verbosity > 0) {if (this->fmrout){
        return this->fprintf (this->fmrout, format, args...); } } }
      return 0;
    }
    template<typename ...Args>// only prints from master
    int label_printf (const char* label, const char* format, Args ...args ){
      if(this->proc->is_master()){ if(this->verbosity > 0 ){ if(this->fmrout){
        if(this->detail){
          std::string f =std::string("%") +std::to_string(this->pid_width)
            + std::string("i %") +std::to_string(this->label_width)
            + std::string("s : ") + std::string(format);
          return this->fprintf( this->fmrout, f.c_str(),
            this->proc->get_proc_id(), label, args...);
        }else{
          std::string f =std::string("%") +std::to_string(this->label_width)
            + std::string("s : ") + std::string(format);
          return this->fprintf( this->fmrout, f.c_str(), label, args...);
        }
      } } }
      return 0;
    }
    template<typename ...Args>// prints from all processes
    int fprintf (FILE* out, std::string format, Args ...args ){
      if(out) {return std::fprintf (out, format.c_str(), args...); }
      return 0;
    }
    template<typename ...Args>// prints from all processes to cpuout [proc_id]
    int proc_printf (std::string format, Args ...args) {
      const auto pid = this->proc->get_proc_id ();
      if (std::size_t(pid) < this->cpuout.size()) {
        if (cpuout [pid]) {
          return std::fprintf (cpuout [pid], format.c_str(), args...);
      } }
      return 0;
    }
    template<typename ...Args>// only prints from master
    int printf_err (const std::string format, Args ...args ){
      if(this->proc->is_master()){if(this-> fmrerr){
      return this-> fprintf( this-> fmrerr, format.c_str(), args...); } }
      return 0;
    }
    template<typename ...Args>// only prints from master
    int printf (const std::string format, Args ...args ){
      if(this->proc->is_master()){ if(this->verbosity > 0 ){ if(this-> fmrout){
        return this-> fprintf( this-> fmrout, format.c_str(), args...); } } }
      return 0;
    }
};

}//end Femera namespace

#undef FMR_DEBUG
#else
//FMR_WARN_EXTRA_INCLUDE("WARN""ING unnecessary include Flog.hpp")
// end if FMR_HAS_WORK_FLOG_HPP
#endif
