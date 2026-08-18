#include <Terraduino.h>

class SerialDashboardDisplay : public TerraDisplayDriver {
public:
    void clear() override { Serial.println(); }
    void writeLine(uint8_t row, const TerraString &text, bool selected = false) override {
        (void)row;
        Serial.print(selected ? "> " : "  ");
        Serial.println(text);
    }
    void flush() override { }
};

Terraduino terra;
SerialDashboardDisplay display;
TerraduinoUI dashboard(&terra, &display, nullptr);
TerraCistern cistern(5000.0f, 1001, "Main Cistern");
TerraEnvironment outside(1002, "Outside");

void setup() {
    Serial.begin(115200);
    terra.init();
    cistern.setThresholds(15.0f, 30.0f, 95.0f);
    cistern.configureFillBand(35.0f, 90.0f, 99.0f);
    cistern.setStoredLiters(3200.0f);
    outside.setAirTemperature(4.0f);
    terra.registerObject(&cistern);
    terra.registerObject(&outside);
    dashboard.begin();
}

void loop() {
    terra.update();
    dashboard.update();
}
