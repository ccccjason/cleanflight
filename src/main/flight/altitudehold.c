/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "common/maths.h"

#include "platform.h"

#include "common/maths.h"
#include "common/axis.h"

#include "drivers/accgyro.h"
#include "drivers/serial.h"
#include "drivers/gpio.h"
#include "drivers/timer.h"
#include "drivers/pwm_rx.h"

// FIXME remove dependency on currentProfile and masterConfig globals and clean up include file list.

#include "common/color.h"

#include "flight/flight.h"
#include "sensors/sensors.h"
#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/battery.h"
#include "sensors/boardalignment.h"
#include "sensors/gyro.h"
#include "sensors/sonar.h"

#include "io/escservo.h"
#include "io/gimbal.h"
#include "io/gps.h"
#include "io/serial.h"
#include "io/ledstrip.h"
#include "flight/failsafe.h"
#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/navigation.h"
#include "telemetry/telemetry.h"

#include "rx/rx.h"
#include "io/rc_controls.h"

#include "config/runtime_config.h"
#include "config/config.h"
#include "config/config_profile.h"
#include "config/config_master.h"

#if defined(BARO) || defined(SONAR)

barometerConfig_t *barometerConfig;
pidProfile_t *pidProfile;

void configureAltitudeHold(pidProfile_t *initialPidProfile, barometerConfig_t *intialBarometerConfig)
{
    pidProfile = initialPidProfile;
    barometerConfig = intialBarometerConfig;
}


static int16_t initialThrottleHold;


// 40hz update rate (20hz LPF on acc)
#define BARO_UPDATE_FREQUENCY_40HZ (1000 * 25)

#define DEGREES_80_IN_DECIDEGREES 800

static void applyMultirotorAltHold(void)
{
    static uint8_t isAltHoldChanged = 0;
    // multirotor alt hold
    if (currentProfile->alt_hold_fast_change) {
        // rapid alt changes
        if (abs(rcCommand[THROTTLE] - initialThrottleHold) > currentProfile->alt_hold_deadband) {
            errorVelocityI = 0;
            isAltHoldChanged = 1;
            rcCommand[THROTTLE] += (rcCommand[THROTTLE] > initialThrottleHold) ? -currentProfile->alt_hold_deadband : currentProfile->alt_hold_deadband;
        } else {
            if (isAltHoldChanged) {
                AltHold = EstAlt;
                isAltHoldChanged = 0;
            }
            rcCommand[THROTTLE] = constrain(initialThrottleHold + altHoldThrottleAdjustment, masterConfig.escAndServoConfig.minthrottle, masterConfig.escAndServoConfig.maxthrottle);
        }
    } else {
        // slow alt changes, mostly used for aerial photography
        if (abs(rcCommand[THROTTLE] - initialThrottleHold) > currentProfile->alt_hold_deadband) {
            // set velocity proportional to stick movement +100 throttle gives ~ +50 cm/s
            setVelocity = (rcCommand[THROTTLE] - initialThrottleHold) / 2;
            velocityControl = 1;
            isAltHoldChanged = 1;
        } else if (isAltHoldChanged) {
            AltHold = EstAlt;
            velocityControl = 0;
            isAltHoldChanged = 0;
        }
        rcCommand[THROTTLE] = constrain(initialThrottleHold + altHoldThrottleAdjustment, masterConfig.escAndServoConfig.minthrottle, masterConfig.escAndServoConfig.maxthrottle);
    }
}

static void applyFixedWingAltHold()
{
    // handle fixedwing-related althold. UNTESTED! and probably wrong
    // most likely need to check changes on pitch channel and 'reset' althold similar to
    // how throttle does it on multirotor

    rcCommand[PITCH] += altHoldThrottleAdjustment * masterConfig.fixedwing_althold_dir;
}

void applyAltHold(void)
{
    if (STATE(FIXED_WING)) {
        applyFixedWingAltHold();
    } else {
        applyMultirotorAltHold();
    }
}

void updateAltHoldState(void)
{
    // Baro alt hold activate
    if (!rcOptions[BOXBARO]) {
        DISABLE_FLIGHT_MODE(BARO_MODE);
        return;
    }

    if (!FLIGHT_MODE(BARO_MODE)) {
        ENABLE_FLIGHT_MODE(BARO_MODE);
        AltHold = EstAlt;
        initialThrottleHold = rcCommand[THROTTLE];
        errorVelocityI = 0;
        altHoldThrottleAdjustment = 0;
    }
}

void updateSonarAltHoldState(void)
{
    // Sonar alt hold activate
    if (!rcOptions[BOXSONAR]) {
        DISABLE_FLIGHT_MODE(SONAR_MODE);
        return;
    }

    if (!FLIGHT_MODE(SONAR_MODE)) {
        ENABLE_FLIGHT_MODE(SONAR_MODE);
        AltHold = EstAlt;
        initialThrottleHold = rcCommand[THROTTLE];
        errorVelocityI = 0;
        altHoldThrottleAdjustment = 0;
    }
}

bool isThrustFacingDownwards(rollAndPitchInclination_t *inclination)
{
    return abs(inclination->values.rollDeciDegrees) < DEGREES_80_IN_DECIDEGREES && abs(inclination->values.pitchDeciDegrees) < DEGREES_80_IN_DECIDEGREES;
}

int16_t calculateTiltAngle(rollAndPitchInclination_t *inclination)
{
    return max(abs(inclination->values.rollDeciDegrees), abs(inclination->values.pitchDeciDegrees));
}


int32_t calculateAltHoldThrottleAdjustment(int32_t vel_tmp, float accZ_tmp, float accZ_old)
{
    int32_t result = 0;
    int32_t error;
    int32_t setVel;

    if (!isThrustFacingDownwards(&inclination)) {
        return result;
    }

    // Altitude P-Controller

    if (!velocityControl) {
        error = constrain(AltHold - EstAlt, -500, 500);
        error = applyDeadband(error, 10); // remove small P parameter to reduce noise near zero position
        setVel = constrain((pidProfile->P8[PIDALT] * error / 128), -300, +300); // limit velocity to +/- 3 m/s
    } else {
        setVel = setVelocity;
    }
    // Velocity PID-Controller

    // P
    error = setVel - vel_tmp;
    result = constrain((pidProfile->P8[PIDVEL] * error / 32), -300, +300);

    // I
    errorVelocityI += (pidProfile->I8[PIDVEL] * error);
    errorVelocityI = constrain(errorVelocityI, -(8192 * 200), (8192 * 200));
    result += errorVelocityI / 8192;     // I in range +/-200

    // D
    result -= constrain(pidProfile->D8[PIDVEL] * (accZ_tmp + accZ_old) / 512, -150, 150);

    return result;
}

void calculateEstimatedAltitude(uint32_t currentTime)
{
    static uint32_t previousTime;
    uint32_t dTime;
    int32_t baroVel;
    float dt;
    float vel_acc;
    int32_t vel_tmp;
    float accZ_tmp;
    static float accZ_old = 0.0f;
    static float vel = 0.0f;
    static float accAlt = 0.0f;
    static int32_t lastBaroAlt;

    static int32_t baroAlt_offset = 0;
    float sonarTransition;

#ifdef SONAR
    int16_t tiltAngle;
#endif



    dTime = currentTime - previousTime;
    if (dTime < BARO_UPDATE_FREQUENCY_40HZ)
        return;

    previousTime = currentTime;

#ifdef BARO
    if (!isBaroCalibrationComplete()) {
        performBaroCalibrationCycle();
        vel = 0;
        accAlt = 0;
    }

    BaroAlt = baroCalculateAltitude();
#else
    BaroAlt = 0;
#endif

#ifdef SONAR
    tiltAngle = calculateTiltAngle(&inclination);
    sonarAlt = sonarCalculateAltitude(sonarAlt, tiltAngle);
#endif

    if (sonarAlt > 0 && sonarAlt < 200) {
        baroAlt_offset = BaroAlt - sonarAlt;
        BaroAlt = sonarAlt;
    } else {
        BaroAlt -= baroAlt_offset;
        if (sonarAlt > 0) {
            sonarTransition = (300 - sonarAlt) / 100.0f;
            BaroAlt = sonarAlt * sonarTransition + BaroAlt * (1.0f - sonarTransition);
        }
    }

    dt = accTimeSum * 1e-6f; // delta acc reading time in seconds

    // Integrator - velocity, cm/sec
    accZ_tmp = (float)accSum[2] / (float)accSumCount;
    vel_acc = accZ_tmp * accVelScale * (float)accTimeSum;

    // Integrator - Altitude in cm
    accAlt += (vel_acc * 0.5f) * dt + vel * dt;                                                                 // integrate velocity to get distance (x= a/2 * t^2)
    accAlt = accAlt * barometerConfig->baro_cf_alt + (float)BaroAlt * (1.0f - barometerConfig->baro_cf_alt);    // complementary filter for altitude estimation (baro & acc)
    vel += vel_acc;

#if 0
    debug[1] = accSum[2] / accSumCount; // acceleration
    debug[2] = vel;                     // velocity
    debug[3] = accAlt;                  // height
#endif

    accSum_reset();

#ifdef BARO
    if (!isBaroCalibrationComplete()) {
        return;
    }
#endif

    if (sonarAlt > 0 && sonarAlt < 200) {
        // the sonar has the best range
        EstAlt = BaroAlt;
    } else {
        EstAlt = accAlt;
    }

    baroVel = (BaroAlt - lastBaroAlt) * 1000000.0f / dTime;
    lastBaroAlt = BaroAlt;

    baroVel = constrain(baroVel, -1500, 1500);  // constrain baro velocity +/- 1500cm/s
    baroVel = applyDeadband(baroVel, 10);       // to reduce noise near zero

    // apply Complimentary Filter to keep the calculated velocity based on baro velocity (i.e. near real velocity).
    // By using CF it's possible to correct the drift of integrated accZ (velocity) without loosing the phase, i.e without delay
    vel = vel * barometerConfig->baro_cf_vel + baroVel * (1.0f - barometerConfig->baro_cf_vel);
    vel_tmp = lrintf(vel);

    // set vario
    vario = applyDeadband(vel_tmp, 5);

    altHoldThrottleAdjustment = calculateAltHoldThrottleAdjustment(vel_tmp, accZ_tmp, accZ_old);

    accZ_old = accZ_tmp;
}
#endif

