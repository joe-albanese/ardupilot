#include "Blimp.h"

bool AP_Arming_Blimp::pre_arm_checks(bool display_failure)
{
    const bool passed = run_pre_arm_checks(display_failure);
    set_pre_arm_check(passed);
    return passed;
}

// perform pre-arm checks
//  return true if the checks pass successfully
bool AP_Arming_Blimp::run_pre_arm_checks(bool display_failure)
{
    // exit immediately if already armed
    if (blimp.motors->armed()) {
        return true;
    }

    if (!hal.scheduler->is_system_initialized()) {
        check_failed(display_failure, "System not initialised");
        return false;
    }

    // check if motor interlock and Emergency Stop aux switches are used
    // at the same time.  This cannot be allowed.
    if (rc().find_channel_for_option(RC_Channel::AUX_FUNC::MOTOR_INTERLOCK) &&
        rc().find_channel_for_option(RC_Channel::AUX_FUNC::MOTOR_ESTOP)) {
        check_failed(display_failure, "Interlock/E-Stop Conflict");
        return false;
    }

    // if pre arm checks are disabled run only the mandatory checks
    if (checks_to_perform == 0) {
        return mandatory_checks(display_failure);
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wbitwise-instead-of-logical"
    return parameter_checks(display_failure)
#if AP_FENCE_ENABLED
           & fence_checks(display_failure)
#endif
           & motor_checks(display_failure)
           & gcs_failsafe_check(display_failure)
           & alt_checks(display_failure)
           & AP_Arming::pre_arm_checks(display_failure);
#pragma clang diagnostic pop
}

bool AP_Arming_Blimp::barometer_checks(bool display_failure)
{
    if (!AP_Arming::barometer_checks(display_failure)) {
        return false;
    }

    bool ret = true;
    // check Baro
    if (check_enabled(Check::BARO)) {
        // Check baro & inav alt are within 1m if EKF is operating in an absolute position mode.
        // Do not check if intending to operate in a ground relative height mode as EKF will output a ground relative height
        // that may differ from the baro height due to baro drift.
        const auto &ahrs = AP::ahrs();
        const bool using_baro_ref = !ahrs.has_status(AP_AHRS::Status::PRED_HORIZ_POS_REL) && ahrs.has_status(AP_AHRS::Status::PRED_HORIZ_POS_ABS);
        if (using_baro_ref) {
            if (fabsf(blimp.inertial_nav.get_position_z_up_cm() - blimp.baro_alt) > PREARM_MAX_ALT_DISPARITY_CM) {
                check_failed(Check::BARO, display_failure, "Altitude disparity");
                ret = false;
            }
        }
    }
    return ret;
}

bool AP_Arming_Blimp::ins_checks(bool display_failure)
{
    bool ret = AP_Arming::ins_checks(display_failure);

    if (check_enabled(Check::INS)) {

        // get ekf attitude (if bad, it's usually the gyro biases)
        if (!pre_arm_ekf_attitude_check()) {
            check_failed(Check::INS, display_failure, "EKF attitude is bad");
            ret = false;
        }
    }

    return ret;
}

bool AP_Arming_Blimp::board_voltage_checks(bool display_failure)
{
    if (!AP_Arming::board_voltage_checks(display_failure)) {
        return false;
    }

    // check battery voltage
    if (check_enabled(Check::VOLTAGE)) {
        if (blimp.battery.has_failsafed()) {
            check_failed(Check::VOLTAGE, display_failure, "Battery failsafe");
            return false;
        }

        // call parent battery checks
        if (!AP_Arming::battery_checks(display_failure)) {
            return false;
        }
    }

    return true;
}

bool AP_Arming_Blimp::parameter_checks(bool display_failure)
{
    // check various parameter values
    if (check_enabled(Check::PARAMETERS)) {

        // failsafe parameter checks
        if (blimp.g.failsafe_throttle) {
            // check throttle min is above throttle failsafe trigger and that the trigger is above ppm encoder's loss-of-signal value of 900
            if (blimp.channel_up->get_radio_min() <= blimp.g.failsafe_throttle_value+10 || blimp.g.failsafe_throttle_value < 910) {
                check_failed(Check::PARAMETERS, display_failure, "Check FS_THR_VALUE");
                return false;
            }
        }
    }

    return true;
}

// check motor setup was successful
bool AP_Arming_Blimp::motor_checks(bool display_failure)
{
    // check motors initialised  correctly
    if (!blimp.motors->initialised_ok()) {
        check_failed(display_failure, "Check firmware or FRAME_CLASS");
        return false;
    }

    // further checks enabled with parameters
    if (!check_enabled(Check::PARAMETERS)) {
        return true;
    }

    return true;
}

bool AP_Arming_Blimp::rc_calibration_checks(bool display_failure)
{
    return true;
}

// performs pre_arm gps related checks and returns true if passed
bool AP_Arming_Blimp::gps_checks(bool display_failure)
{
    // run mandatory gps checks first
    if (!mandatory_gps_checks(display_failure)) {
        AP_Notify::flags.pre_arm_gps_check = false;
        return false;
    }

    // check if flight mode requires GPS
    bool mode_requires_gps = blimp.flightmode->requires_GPS();


    // return true if GPS is not required
    if (!mode_requires_gps) {
        AP_Notify::flags.pre_arm_gps_check = true;
        return true;
    }

    // return true immediately if gps check is disabled
    if (!check_enabled(Check::GPS)) {
        AP_Notify::flags.pre_arm_gps_check = true;
        return true;
    }

    // warn about hdop separately - to prevent user confusion with no gps lock
    if (blimp.gps.get_hdop() > blimp.g.gps_hdop_good) {
        check_failed(Check::GPS, display_failure, "High GPS HDOP");
        AP_Notify::flags.pre_arm_gps_check = false;
        return false;
    }

    // call parent gps checks
    if (!AP_Arming::gps_checks(display_failure)) {
        AP_Notify::flags.pre_arm_gps_check = false;
        return false;
    }

    // if we got here all must be ok
    AP_Notify::flags.pre_arm_gps_check = true;
    return true;
}

// check ekf attitude is acceptable
bool AP_Arming_Blimp::pre_arm_ekf_attitude_check()
{
    return AP::ahrs().has_status(AP_AHRS::Status::ATTITUDE_VALID);
}

// performs mandatory gps checks.  returns true if passed
bool AP_Arming_Blimp::mandatory_gps_checks(bool display_failure)
{
    // always check if inertial nav has started and is ready
    const auto &ahrs = AP::ahrs();
    char failure_msg[50] = {};
    if (!ahrs.pre_arm_check(false, failure_msg, sizeof(failure_msg))) {
        check_failed(display_failure, "AHRS: %s", failure_msg);
        return false;
    }

    // check if flight mode requires GPS
    bool mode_requires_gps = blimp.flightmode->requires_GPS();

    if (mode_requires_gps) {
        if (!blimp.position_ok()) {
            // vehicle level position estimate checks
            check_failed(display_failure, "Need Position Estimate");
            return false;
        }
    } else  {
        // return true if GPS is not required
        return true;
    }

    // check for GPS glitch (as reported by EKF)
    nav_filter_status filt_status;
    if (ahrs.get_filter_status(filt_status)) {
        if (filt_status.flags.gps_glitching) {
            check_failed(display_failure, "GPS glitching");
            return false;
        }
    }

    // if we got here all must be ok
    return true;
}

// Check GCS failsafe
bool AP_Arming_Blimp::gcs_failsafe_check(bool display_failure)
{
    if (blimp.failsafe.gcs) {
        check_failed(display_failure, "GCS failsafe on");
        return false;
    }
    return true;
}

// performs altitude checks.  returns true if passed
bool AP_Arming_Blimp::alt_checks(bool display_failure)
{
    // always EKF altitude estimate
    if (!blimp.flightmode->has_manual_throttle() && !blimp.ekf_alt_ok()) {
        check_failed(display_failure, "Need Alt Estimate");
        return false;
    }

    return true;
}

// arm_checks - perform final checks before arming
//  always called just before arming.  Return true if ok to arm
//  has side-effect that logging is started
bool AP_Arming_Blimp::arm_checks(AP_Arming::Method method)
{
    return AP_Arming::arm_checks(method);
}

// mandatory checks that will be run if ARMING_CHECK is zero or arming forced
bool AP_Arming_Blimp::mandatory_checks(bool display_failure)
{
    // call mandatory gps checks and update notify status because regular gps checks will not run
    bool result = mandatory_gps_checks(display_failure);
    AP_Notify::flags.pre_arm_gps_check = result;

    // call mandatory alt check
    if (!alt_checks(display_failure)) {
        result = false;
    }

    return result & AP_Arming::mandatory_checks(display_failure);
}

void AP_Arming_Blimp::set_pre_arm_check(bool b)
{
    blimp.ap.pre_arm_check = b;
    AP_Notify::flags.pre_arm_check = b;
}

bool AP_Arming_Blimp::arm(const AP_Arming::Method method, const bool do_arming_checks)
{
    static bool in_arm_motors = false;

    // exit immediately if already in this function
    if (in_arm_motors) {
        return false;
    }
    in_arm_motors = true;

    // return true if already armed
    if (blimp.motors->armed()) {
        in_arm_motors = false;
        return true;
    }

    if (!AP_Arming::arm(method, do_arming_checks)) {
        AP_Notify::events.arming_failed = true;
        in_arm_motors = false;
        return false;
    }

#if HAL_LOGGING_ENABLED
    // let logger know that we're armed (it may open logs e.g.)
    AP::logger().set_vehicle_armed(true);
#endif

    // notify that arming will occur (we do this early to give plenty of warning)
    AP_Notify::flags.armed = true;
    // call notify update a few times to ensure the message gets out
    for (uint8_t i=0; i<=10; i++) {
        AP::notify().update();
    }

    send_arm_disarm_statustext("Arming motors"); //MIR kept in - usually only in SITL

    auto &ahrs = AP::ahrs();

    blimp.initial_armed_bearing = ahrs.yaw_sensor;

    if (!ahrs.home_is_set()) {
        // Reset EKF altitude if home hasn't been set yet (we use EKF altitude as substitute for alt above home)
        ahrs.resetHeightDatum();
        LOGGER_WRITE_EVENT(LogEvent::EKF_ALT_RESET);

        // we have reset height, so arming height is zero
        blimp.arming_altitude_m = 0;
    } else if (!ahrs.home_is_locked()) {
        // Reset home position if it has already been set before (but not locked)
        if (!blimp.set_home_to_current_location(false)) {
            // ignore failure
        }

        // remember the height when we armed
        blimp.arming_altitude_m = blimp.inertial_nav.get_position_z_up_cm() * 0.01;
    }

    hal.util->set_soft_armed(true);

    // finally actually arm the motors
    blimp.motors->armed(true);

#if HAL_LOGGING_ENABLED
    // log flight mode in case it was changed while vehicle was disarmed
    AP::logger().Write_Mode((uint8_t)blimp.control_mode, blimp.control_mode_reason);
#endif

    // perf monitor ignores delay due to arming
    AP::scheduler().perf_info.ignore_this_loop();

    // flag exiting this function
    in_arm_motors = false;

    // Log time stamp of arming event
    blimp.arm_time_ms = millis();

    // Start the arming delay
    blimp.ap.in_arming_delay = true;

    // return success
    return true;
}

// arming.disarm - disarm motors
bool AP_Arming_Blimp::disarm(const AP_Arming::Method method, bool do_disarm_checks)
{
    // return immediately if we are already disarmed
    if (!blimp.motors->armed()) {
        return true;
    }

    if (method == AP_Arming::Method::RUDDER) {
        if (!blimp.flightmode->has_manual_throttle() && !blimp.ap.land_complete) {
            return false;
        }
    }

    if (!AP_Arming::disarm(method, do_disarm_checks)) {
        return false;
    }

    send_arm_disarm_statustext("Disarming motors"); //MIR keeping in - usually only in SITL

    auto &ahrs = AP::ahrs();

    // save compass offsets learned by the EKF if enabled
    Compass &compass = AP::compass();
    if (ahrs.use_compass() && compass.get_learn_type() == Compass::LearnType::COPY_FROM_EKF) {
        for (uint8_t i=0; i<COMPASS_MAX_INSTANCES; i++) {
            Vector3f magOffsets;
            if (ahrs.getMagOffsets(i, magOffsets)) {
                compass.set_and_save_offsets(i, magOffsets);
            }
        }
    }

    // send disarm command to motors
    blimp.motors->armed(false);

#if HAL_LOGGING_ENABLED
    AP::logger().set_vehicle_armed(false);
#endif

    hal.util->set_soft_armed(false);

    blimp.ap.in_arming_delay = false;

    return true;
}
