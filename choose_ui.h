
#ifndef GENERATOR_CHOOSE_UI_H
#define GENERATOR_CHOOSE_UI_H
#include <stdbool.h>
#define GUI false
#if GUI
#include "gui.h"
#else
#include "cli.h"
#endif
#endif //GENERATOR_CHOOSE_UI_H
