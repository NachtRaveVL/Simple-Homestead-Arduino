// Simple-Homestead-Arduino UI Setup Example
//
// Keeps the tcMenu include/layout pattern in place while the Terraduino-specific menu tree
// is still being designed. The shared tcMenu transport/display adapters are available, but
// the homestead menu content intentionally remains TODO work in this release.

#include <Terraduino.h>
#include <min/TerraduinoUI.h>

Terraduino terraController;

void setup()
{
    Serial.begin(115200);

    terraController.init();
    terraController.launch();

    // TODO: Configure the chosen tcMenu display/input adapters and Terraduino menu tree.
}

void loop()
{
    terraController.update();
}
