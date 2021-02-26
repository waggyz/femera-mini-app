#include "core.h"

#include <cstdio>     // std::printf
#include <cstring>    // strlen()

#undef FMR_DEBUG
#ifdef FMR_DEBUG
#endif

namespace Femera{
  Flog::Flog (Proc* P):proc(P){
  }
int Flog::init (int*, char**) {
  const int n = this->proc->get_proc_n();
  this->pid_width = (n > 9) ? fmr::math::count_digits (n-1) : 1;
  return this->pid_width < 1 ? 1 : 0 ;
}
int Flog::print_log_time (std::string suffix) {
  if (this->timing >= this->verblevel){
    if (this->verbosity >= this->verblevel){
      fmr::perf::timer_stop (& this->time);
      std::string lab = this->task_name+" "+suffix;
      this->print_label_meter (lab, "", this->time);
  } }
  return 0;
}
int Flog::printf (const char* format){
  fmr::perf::timer_resume (& this->time);
  if (this->proc->is_master()){if (this->verbosity > 0){
    return std::fprintf (this-> fmrout, format);
  } }
  fmr::perf::timer_pause (& this->time);
  return 0;
}
std::string Flog::print_label_line (std::string label, std::string info){
  fmr::perf::timer_resume (& this->time);
  std::string line = this->format_label_line (label, info);
  this->printf (line.c_str());
  fmr::perf::timer_pause (& this->time);
  return line;
}
std::string Flog::format_label_line (std::string label, std::string info){
  fmr::perf::timer_resume (& this->time);
  if (this->label_width < 3){
    fmr::perf::timer_pause (& this->time);
    return info;
  }
  if (this->label_width > 240){
    this->label_width = 240;
    this->printf_err ("WARN""ING Set line label width to 240.\n");
    label = label.substr (0, this->label_width);
  }
  std::vector<char> buf(255 +1,0); int c=0;
  if(this-> detail){
    c = std::snprintf (& buf[0], 255, "%*i %*s : ",
      this->pid_width, this->proc->get_proc_id(),
      this->label_width, label.c_str() );
  }else{
    c = std::snprintf (& buf[0], 255, "%*s : ",this->label_width, label.c_str());
  }
  //TODO Wrap and indent long lines?
  fmr::perf::timer_pause (& this->time);
  if (c>0){
    return std::string (& buf[0]) + info + "\n";
  }
  return info + "\n";
}
std::string Flog::print_center (const char *text){//, int width ){
  fmr::perf::timer_resume (& this->time);
  // https://stackoverflow.com/questions/2461667/centering-strings-with-printf
  if(this->proc->is_master()){
    std::vector<char> buf(255 +1,0);
    int c=0;
    if(this-> detail){
      const int pad =(this->line_width - int(strlen(text)))/2 - this->pid_width;
      c = std::snprintf (& buf[0], 255, "%*i%*s%s%*s\n",
        this->pid_width, this->proc->get_proc_id (), pad, "", text, pad, "");
    }else{
      const int pad = (this->line_width-int(strlen (text)))/2;
      c = std::snprintf (& buf[0], 255, "%*s%s%*s\n", pad, "", text, pad, "");
    }
    if (c>0){
      std::printf (& buf[0]);
      fmr::perf::timer_pause (& this->time);
      return std::string(& buf[0]);
  } }
  fmr::perf::timer_pause (& this->time);
  return std::string("");
}
std::string Flog::print_heading(const char *head ){
  fmr::perf::timer_resume (& this->time);
  if (this->proc->is_master()) {if (this->verbosity >= this->verblevel) {
    std::string text="", timestr="";
    auto now_time = std::chrono::system_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      now_time.time_since_epoch()).count();
    const auto as_time_t = std::chrono::system_clock::to_time_t (now_time);
    struct tm tmbuf;
    if (::gmtime_r (& as_time_t, & tmbuf)){
      char buf[32];
      if (std::strftime (& buf[0], sizeof(buf), "UTC %Y-%m-%d %H:%M:%S",
        & tmbuf)) {
        timestr = std::string{buf};
    } }
    //throw std::runtime_error("Failed to get current date as string");
    std::vector<char> buf(255 +1,0);
    int c=0;
    if(this-> detail){
      c = std::snprintf (& buf[0], 255, "%*i%*s.%03i\n",
        this->pid_width, this->proc->get_proc_id (),
        this->line_width-this->pid_width-4, timestr.c_str(), int(ms % 1000));
    }else{
      c = std::snprintf (& buf[0], 255, "%*s.%03i\n",
        this->line_width-4, timestr.c_str(), int(ms % 1000));
    }
    if (c>0){
      std::printf (& buf[0]);
      text += std::string (& buf[0]);
    }
    text += Flog::print_center (head);
    fmr::perf::timer_pause (& this->time);
    return text;
  } }
  fmr::perf::timer_pause (& this->time);
  return std::string("");
}
int Flog::print_label_meter (const std::string label, std::string units,
      fmr::perf::Meter M){
  fmr::perf::timer_resume (& this->time);
  //TODO move Flog::print_label_meter(..) to fmr:: ?
  // Get times.
  std::string total = fmr::perf::format_time_units (
    fmr::perf::timer_total_elapsed (M));
  std::string busy  = fmr::perf::format_time_units (
    fmr::perf::timer_busy_elapsed (M));
  std::string idle  = fmr::perf::format_time_units (
    fmr::perf::timer_idle_elapsed (M));
  this->label_printf (label.c_str(),
    "%s busy, %s idle, %s total\n",
    busy.c_str(), idle.c_str(), total.c_str());
  if(M.count != 0 || M.flops != 0 || M.bytes != 0){
    auto plabel = label;
    if (plabel.length() > 3){
      plabel.replace (plabel.length()-4,4,"perf");//TODO Fix this.
    }
    if (units.length() > 5){ units = units.substr(0,5); }
    std::string speed = units+"/s";
    if (speed.length() > 7){ speed = speed.substr(0,7); }
    if (units.length() < 7){ units.append (7-units.length(),' '); }
    if (speed.length() < 7){ speed.append (7-speed.length(),' '); }
    // Get counts.
    std::string count = fmr::perf::format_count (M.count, units);
    std::string flops = fmr::perf::format_count (M.flops,"flop  ");
    std::string bytes = fmr::perf::format_count (M.bytes,"B     ");
    if (M.flops > 0){
      this->label_printf (plabel.c_str(),
        "%s  %s  %s  %7.3f flop/B\n",
        flops.c_str(), bytes.c_str(), count.c_str(),
        double (M.flops) / double (M.bytes));
    }else{
      this->label_printf (plabel.c_str(),
        "%s  %s  %s\n", flops.c_str(), bytes.c_str(), count.c_str());
    }
    // active speeds
    flops = fmr::perf::format_units (
      fmr::perf::Float (M.flops) / fmr::perf::Float (M.busy_ns)
        * fmr::perf::Float (1e9),"flop/s");
    bytes = fmr::perf::format_units (
      fmr::perf::Float (M.bytes) / fmr::perf::Float (M.busy_ns)
        * fmr::perf::Float (1e9),"B/s   ");
    count = fmr::perf::format_units (
      fmr::perf::Float (M.count) / fmr::perf::Float (M.busy_ns)
        * fmr::perf::Float (1e9), speed);
    this->label_printf (plabel.c_str(),
      "%s  %s  %s          Active\n",
      flops.c_str(), bytes.c_str(), count.c_str());
    // overall speeds
    fmr::perf::Float overall = fmr::perf::Float (1e9)
      / fmr::perf::Float (M.click - M.start);
    flops = fmr::perf::format_units (
      fmr::perf::Float (M.flops) * overall,"flop/s");
    bytes = fmr::perf::format_units (
      fmr::perf::Float (M.bytes) * overall,"B/s   ");
    count = fmr::perf::format_units (
      fmr::perf::Float (M.count) * overall, speed);
    this->label_printf (plabel.c_str(),
      "%s  %s  %s          Overall\n",
      flops.c_str(), bytes.c_str(), count.c_str());
  }
  fmr::perf::timer_pause (& this->time);
  return 0;
}
}// end Femera namespace
#if 0
        //TODO move this into print_label_meter to format 0 < |speed| < 1.
        double solv_per_sec = fmr::perf::overall_speed ( this->time );
        double sec_per_solv = 1.0 / solv_per_sec;
        double t=sec_per_solv;
        std::string f="";
        if     ( sec_per_solv <   1.0){f="%.0f ms/sim (%.1f sim/sc)\n"; t*=1e3; }
        else if( sec_per_solv < 120.0){f="%.1f sc/sim\n"; }
        else if( sec_per_solv <7200.0){f="%.1f mn/sim\n"; t/=   60.0; }
        else                          {f="%.1f hr/sim\n"; t/= 3600.0;
        }
        if( sec_per_solv < 1 ){
           label_printf ("Overall perf", f.c_str(), t, solv_per_sec );
        }else{
           label_printf ("Overall perf", f.c_str(), t );
      }
#endif
#undef FMR_DEBUG
