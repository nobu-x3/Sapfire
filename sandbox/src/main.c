#include <core/application.h>

int main(void) {
  application_config cfg;
  cfg.width = 800;
  cfg.height = 600;
  cfg.name = "Sapfire Sandbox";
  application_state* app = application_create(&cfg);
  application_run(app);
  return 0;
}
