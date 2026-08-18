#ifndef TerraMenuScreens_H
#define TerraMenuScreens_H
class Terraduino;
class TerraMenuModel;
void terraBuildHomeScreen(const Terraduino &, TerraMenuModel &);
void terraBuildWaterScreen(const Terraduino &, TerraMenuModel &);
void terraBuildThermalScreen(const Terraduino &, TerraMenuModel &);
void terraBuildEnvironmentScreen(const Terraduino &, TerraMenuModel &);
void terraBuildSensorsScreen(const Terraduino &, TerraMenuModel &);
void terraBuildActuatorsScreen(const Terraduino &, TerraMenuModel &);
void terraBuildPowerRailsScreen(const Terraduino &, TerraMenuModel &);
void terraBuildSchedulingScreen(const Terraduino &, TerraMenuModel &);
void terraBuildAlertsScreen(const Terraduino &, TerraMenuModel &);
void terraBuildSettingsScreen(const Terraduino &, TerraMenuModel &);
void terraBuildInformationScreen(const Terraduino &, TerraMenuModel &);
#endif
