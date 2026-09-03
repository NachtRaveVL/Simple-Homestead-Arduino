/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Scheduler
*/

#include "Terraduino.h"

TerraScheduler::TerraScheduler()
    : _needsScheduling(false), _inDaytimeMode(false), _lastDay{0}
{ ; }

TerraScheduler::~TerraScheduler()
{
    while (_trackings.size()) {
        auto trackingIter = _trackings.begin();
        delete trackingIter->second;
        _trackings.erase(trackingIter);
    }
}

void TerraScheduler::update()
{
    if (hasSchedulerData()) {
        #ifdef TERRA_USE_VERBOSE_OUTPUT
            Serial.println(F("Scheduler::update")); flushYield();
        #endif

        {   time_t time = unixNow();
            DateTime currTime = localTime(time);
            bool daytimeMode = _dailyTwilight.isDaytime(time);

            if (_inDaytimeMode != daytimeMode) {
                _inDaytimeMode = daytimeMode;
                setNeedsScheduling();
                Terraduino::_activeInstance->setNeedsRedraw();
            }

            if (!(_lastDay[0] == currTime.year()-2000 &&
                  _lastDay[1] == currTime.month() &&
                  _lastDay[2] == currTime.day())) {
                // only log uptime upon actual day change and if uptime has been at least 1d
                if (getLogger()->getSystemUptime() >= (time_t)SECS_PER_DAY) {
                    getLogger()->logSystemUptime();
                }
                broadcastDateChange();
            }
        }

        if (needsScheduling()) { performScheduling(); }

        for (auto trackingIter = _trackings.begin(); trackingIter != _trackings.end(); ++trackingIter) {
            trackingIter->second->update();
        }

        #ifdef TERRA_USE_VERBOSE_OUTPUT
            Serial.println(F("Scheduler::~update")); flushYield();
        #endif
    }
}

void TerraScheduler::setReportInterval(TimeSpan reportInterval)
{
    TERRA_SOFT_ASSERT(hasSchedulerData(), SFP(TStr_Err_NotYetInitialized));

    if (hasSchedulerData() && schedulerData()->reportInterval != reportInterval.totalseconds()) {
        schedulerData()->reportInterval = reportInterval.totalseconds();
        Terraduino::_activeInstance->_systemData->bumpRevisionIfNeeded();
    }
}

TimeSpan TerraScheduler::getReportInterval() const
{
    TERRA_SOFT_ASSERT(hasSchedulerData(), SFP(TStr_Err_NotYetInitialized));
    return TimeSpan(hasSchedulerData() ? schedulerData()->reportInterval : 0);
}

void TerraScheduler::updateDayTracking()
{
    time_t time = unixNow();
    DateTime currTime = localTime(time);
    _lastDay[0] = currTime.year()-2000;
    _lastDay[1] = currTime.month();
    _lastDay[2] = currTime.day();

    Location loc = getController()->getSystemLocation();
    if (loc.hasPosition()) {
        double transit;
        calcSunriseSunset((unsigned long)time, loc.latitude, loc.longitude, transit, _dailyTwilight.sunrise, _dailyTwilight.sunset,
                          loc.resolveSunAlt(), TERRA_SYS_SUNRISESET_CALCITERS);
        _dailyTwilight.isUTC = true;
    } else if (_dailyTwilight.isUTC) {
        _dailyTwilight = Twilight();
    }
    _inDaytimeMode = _dailyTwilight.isDaytime(time);

    setNeedsScheduling();
    Terraduino::_activeInstance->setNeedsRedraw();
}

void TerraScheduler::performScheduling()
{
    TERRA_HARD_ASSERT(hasSchedulerData(), SFP(TStr_Err_NotYetInitialized));

    for (auto iter = Terraduino::_activeInstance->_objects.begin(); iter != Terraduino::_activeInstance->_objects.end(); ++iter) {
        if (iter->second->isReservoirType()) {
            auto reservoir = static_pointer_cast<TerraReservoir>(iter->second);

            {   auto trackingIter = _trackings.find(reservoir->getKey());
                int trackingActuators = 0;
                auto links = reservoir->getLinkages();

                for (tposi_t linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
                    if (links.second[linksIndex].first->isActuatorType()) {
                        auto actuator = static_cast<TerraActuator *>(links.second[linksIndex].first);

                        if (actuator->getActuatorType() == Terra_ActuatorType_Heater &&
                            actuator->getParentReservoir().get() == reservoir.get()) {
                            trackingActuators++;
                        } else if (actuator->isRelayPumpClass()) {
                            auto transferActuator = static_cast<TerraRelayPumpActuator *>(actuator);
                            auto actuatorType = transferActuator->getActuatorType();

                            if ((actuatorType == Terra_ActuatorType_Pump ||
                                 actuatorType == Terra_ActuatorType_Valve ||
                                 actuatorType == Terra_ActuatorType_Circulator) &&
                                (transferActuator->getSourceReservoir().get() == reservoir.get() ||
                                 transferActuator->getDestinationReservoir().get() == reservoir.get())) {
                                trackingActuators++;
                            }
                        }
                    }
                }

                if (trackingActuators) {
                    if (trackingIter != _trackings.end()) {
                        if (trackingIter->second) {
                            trackingIter->second->setupStaging();
                        }
                    } else {
                        #ifdef TERRA_USE_VERBOSE_OUTPUT
                            Serial.print(F("Scheduler::performScheduling Tracking actuator linkages found for: ")); Serial.print(iter->second->getId().getDisplayString());
                            Serial.print(':'); Serial.print(' '); Serial.println(trackingActuators); flushYield();
                        #endif

                        TerraTracking *tracking = new TerraTracking(reservoir);
                        TERRA_SOFT_ASSERT(tracking, SFP(TStr_Err_AllocationFailure));
                        if (tracking) { _trackings[reservoir->getKey()] = tracking; }
                    }
                } else if (trackingIter != _trackings.end()) { // No tracking actuators to warrant process -> delete if exists
                    #ifdef TERRA_USE_VERBOSE_OUTPUT
                        Serial.print(F("Scheduler::performScheduling NO tracking actuator linkages found for: ")); Serial.println(iter->second->getId().getDisplayString()); flushYield();
                    #endif
                    if (trackingIter->second) { delete trackingIter->second; }
                    _trackings.erase(trackingIter);
                }
            }
        }
    }

    _needsScheduling = false;
}

void TerraScheduler::broadcastDateChange()
{
    updateDayTracking();

    #ifdef TERRA_USE_MULTITASKING
        // these can take a while to complete
        taskManager.scheduleOnce(0, []{
            if (getController()) {
                getController()->broadcastDateChanged();
            }
            yield();
            if (getLogger()) {
                getLogger()->notifyDateChanged();
            }
            yield();
            if (getPublisher()) {
                getPublisher()->notifyDateChanged();
            }
            yield();
        });
    #else
        if (getController()) {
            getController()->broadcastDateChanged();
        }
        if (getLogger()) {
            getLogger()->notifyDateChanged();
        }
        if (getPublisher()) {
            getPublisher()->notifyDateChanged();
        }
    #endif
}


TerraProcess::TerraProcess(SharedPtr<TerraReservoir> reservoirIn)
    : reservoir(reservoirIn), stageStart(unixNow())
{ ; }

void TerraProcess::clearActuatorReqs()
{
    while (actuatorReqs.size()) {
        actuatorReqs.begin()->disableActivation();
        actuatorReqs.erase(actuatorReqs.begin());
    }
}

void TerraProcess::setActuatorReqs(const Vector<TerraActuatorAttachment, TERRA_SCH_REQACTS_MAXSIZE> &actuatorReqsIn)
{
    for (auto attachIter = actuatorReqs.begin(); attachIter != actuatorReqs.end(); ++attachIter) {
        bool found = false;
        auto key = attachIter->getKey();

        for (auto attachInIter = actuatorReqsIn.begin(); attachInIter != actuatorReqsIn.end(); ++attachInIter) {
            if (key == attachInIter->getKey()) {
                found = true;
                break;
            }
        }

        if (!found) { // disables actuators not found in new list
            attachIter->disableActivation();
        }
    }

    {   actuatorReqs.clear();
        for (auto attachInIter = actuatorReqsIn.begin(); attachInIter != actuatorReqsIn.end(); ++attachInIter) {
            actuatorReqs.push_back(*attachInIter);
            actuatorReqs.back().setParent(nullptr);
        }
    }
}


TerraTracking::TerraTracking(SharedPtr<TerraReservoir> reservoir)
    : TerraProcess(reservoir), stage(Init), canProcessAfter(0), lastEnvReport(0),
      stormingReported(false), nightSeqReported(false), coverSeqReported(false)
{
    setupStaging();
}

TerraTracking::~TerraTracking()
{
    clearActuatorReqs();
}

void TerraTracking::setupStaging()
{
    #ifdef TERRA_USE_VERBOSE_OUTPUT
    {   static int8_t _stageFS1 = (int8_t)-1; if (_stageFS1 != (int8_t)stage) {
        Serial.print(F("Tracking::setupStaging stage: ")); Serial.println((_stageFS1 = (int8_t)stage)); flushYield(); } }
    #endif

    Vector<TerraActuatorAttachment, TERRA_SCH_REQACTS_MAXSIZE> newActuatorReqs;

    switch (stage) {
        case Condition: {
            if (!(reservoir->isHigh(true) || reservoir->isFilled(true))) {
                auto links = reservoir->getLinkages();

                for (tposi_t linksIndex = 0;
                     linksIndex < links.first && links.second[linksIndex].first &&
                     newActuatorReqs.size() < TERRA_SCH_REQACTS_MAXSIZE;
                     ++linksIndex) {
                    if (links.second[linksIndex].first->isActuatorType()) {
                        auto actuator = static_cast<TerraActuator *>(links.second[linksIndex].first);
                        bool useActuator = actuator->getActuatorType() == Terra_ActuatorType_Heater &&
                                           actuator->getParentReservoir().get() == reservoir.get();

                        if (!useActuator && actuator->isRelayPumpClass() &&
                            actuator->getActuatorType() == Terra_ActuatorType_Circulator) {
                            auto circulator = static_cast<TerraRelayPumpActuator *>(actuator);
                            auto sourceReservoir = circulator->getSourceReservoir();
                            auto destinationReservoir = circulator->getDestinationReservoir();

                            useActuator = sourceReservoir.get() == reservoir.get() &&
                                          (!destinationReservoir || destinationReservoir.get() == reservoir.get());
                        }

                        if (useActuator) {
                            newActuatorReqs.push_back(TerraActuatorAttachment());
                            newActuatorReqs.back().setObject(getSharedPtr<TerraActuator>(actuator));
                        }
                    }
                }
            }
        } break;

        case Distribute: {
            if (!(reservoir->isLow(true) || reservoir->isEmpty(true))) {
                auto links = reservoir->getLinkages();

                for (tposi_t linksIndex = 0;
                     linksIndex < links.first && links.second[linksIndex].first &&
                     newActuatorReqs.size() < TERRA_SCH_REQACTS_MAXSIZE;
                     ++linksIndex) {
                    if (links.second[linksIndex].first->isActuatorType()) {
                        auto actuator = static_cast<TerraActuator *>(links.second[linksIndex].first);

                        if (actuator->isRelayPumpClass()) {
                            auto transferActuator = static_cast<TerraRelayPumpActuator *>(actuator);
                            auto actuatorType = transferActuator->getActuatorType();
                            auto sourceReservoir = transferActuator->getSourceReservoir();
                            auto destinationReservoir = transferActuator->getDestinationReservoir();

                            if ((actuatorType == Terra_ActuatorType_Pump ||
                                 actuatorType == Terra_ActuatorType_Valve ||
                                 actuatorType == Terra_ActuatorType_Circulator) &&
                                sourceReservoir.get() == reservoir.get() &&
                                destinationReservoir && destinationReservoir.get() != reservoir.get() &&
                                !(destinationReservoir->isHigh(true) || destinationReservoir->isFilled(true))) {
                                bool destinationNeedsFill = destinationReservoir->isLow(true) || destinationReservoir->isEmpty(true);
                                auto destinationTrackingIter = getScheduler()->_trackings.find(destinationReservoir->getKey());

                                if (!destinationNeedsFill && destinationTrackingIter != getScheduler()->_trackings.end() &&
                                    destinationTrackingIter->second) {
                                    destinationNeedsFill = destinationTrackingIter->second->stage == Fill;
                                }

                                if (destinationNeedsFill) {
                                    newActuatorReqs.push_back(TerraActuatorAttachment());
                                    newActuatorReqs.back().setObject(getSharedPtr<TerraActuator>(actuator));
                                }
                            }
                        }
                    }
                }
            }
        } break;

        case Init:
        case Assess:
        case Fill:
        case Settle:
        default:
            break;
    }

    setActuatorReqs(newActuatorReqs);
    canProcessAfter = stage == Settle ? unixNow() + 1 : unixNow();

    #ifdef TERRA_USE_VERBOSE_OUTPUT
    {   static int8_t _stageFS2 = (int8_t)-1; if (_stageFS2 != (int8_t)stage) {
        Serial.print(F("Tracking::~setupStaging stage: ")); Serial.println((_stageFS2 = (int8_t)stage)); flushYield(); } }
    #endif
}

void TerraTracking::update()
{
    #ifdef TERRA_USE_VERBOSE_OUTPUT
    {   static int8_t _stageFU1 = (int8_t)-1; if (_stageFU1 != (int8_t)stage) {
        Serial.print(F("Tracking::update stage: ")); Serial.println((_stageFU1 = (int8_t)stage)); flushYield(); } }
    #endif

    if (!getController() || getController()->getSystemMode() != Terra_SystemMode_Automatic) {
        if (stage != Init || actuatorReqs.size()) {
            switch (stage) {
                case Assess: {
                    getLogger()->logProcess(reservoir.get(), SFP(TStr_Log_AssessmentSequence), SFP(TStr_Log_HasEnded));
                    getLogger()->logMessage(SFP(TStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan(unixNow() - stageStart)));
                } break;
                case Fill: {
                    getLogger()->logProcess(reservoir.get(), SFP(TStr_Log_FillSequence), SFP(TStr_Log_HasEnded));
                    getLogger()->logMessage(SFP(TStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan(unixNow() - stageStart)));
                } break;
                case Condition: {
                    getLogger()->logProcess(reservoir.get(), SFP(TStr_Log_ConditioningSequence), SFP(TStr_Log_HasEnded));
                    getLogger()->logMessage(SFP(TStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan(unixNow() - stageStart)));
                } break;
                case Distribute: {
                    getLogger()->logProcess(reservoir.get(), SFP(TStr_Log_DistributionSequence), SFP(TStr_Log_HasEnded));
                    getLogger()->logMessage(SFP(TStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan(unixNow() - stageStart)));
                } break;
                case Settle: {
                    getLogger()->logProcess(reservoir.get(), SFP(TStr_Log_SettlingSequence), SFP(TStr_Log_HasEnded));
                    getLogger()->logMessage(SFP(TStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan(unixNow() - stageStart)));
                } break;
                default:
                    break;
            }
            reset();
        }
        return;
    }

    time_t time = unixNow();

    if (!canProcessAfter || time >= canProcessAfter) {
        auto stageWas = stage;
        auto stageStartWas = stageStart;

        switch (stage) {
            case Init: {
                stage = Assess; stageStart = time;
                setupStaging();
            } break;

            case Assess: {
                bool needsFill = reservoir->isLow(true) || reservoir->isEmpty(true);
                bool hasInboundTransfer = false;
                bool hasConditioningActuator = false;
                bool hasOutboundDemand = false;
                auto links = reservoir->getLinkages();

                for (tposi_t linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
                    if (links.second[linksIndex].first->isActuatorType()) {
                        auto actuator = static_cast<TerraActuator *>(links.second[linksIndex].first);

                        if (actuator->getActuatorType() == Terra_ActuatorType_Heater &&
                            actuator->getParentReservoir().get() == reservoir.get()) {
                            hasConditioningActuator = true;
                        }

                        if (actuator->isRelayPumpClass()) {
                            auto transferActuator = static_cast<TerraRelayPumpActuator *>(actuator);
                            auto actuatorType = transferActuator->getActuatorType();
                            auto sourceReservoir = transferActuator->getSourceReservoir();
                            auto destinationReservoir = transferActuator->getDestinationReservoir();
                            bool isTransferType = actuatorType == Terra_ActuatorType_Pump ||
                                                  actuatorType == Terra_ActuatorType_Valve ||
                                                  actuatorType == Terra_ActuatorType_Circulator;

                            if (isTransferType && sourceReservoir && destinationReservoir &&
                                destinationReservoir.get() != sourceReservoir.get()) {
                                if (destinationReservoir.get() == reservoir.get()) {
                                    hasInboundTransfer = true;
                                }
                                if (sourceReservoir.get() == reservoir.get() &&
                                    !(destinationReservoir->isHigh(true) || destinationReservoir->isFilled(true))) {
                                    bool destinationNeedsFill = destinationReservoir->isLow(true) || destinationReservoir->isEmpty(true);
                                    auto destinationTrackingIter = getScheduler()->_trackings.find(destinationReservoir->getKey());

                                    if (!destinationNeedsFill && destinationTrackingIter != getScheduler()->_trackings.end() &&
                                        destinationTrackingIter->second) {
                                        destinationNeedsFill = destinationTrackingIter->second->stage == Fill;
                                    }

                                    hasOutboundDemand = hasOutboundDemand || destinationNeedsFill;
                                }
                            }

                            if (actuatorType == Terra_ActuatorType_Circulator &&
                                sourceReservoir.get() == reservoir.get() &&
                                (!destinationReservoir || destinationReservoir.get() == reservoir.get())) {
                                hasConditioningActuator = true;
                            }
                        }
                    }
                }

                if (needsFill) {
                    if (hasInboundTransfer) {
                        stage = Fill;
                    } else if (reservoir->isAnyThermalClass() && hasConditioningActuator) {
                        stage = Condition;
                    } else {
                        stage = Settle;
                    }
                } else if (hasOutboundDemand) {
                    stage = Distribute;
                } else {
                    stage = Settle;
                }
                stageStart = time;
                setupStaging();
            } break;

            case Fill: {
                if (reservoir->isHigh(true) || reservoir->isFilled(true)) {
                    stage = Settle; stageStart = time;
                    setupStaging();
                } else {
                    bool hasInboundTransfer = false;
                    auto links = reservoir->getLinkages();

                    for (tposi_t linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
                        if (links.second[linksIndex].first->isActuatorType()) {
                            auto actuator = static_cast<TerraActuator *>(links.second[linksIndex].first);

                            if (actuator->isRelayPumpClass()) {
                                auto transferActuator = static_cast<TerraRelayPumpActuator *>(actuator);
                                auto actuatorType = transferActuator->getActuatorType();
                                auto sourceReservoir = transferActuator->getSourceReservoir();
                                auto destinationReservoir = transferActuator->getDestinationReservoir();

                                if ((actuatorType == Terra_ActuatorType_Pump ||
                                     actuatorType == Terra_ActuatorType_Valve ||
                                     actuatorType == Terra_ActuatorType_Circulator) &&
                                    sourceReservoir && destinationReservoir.get() == reservoir.get() &&
                                    destinationReservoir.get() != sourceReservoir.get()) {
                                    hasInboundTransfer = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (!hasInboundTransfer) {
                        stage = Settle; stageStart = time;
                        setupStaging();
                    }
                }
            } break;

            case Condition: {
                if (reservoir->isHigh(true) || reservoir->isFilled(true)) {
                    stage = Settle; stageStart = time;
                    setupStaging();
                } else {
                    bool hasConditioningActuator = false;
                    auto links = reservoir->getLinkages();

                    for (tposi_t linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
                        if (links.second[linksIndex].first->isActuatorType()) {
                            auto actuator = static_cast<TerraActuator *>(links.second[linksIndex].first);

                            if (actuator->getActuatorType() == Terra_ActuatorType_Heater &&
                                actuator->getParentReservoir().get() == reservoir.get()) {
                                hasConditioningActuator = true;
                                break;
                            }

                            if (actuator->isRelayPumpClass() &&
                                actuator->getActuatorType() == Terra_ActuatorType_Circulator) {
                                auto circulator = static_cast<TerraRelayPumpActuator *>(actuator);
                                auto sourceReservoir = circulator->getSourceReservoir();
                                auto destinationReservoir = circulator->getDestinationReservoir();

                                if (sourceReservoir.get() == reservoir.get() &&
                                    (!destinationReservoir || destinationReservoir.get() == reservoir.get())) {
                                    hasConditioningActuator = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (!reservoir->isAnyThermalClass() || !hasConditioningActuator) {
                        stage = Settle; stageStart = time;
                        setupStaging();
                    }
                }
            } break;

            case Distribute: {
                if (reservoir->isLow(true) || reservoir->isEmpty(true)) {
                    stage = Settle; stageStart = time;
                    setupStaging();
                } else {
                    bool hasOutboundDemand = false;
                    auto links = reservoir->getLinkages();

                    for (tposi_t linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
                        if (links.second[linksIndex].first->isActuatorType()) {
                            auto actuator = static_cast<TerraActuator *>(links.second[linksIndex].first);

                            if (actuator->isRelayPumpClass()) {
                                auto transferActuator = static_cast<TerraRelayPumpActuator *>(actuator);
                                auto actuatorType = transferActuator->getActuatorType();
                                auto sourceReservoir = transferActuator->getSourceReservoir();
                                auto destinationReservoir = transferActuator->getDestinationReservoir();

                                if ((actuatorType == Terra_ActuatorType_Pump ||
                                     actuatorType == Terra_ActuatorType_Valve ||
                                     actuatorType == Terra_ActuatorType_Circulator) &&
                                    sourceReservoir.get() == reservoir.get() &&
                                    destinationReservoir && destinationReservoir.get() != reservoir.get() &&
                                    !(destinationReservoir->isHigh(true) || destinationReservoir->isFilled(true))) {
                                    bool destinationNeedsFill = destinationReservoir->isLow(true) || destinationReservoir->isEmpty(true);
                                    auto destinationTrackingIter = getScheduler()->_trackings.find(destinationReservoir->getKey());

                                    if (!destinationNeedsFill && destinationTrackingIter != getScheduler()->_trackings.end() &&
                                        destinationTrackingIter->second) {
                                        destinationNeedsFill = destinationTrackingIter->second->stage == Fill;
                                    }

                                    hasOutboundDemand = hasOutboundDemand || destinationNeedsFill;
                                }
                            }
                        }
                    }

                    if (!hasOutboundDemand) {
                        stage = Settle; stageStart = time;
                        setupStaging();
                    } else {
                        setupStaging();
                    }
                }
            } break;

            case Settle: {
                stage = Assess; stageStart = time;
                setupStaging();
            } break;

            default:
                break;
        }

        if (stageWas != stage) {
            switch (stageWas) {
                case Assess: {
                    getLogger()->logProcess(reservoir.get(), SFP(TStr_Log_AssessmentSequence), SFP(TStr_Log_HasEnded));
                    getLogger()->logMessage(SFP(TStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan(time - stageStartWas)));
                } break;
                case Fill: {
                    getLogger()->logProcess(reservoir.get(), SFP(TStr_Log_FillSequence), SFP(TStr_Log_HasEnded));
                    getLogger()->logMessage(SFP(TStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan(time - stageStartWas)));
                } break;
                case Condition: {
                    getLogger()->logProcess(reservoir.get(), SFP(TStr_Log_ConditioningSequence), SFP(TStr_Log_HasEnded));
                    getLogger()->logMessage(SFP(TStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan(time - stageStartWas)));
                } break;
                case Distribute: {
                    getLogger()->logProcess(reservoir.get(), SFP(TStr_Log_DistributionSequence), SFP(TStr_Log_HasEnded));
                    getLogger()->logMessage(SFP(TStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan(time - stageStartWas)));
                } break;
                case Settle: {
                    getLogger()->logProcess(reservoir.get(), SFP(TStr_Log_SettlingSequence), SFP(TStr_Log_HasEnded));
                    getLogger()->logMessage(SFP(TStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan(time - stageStartWas)));
                } break;
                default:
                    break;
            }

            switch (stage) {
                case Assess: {
                    getLogger()->logProcess(reservoir.get(), SFP(TStr_Log_AssessmentSequence), SFP(TStr_Log_HasBegan));
                    getLogger()->logMessage(SFP(TStr_Log_Field_Time_Start), localTime(stageStart).timestamp(DateTime::TIMESTAMP_TIME));
                } break;
                case Fill: {
                    getLogger()->logProcess(reservoir.get(), SFP(TStr_Log_FillSequence), SFP(TStr_Log_HasBegan));
                    getLogger()->logMessage(SFP(TStr_Log_Field_Time_Start), localTime(stageStart).timestamp(DateTime::TIMESTAMP_TIME));
                } break;
                case Condition: {
                    getLogger()->logProcess(reservoir.get(), SFP(TStr_Log_ConditioningSequence), SFP(TStr_Log_HasBegan));
                    getLogger()->logMessage(SFP(TStr_Log_Field_Time_Start), localTime(stageStart).timestamp(DateTime::TIMESTAMP_TIME));
                } break;
                case Distribute: {
                    getLogger()->logProcess(reservoir.get(), SFP(TStr_Log_DistributionSequence), SFP(TStr_Log_HasBegan));
                    getLogger()->logMessage(SFP(TStr_Log_Field_Time_Start), localTime(stageStart).timestamp(DateTime::TIMESTAMP_TIME));
                } break;
                case Settle: {
                    getLogger()->logProcess(reservoir.get(), SFP(TStr_Log_SettlingSequence), SFP(TStr_Log_HasBegan));
                    getLogger()->logMessage(SFP(TStr_Log_Field_Time_Start), localTime(stageStart).timestamp(DateTime::TIMESTAMP_TIME));
                } break;
                default:
                    break;
            }
        }
    }

    if (actuatorReqs.size()) {
        for (auto attachIter = actuatorReqs.begin(); attachIter != actuatorReqs.end(); ++attachIter) {
            attachIter->setupActivation();
            attachIter->enableActivation();
        }
    }

    #ifdef TERRA_USE_VERBOSE_OUTPUT
    {   static int8_t _stageFU2 = (int8_t)-1; if (_stageFU2 != (int8_t)stage) {
        Serial.print(F("Tracking::~update stage: ")); Serial.println((_stageFU2 = (int8_t)stage)); flushYield(); } }
    #endif
}

void TerraTracking::reset()
{
    clearActuatorReqs();
    stage = Init;
    stageStart = unixNow();
    canProcessAfter = 0;
    lastEnvReport = 0;
    stormingReported = false;
    nightSeqReported = false;
    coverSeqReported = false;
    setupStaging();
}


TerraSchedulerSubData::TerraSchedulerSubData()
    : TerraSubData(0), reportInterval(8 * SECS_PER_HOUR)
{ ; }

void TerraSchedulerSubData::toJSONObject(JsonObject &objectOut) const
{
    //TerraSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    if (reportInterval != (8 * SECS_PER_HOUR)) { objectOut[SFP(TStr_Key_ReportInterval)] = reportInterval; }
}

void TerraSchedulerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    //TerraSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    reportInterval = objectIn[SFP(TStr_Key_ReportInterval)] | reportInterval;
}
