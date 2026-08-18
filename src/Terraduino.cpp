/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Controller
*/

#include "Terraduino.h"
#include "TerraUtils.h"

Terraduino *Terraduino::_activeInstance = nullptr;

Terraduino *getController()
{
    return Terraduino::getActiveInstance();
}

TerraLogger *getLogger()
{
    return getController() ? &getController()->logger : nullptr;
}

TerraPublisher *getPublisher()
{
    return getController() ? &getController()->publisher : nullptr;
}

TerraScheduler *getScheduler()
{
    return getController() ? &getController()->scheduler : nullptr;
}

Terraduino::Terraduino()
    : TerraFactory(), scheduler(), logger(), publisher(), modules(), _data(),
      _initialized(false), _running(false), _lastUpdateAt(0)
{
    if (!_activeInstance) { _activeInstance = this; }
}

Terraduino::~Terraduino()
{
    if (_activeInstance == this) { _activeInstance = nullptr; }
}

void Terraduino::init(const TerraSystemSetup &setup) {
    _data.setup = setup;
    _initialized = true;
    _running = false;
    _lastUpdateAt = 0;
    logger.setMinimumLevel(_data.setup.loggerMinimumLevel);
    publisher.setInterval(_data.setup.publisherIntervalMs);
    logger.info("system", "initialized");
}

void Terraduino::launch() {
    if (!_initialized) init();
    _running = true;
    logger.info("system", "launched");
}

void Terraduino::suspend() {
    _running = false;
    logger.info("system", "suspended");
}

void Terraduino::update(uint32_t now, uint16_t minuteOfDay, int16_t dayNumber) {
    if (!_initialized || !_running || _data.setup.controlMode == Terra_ControlMode_Disabled) return;
    if (_lastUpdateAt && !terraElapsed(now, _lastUpdateAt, _data.setup.updateIntervalMs)) return;
    _lastUpdateAt = now;
    updateObjects(now);
    scheduler.update(now, minuteOfDay, dayNumber);
    publisher.update(now);
}

bool Terraduino::importSystemJSON(const TerraString &json) {
    TerraSystemData parsed = _data;
    if (!parsed.fromJSON(json)) return false;
    _data = parsed;
    logger.setMinimumLevel(_data.setup.loggerMinimumLevel);
    publisher.setInterval(_data.setup.publisherIntervalMs);
    return true;
}

bool Terraduino::importSystemBinary(const uint8_t *buffer, size_t length) {
    TerraSystemData parsed = _data;
    if (!parsed.fromBinary(buffer, length)) return false;
    _data = parsed;
    logger.setMinimumLevel(_data.setup.loggerMinimumLevel);
    publisher.setInterval(_data.setup.publisherIntervalMs);
    return true;
}
