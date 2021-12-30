#include "femera.h"

int main (int argc, char** argv) {
  auto mini = femera::Main ();
  return mini.exit (mini.init (&argc, argv));
}
