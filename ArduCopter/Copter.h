/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
/*
  This is the main Copter class
 */

////////////////////////////////////////////////////////////////////////////////
// Header includes
////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <stdio.h>
#include <stdarg.h>

#include <AP_HAL/AP_HAL.h>

// Common dependencies
#include <AP_Common/AP_Common.h>            // Common definitions and utility routines for the ArduPilot libraries
#include <AP_Common/Location.h>             // Library having the implementation of location class         
#include <AP_Param/AP_Param.h>              // A system for managing and storing variables that are of general interest to the system.
#include <StorageManager/StorageManager.h>  // library for Management for hal.storage to allow for backwards compatible mapping of storage offsets to available storage

// Application dependencies
#include <AP_Logger/AP_Logger.h>            // ArduPilot Mega Flash Memory Library
#include <AP_Math/AP_Math.h>                // ArduPilot Mega Vector/Matrix math Library
#include <AP_AccelCal/AP_AccelCal.h>        // interface and maths for accelerometer calibration
#include <AP_InertialSensor/AP_InertialSensor.h>                // ArduPilot Mega Inertial Sensor (accel & gyro) Library
#include <AP_AHRS/AP_AHRS.h>                                    // AHRS (Attitude Heading Reference System) interface library for ArduPilot
#include <AP_Mission/AP_Mission.h>                              // Mission command library
#include <AP_Mission/AP_Mission_ChangeDetector.h>               // Mission command change detection library
#include <AC_AttitudeControl/AC_AttitudeControl_Multi.h>        // Attitude control library
#include <AC_AttitudeControl/AC_AttitudeControl_Multi_6DoF.h>   // 6DoF Attitude control library
#include <AC_AttitudeControl/AC_AttitudeControl_Heli.h>         // Attitude control library for traditional helicopter
#include <AC_AttitudeControl/AC_PosControl.h>                   // Position control library
#include <AC_AttitudeControl/AC_CommandModel.h>                 // Command model library
#include <AP_Motors/AP_Motors.h>            // AP Motors library
#include <Filter/Filter.h>                  // Filter library
#include <AP_Vehicle/AP_Vehicle.h>          // needed for AHRS build
#include <AC_WPNav/AC_WPNav.h>              // ArduCopter waypoint navigation library
#include <AC_WPNav/AC_Loiter.h>             // ArduCopter Loiter Mode Library
#include <AC_WPNav/AC_Circle.h>             // circle navigation library
#include <AP_Declination/AP_Declination.h>  // ArduPilot Mega Declination Helper Library
#include <AP_RCMapper/AP_RCMapper.h>        // RC input mapping library
#include <AP_BattMonitor/AP_BattMonitor.h>  // Battery monitor library
#include <AP_LandingGear/AP_LandingGear.h>  // Landing Gear library
#include <AC_InputManager/AC_InputManager.h>        // Pilot input handling library
#include <AC_InputManager/AC_InputManager_Heli.h>   // Heli specific pilot input handling library
#include <AP_Arming/AP_Arming.h>            // ArduPilot motor arming library
#include <AP_SmartRTL/AP_SmartRTL.h>        // ArduPilot Smart Return To Launch Mode (SRTL) library
#include <AP_TempCalibration/AP_TempCalibration.h>  // temperature calibration library
#include <AC_AutoTune/AC_AutoTune_Multi.h>  // ArduCopter autotune library. support for autotune of multirotors.
#include <AC_AutoTune/AC_AutoTune_Heli.h>   // ArduCopter autotune library. support for autotune of helicopters.
#include <AP_Parachute/AP_Parachute.h>      // ArduPilot parachute release library
#include <AC_Sprayer/AC_Sprayer.h>          // Crop sprayer library
#include <AP_Avoidance/AP_Avoidance.h>      // "ADSB" avoidance library
#include <AP_ADSB/AP_ADSB.h>                // ADS-B RF based collision avoidance module library
#include <AP_Proximity/AP_Proximity.h>      // ArduPilot proximity sensor library
#include <AC_PrecLand/AC_PrecLand_config.h>
#include <AP_OpticalFlow/AP_OpticalFlow.h>
#include <AP_Winch/AP_Winch_config.h>
#include <AP_SurfaceDistance/AP_SurfaceDistance.h>

// Configuration
#include "defines.h"
#include "config.h"

#if FRAME_CONFIG == HELI_FRAME
 #define MOTOR_CLASS AP_MotorsHeli
#else
 #define MOTOR_CLASS AP_MotorsMulticopter
#endif

#if MODE_AUTOROTATE_ENABLED
 #include <AC_Autorotation/AC_Autorotation.h> // Autorotation controllers
#endif

#include "RC_Channel_Copter.h"         // RC Channel Library

#include "GCS_MAVLink_Copter.h"
#include "GCS_Copter.h"
#include "AP_Rally.h"           // Rally point library
#include "AP_Arming_Copter.h"

#include <AP_ExternalControl/AP_ExternalControl_config.h>
#if AP_EXTERNAL_CONTROL_ENABLED
#include "AP_ExternalControl_Copter.h"
#endif

#include <AP_Beacon/AP_Beacon_config.h>
#if AP_BEACON_ENABLED
 #include <AP_Beacon/AP_Beacon.h>
#endif

#if AP_AVOIDANCE_ENABLED
 #include <AC_Avoidance/AC_Avoid.h>
#endif
#if AP_OAPATHPLANNER_ENABLED
 #include <AC_WPNav/AC_WPNav_OA.h>
 #include <AC_Avoidance/AP_OAPathPlanner.h>
#endif
#if AC_PRECLAND_ENABLED
 # include <AC_PrecLand/AC_PrecLand.h>
 # include <AC_PrecLand/AC_PrecLand_StateMachine.h>
#endif
#if MODE_FOLLOW_ENABLED
 # include <AP_Follow/AP_Follow.h>
#endif
#if AP_TERRAIN_AVAILABLE
 # include <AP_Terrain/AP_Terrain.h>
#endif
#if AP_RANGEFINDER_ENABLED
 # include <AP_RangeFinder/AP_RangeFinder.h>
#endif

#include <AP_Mount/AP_Mount.h>

#include <AP_Camera/AP_Camera.h>

#if HAL_BUTTON_ENABLED
 # include <AP_Button/AP_Button.h>
#endif

#if OSD_ENABLED || OSD_PARAM_ENABLED
 #include <AP_OSD/AP_OSD.h>
#endif

#if AP_COPTER_ADVANCED_FAILSAFE_ENABLED
 # include "afs_copter.h"
#endif
#if TOY_MODE_ENABLED
 # include "toy_mode.h"
#endif
#if AP_WINCH_ENABLED
 # include <AP_Winch/AP_Winch.h>
#endif
#include <AP_RPM/AP_RPM.h>

#if AP_SCRIPTING_ENABLED
#include <AP_Scripting/AP_Scripting.h>
#endif

#if AC_CUSTOMCONTROL_MULTI_ENABLED
#include <AC_CustomControl/AC_CustomControl.h>                  // Custom control library
#endif

#if AP_AVOIDANCE_ENABLED && !AP_FENCE_ENABLED
  #error AC_Avoidance relies on AP_FENCE_ENABLED which is disabled
#endif

#if AP_OAPATHPLANNER_ENABLED && !AP_FENCE_ENABLED
  #error AP_OAPathPlanner relies on AP_FENCE_ENABLED which is disabled
#endif

#if MODE_AUTOROTATE_ENABLED && !AP_RPM_ENABLED
  #error AC_Autorotation relies on AP_RPM_ENABLED which is disabled
#endif

#if HAL_ADSB_ENABLED
#include "avoidance_adsb.h"
#endif
// Local modules
#include "Parameters.h"
#if USER_PARAMS_ENABLED
#include "UserParameters.h"
#endif
#include "mode.h"

class Copter : public AP_Vehicle {
public:
    friend class GCS_MAVLINK_Copter;
    friend class GCS_Copter;
    friend class AP_Rally_Copter;
    friend class Parameters;
    friend class ParametersG2;
    friend class AP_Avoidance_Copter;

#if AP_COPTER_ADVANCED_FAILSAFE_ENABLED
    friend class AP_AdvancedFailsafe_Copter;
#endif
    friend class AP_Arming_Copter;
#if AP_EXTERNAL_CONTROL_ENABLED
    friend class AP_ExternalControl_Copter;
#endif
    friend class ToyMode;
    friend class RC_Channel_Copter;
    friend class RC_Channels_Copter;

    friend class AutoTune;

    friend class Mode;
    friend class ModeAcro;
    friend class ModeAcro_Heli;
    friend class ModeAltHold;
    friend class ModeAuto;
    friend class ModeAutoTune;
    friend class ModeAvoidADSB;
    friend class ModeBrake;
    friend class ModeCircle;
    friend class ModeDrift;
    friend class ModeFlip;
    friend class ModeFlowHold;
    friend class ModeFollow;
    friend class ModeGuided;
    friend class ModeLand;
    friend class ModeLoiter;
    friend class ModePosHold;
    friend class ModeRTL;
    friend class ModeSmartRTL;
    friend class ModeSport;
    friend class ModeStabilize;
    friend class ModeStabilize_Heli;
    friend class ModeSystemId;
    friend class ModeThrow;
    friend class ModeZigZag;
    friend class ModeAutorotate;
    friend class ModeTurtle;

    friend class _AutoTakeoff;

    friend class PayloadPlace;

    Copter(void);

private:

    // key aircraft parameters passed to multiple libraries
    AP_MultiCopter aparm;

    // Global parameters are all contained within the 'g' class.
    Parameters g;
    ParametersG2 g2;

    // used to detect MAVLink acks from GCS to stop compassmot
    uint8_t command_ack_counter;

    // primary input control channels
    RC_Channel *channel_roll;
    RC_Channel *channel_pitch;
    RC_Channel *channel_throttle;
    RC_Channel *channel_yaw;

    // channel which is being used to tune a parameter value:
    RC_Channel *rc_tuning;

    // flight modes convenience array
    AP_Int8 *flight_modes;
    const uint8_t num_flight_modes = 6;

    AP_SurfaceDistance rangefinder_state {ROTATION_PITCH_270, 0U};
    AP_SurfaceDistance rangefinder_up_state {ROTATION_PITCH_90, 1U};

    // helper function to get inertially interpolated rangefinder height.
    bool get_rangefinder_height_interpolated_cm(int32_t& ret) const;

#if AP_RANGEFINDER_ENABLED
    class SurfaceTracking {
    public:

        // update_surface_offset - manages the vertical offset of the position controller to follow the
        //   measured ground or ceiling level measured using the range finder.
        void update_surface_offset();

        // target has already been set by terrain following so do not initialise again
        // this should be called by flight modes when switching from terrain following to surface tracking (e.g. ZigZag)
        void external_init();

        // get target and actual distances (in m) for logging purposes
        bool get_target_dist_for_logging(float &target_dist) const;
        float get_dist_for_logging() const;
        void invalidate_for_logging() { valid_for_logging = false; }

        // surface tracking surface
        enum class Surface {
            NONE = 0,
            GROUND = 1,
            CEILING = 2
        };
        // set surface to track
        void set_surface(Surface new_surface);
        // initialise surface tracking
        void init(Surface surf) { surface = surf; }

    private:
        Surface surface;
        uint32_t last_update_ms;    // system time of last update to target_alt_cm
        uint32_t last_glitch_cleared_ms;    // system time of last handle glitch recovery
        bool valid_for_logging;     // true if we have a desired target altitude
        bool reset_target;          // true if target should be reset because of change in surface being tracked
    } surface_tracking;
#endif

#if AP_RPM_ENABLED
    AP_RPM rpm_sensor;
#endif

    // Inertial Navigation EKF - different viewpoint
    AP_AHRS_View *ahrs_view;

    // Arming/Disarming management class
    AP_Arming_Copter arming;

    // Optical flow sensor
#if AP_OPTICALFLOW_ENABLED
    AP_OpticalFlow optflow;
#endif

    // external control library
#if AP_EXTERNAL_CONTROL_ENABLED
    AP_ExternalControl_Copter external_control;
#endif


    // system time in milliseconds of last recorded yaw reset from ekf
    uint32_t ekfYawReset_ms;
    int8_t ekf_primary_core;

    // vibration check
    struct {
        bool high_vibes;    // true while high vibration are detected
        uint32_t start_ms;  // system time high vibration were last detected
        uint32_t clear_ms;  // system time high vibrations stopped
    } vibration_check;

    // EKF variances are unfiltered and are designed to recover very quickly when possible
    // thus failsafes should be triggered on filtered values in order to avoid transient errors 
    LowPassFilterFloat pos_variance_filt;
    LowPassFilterFloat vel_variance_filt;
    bool variances_valid;
    uint32_t last_ekf_check_us;

    // takeoff check
    uint32_t takeoff_check_warning_ms;  // system time user was last warned of takeoff check failure

    // GCS selection
    GCS_Copter _gcs; // avoid using this; use gcs()
    GCS_Copter &gcs() { return _gcs; }

    // User variables
#ifdef USERHOOK_VARIABLES
# include USERHOOK_VARIABLES
#endif

    // ap_value calculates a 32-bit bitmask representing various pieces of
    // state about the Copter.  It replaces a global variable which was
    // used to track this state.
    uint32_t ap_value() const;

    // These variables are essentially global variables.  These should
    // be removed over time.  It is critical that the offsets of these
    // variables remain unchanged - the logging is dependent on this
    // ordering!
    struct PACKED {
        bool unused1;                        //  0
        bool unused_was_simple_mode_byte1;   //  1
        bool unused_was_simple_mode_byte2;   //  2
        bool pre_arm_rc_check;               //  3 true if rc input pre-arm checks have been completed successfully
        bool pre_arm_check;                  //  4 true if all pre-arm checks (rc, accel calibration, gps lock) have been performed
        bool auto_armed;                     //  5 stops auto missions from beginning until throttle is raised
        bool unused_log_started;             //  6
        bool land_complete;                  //  7 true if we have detected a landing
        bool new_radio_frame;                //  8 Set true if we have new PWM data to act on from the Radio
        bool unused_usb_connected;           //  9
        bool unused_receiver_present;        // 10
        bool compass_mot;                    // 11 true if we are currently performing compassmot calibration
        bool motor_test;                     // 12 true if we are currently performing the motors test
        bool initialised;                    // 13 true once the init_ardupilot function has completed.  Extended status to GCS is not sent until this completes
        bool land_complete_maybe;            // 14 true if we may have landed (less strict version of land_complete)
        bool throttle_zero;                  // 15 true if the throttle stick is at zero, debounced, determines if pilot intends shut-down when not using motor interlock
        bool system_time_set_unused;         // 16 true if the system time has been set from the GPS
        bool gps_glitching;                  // 17 true if GPS glitching is affecting navigation accuracy
        bool using_interlock;                // 18 aux switch motor interlock function is in use
        bool land_repo_active;               // 19 true if the pilot is overriding the landing position
        bool motor_interlock_switch;         // 20 true if pilot is requesting motor interlock enable
        bool in_arming_delay;                // 21 true while we are armed but waiting to spin motors
        bool initialised_params;             // 22 true when the all parameters have been initialised. we cannot send parameters to the GCS until this is done
        bool unused_compass_init_location;   // 23
        bool unused2_aux_switch_rc_override_allowed; // 24
        bool armed_with_airmode_switch;      // 25 we armed using a arming switch
        bool prec_land_active;               // 26 true if precland is active
    } ap;

    AirMode air_mode; // air mode is 0 = not-configured ; 1 = disabled; 2 = enabled;
    bool force_flying; // force flying is enabled when true;

    // This is the state of the flight control system
    // There are multiple states defined such as STABILIZE, ACRO,
    Mode *flightmode;

    RCMapper rcmap;

    // inertial nav alt when we armed
    float arming_altitude_m;

    // Failsafe
    struct {
        uint32_t terrain_first_failure_ms;  // the first time terrain data access failed - used to calculate the duration of the failure
        uint32_t terrain_last_failure_ms;   // the most recent time terrain data access failed

        int8_t radio_counter;            // number of iterations with throttle below throttle_fs_value

        uint8_t radio               : 1; // A status flag for the radio failsafe
        uint8_t gcs                 : 1; // A status flag for the ground station failsafe
        uint8_t ekf                 : 1; // true if ekf failsafe has occurred
        uint8_t terrain             : 1; // true if the missing terrain data failsafe has occurred
        uint8_t adsb                : 1; // true if an adsb related failsafe has occurred
        uint8_t deadreckon          : 1; // true if a dead reckoning failsafe has triggered
    } failsafe;

    bool any_failsafe_triggered() const {
        return failsafe.radio || battery.has_failsafed() || failsafe.gcs || failsafe.ekf || failsafe.terrain || failsafe.adsb || failsafe.deadreckon;
    }

    // dead reckoning state
    struct {
        bool active;        // true if dead reckoning (position estimate using estimated airspeed, no position or velocity source)
        bool timeout;       // true if dead reckoning has timedout and EKF's position and velocity estimate should no longer be trusted
        uint32_t start_ms;  // system time that EKF began deadreckoning
    } dead_reckoning;

    // Motor Output
    MOTOR_CLASS *motors;
    const struct AP_Param::GroupInfo *motors_var_info;

    int32_t _home_bearing;
    uint32_t _home_distance;

    // SIMPLE Mode
    // Used to track the orientation of the vehicle for Simple mode. This value is reset at each arming
    // or in SuperSimple mode when the vehicle leaves a 20m radius from home.
    enum class SimpleMode {
        NONE = 0,
        SIMPLE = 1,
        SUPERSIMPLE = 2,
    } simple_mode;

    float simple_cos_yaw;
    float simple_sin_yaw;
    int32_t super_simple_last_bearing;
    float super_simple_cos_yaw;
    float super_simple_sin_yaw;

    // Stores initial bearing when armed - initial simple bearing is modified in super simple mode so not suitable
    float initial_armed_bearing_rad;

    // Battery Sensors
    AP_BattMonitor battery{MASK_LOG_CURRENT,
                           FUNCTOR_BIND_MEMBER(&Copter::handle_battery_failsafe, void, const char*, const int8_t),
                           _failsafe_priorities};

#if OSD_ENABLED || OSD_PARAM_ENABLED
    AP_OSD osd;
#endif

    // Altitude
    int32_t baro_alt;            // barometer altitude in cm above home
    LowPassFilterVector3f land_accel_ef_filter; // accelerations for land and crash detector tests

    // filtered pilot's throttle input used to cancel landing if throttle held high
    LowPassFilterFloat rc_throttle_control_in_filter;

    // 3D Location vectors
    // Current location of the vehicle (altitude is relative to home)
    Location current_loc;

    // Attitude, Position and Waypoint navigation objects
    // To-Do: move inertial nav up or other navigation variables down here
    AC_AttitudeControl *attitude_control;
    const struct AP_Param::GroupInfo *attitude_control_var_info;
    AC_PosControl *pos_control;
    AC_WPNav *wp_nav;
    AC_Loiter *loiter_nav;

#if AC_CUSTOMCONTROL_MULTI_ENABLED
    AC_CustomControl custom_control{ahrs_view, attitude_control, motors, scheduler.get_loop_period_s()};
#endif

#if MODE_CIRCLE_ENABLED
    AC_Circle *circle_nav;
#endif

    // System Timers
    // --------------
    // arm_time_ms - Records when vehicle was armed. Will be Zero if we are disarmed.
    uint32_t arm_time_ms;

    // Camera
#if AP_CAMERA_ENABLED
    AP_Camera camera{MASK_LOG_CAMERA};
#endif

    // Camera/Antenna mount tracking and stabilisation stuff
#if HAL_MOUNT_ENABLED
    AP_Mount camera_mount;
#endif

#if AP_AVOIDANCE_ENABLED
    AC_Avoid avoid;
#endif

    // Rally library
#if HAL_RALLY_ENABLED
    AP_Rally_Copter rally;
#endif

    // Crop Sprayer
#if HAL_SPRAYER_ENABLED
    AC_Sprayer sprayer;
#endif

    // Parachute release
#if HAL_PARACHUTE_ENABLED
    AP_Parachute parachute;
#endif

    // Landing Gear Controller
#if AP_LANDINGGEAR_ENABLED
    AP_LandingGear landinggear;
#endif

    // terrain handling
#if AP_TERRAIN_AVAILABLE
    AP_Terrain terrain;
#endif

    // Precision Landing
#if AC_PRECLAND_ENABLED
    AC_PrecLand precland;
    AC_PrecLand_StateMachine precland_statemachine;
#endif

    // Pilot Input Management Library
    // Only used for Helicopter for now
#if FRAME_CONFIG == HELI_FRAME
    AC_InputManager_Heli input_manager;
#endif

#if HAL_ADSB_ENABLED
    AP_ADSB adsb;
#endif  // HAL_ADSB_ENABLED

#if AP_ADSB_AVOIDANCE_ENABLED
    // avoidance of adsb enabled vehicles (normally manned vehicles)
    AP_Avoidance_Copter avoidance_adsb{adsb};
#endif

    // last valid RC input time
    uint32_t last_radio_update_ms;

    // last esc calibration notification update
    uint32_t esc_calibration_notify_update_ms;

    // Top-level logic
    // setup the var_info table
    AP_Param param_loader;

#if FRAME_CONFIG == HELI_FRAME
    // Tradheli flags
    typedef struct {
        uint8_t dynamic_flight          : 1;    // 0   // true if we are moving at a significant speed (used to turn on/off leaky I terms)
        bool coll_stk_low                  ;    // 1   // true when collective stick is on lower limit
    } heli_flags_t;
    heli_flags_t heli_flags;

    int16_t hover_roll_trim_scalar_slew;
#endif

    // ground effect detector
    struct {
        bool takeoff_expected;
        bool touchdown_expected;
        uint32_t takeoff_time_ms;
        float takeoff_alt_cm;
    } gndeffect_state;

    bool standby_active;

    static const AP_Scheduler::Task scheduler_tasks[];
    static const AP_Param::Info var_info[];
    static const struct LogStructure log_structure[];

    // enum for ESC CALIBRATION
    enum ESCCalibrationModes : uint8_t {
        ESCCAL_NONE = 0,
        ESCCAL_PASSTHROUGH_IF_THROTTLE_HIGH = 1,
        ESCCAL_PASSTHROUGH_ALWAYS = 2,
        ESCCAL_AUTO = 3,
        ESCCAL_DISABLED = 9,
    };

    enum class FailsafeAction : uint8_t {
        NONE               = 0,
        LAND               = 1,
        RTL                = 2,
        SMARTRTL           = 3,
        SMARTRTL_LAND      = 4,
        TERMINATE          = 5,
        AUTO_DO_LAND_START = 6,
        BRAKE_LAND         = 7
    };

    enum class FailsafeOption {
        RC_CONTINUE_IF_AUTO             = (1<<0),   // 1
        GCS_CONTINUE_IF_AUTO            = (1<<1),   // 2
        RC_CONTINUE_IF_GUIDED           = (1<<2),   // 4
        CONTINUE_IF_LANDING             = (1<<3),   // 8
        GCS_CONTINUE_IF_PILOT_CONTROL   = (1<<4),   // 16
        RELEASE_GRIPPER                 = (1<<5),   // 32
    };


    enum class FlightOption : uint32_t {
        DISABLE_THRUST_LOSS_CHECK     = (1<<0),   // 1
        DISABLE_YAW_IMBALANCE_WARNING = (1<<1),   // 2
        RELEASE_GRIPPER_ON_THRUST_LOSS = (1<<2),  // 4
    };

    // type of fast rate attitude controller in operation
    enum class FastRateType : uint8_t {
        FAST_RATE_DISABLED            = 0,
        FAST_RATE_DYNAMIC             = 1,
        FAST_RATE_FIXED_ARMED         = 2,
        FAST_RATE_FIXED               = 3,
    };

    FastRateType get_fast_rate_type() const { return FastRateType(g2.att_enable.get()); }

    // returns true if option is enabled for this vehicle
    bool option_is_enabled(FlightOption option) const {
        return (g2.flight_options & uint32_t(option)) != 0;
    }

    static constexpr int8_t _failsafe_priorities[] = {
                                                      (int8_t)FailsafeAction::TERMINATE,
                                                      (int8_t)FailsafeAction::LAND,
                                                      (int8_t)FailsafeAction::RTL,
                                                      (int8_t)FailsafeAction::SMARTRTL_LAND,
                                                      (int8_t)FailsafeAction::SMARTRTL,
                                                      (int8_t)FailsafeAction::NONE,
                                                      -1 // the priority list must end with a sentinel of -1
                                                     };

    #define FAILSAFE_LAND_PRIORITY 1
    static_assert(_failsafe_priorities[FAILSAFE_LAND_PRIORITY] == (int8_t)FailsafeAction::LAND,
                  "FAILSAFE_LAND_PRIORITY must match the entry in _failsafe_priorities");
    static_assert(_failsafe_priorities[ARRAY_SIZE(_failsafe_priorities) - 1] == -1,
                  "_failsafe_priorities is missing the sentinel");



    // AP_State.cpp
    void set_auto_armed(bool b);
    void set_simple_mode(SimpleMode b);
    void set_failsafe_radio(bool b);
    void set_failsafe_gcs(bool b);
    void update_using_interlock();

    // Copter.cpp
    void get_scheduler_tasks(const AP_Scheduler::Task *&tasks,
                             uint8_t &task_count,
                             uint32_t &log_bit) override;
#if AP_SCRIPTING_ENABLED || AP_EXTERNAL_CONTROL_ENABLED
#if MODE_GUIDED_ENABLED
    bool set_target_location(const Location& target_loc) override;
    bool start_takeoff(const float alt) override;
#endif // MODE_GUIDED_ENABLED
#endif // AP_SCRIPTING_ENABLED || AP_EXTERNAL_CONTROL_ENABLED

#if AP_SCRIPTING_ENABLED
#if MODE_GUIDED_ENABLED
    bool get_target_location(Location& target_loc) override;
    bool update_target_location(const Location &old_loc, const Location &new_loc) override;
    bool set_target_pos_NED(const Vector3f& target_pos, bool use_yaw, float yaw_deg, bool use_yaw_rate, float yaw_rate_degs, bool yaw_relative, bool terrain_alt) override;
    bool set_target_posvel_NED(const Vector3f& target_pos, const Vector3f& target_vel) override;
    bool set_target_posvelaccel_NED(const Vector3f& target_pos, const Vector3f& target_vel, const Vector3f& target_accel, bool use_yaw, float yaw_deg, bool use_yaw_rate, float yaw_rate_degs, bool yaw_relative) override;
    bool set_target_velocity_NED(const Vector3f& vel_ned) override;
    bool set_target_velaccel_NED(const Vector3f& target_vel, const Vector3f& target_accel, bool use_yaw, float yaw_deg, bool use_yaw_rate, float yaw_rate_degs, bool relative_yaw) override;
    bool set_target_angle_and_climbrate(float roll_deg, float pitch_deg, float yaw_deg, float climb_rate_ms, bool use_yaw_rate, float yaw_rate_degs) override;
    bool set_target_rate_and_throttle(float roll_rate_dps, float pitch_rate_dps, float yaw_rate_dps, float throttle) override;

    // Register a custom mode with given number and names
    AP_Vehicle::custom_mode_state* register_custom_mode(const uint8_t number, const char* full_name, const char* short_name) override;
#endif
#if MODE_CIRCLE_ENABLED
    bool get_circle_radius(float &radius_m) override;
    bool set_circle_rate(float rate_dps) override;
#endif
    bool set_desired_speed(float speed) override;
#if MODE_AUTO_ENABLED
    bool nav_scripting_enable(uint8_t mode) override;
    bool nav_script_time(uint16_t &id, uint8_t &cmd, float &arg1, float &arg2, int16_t &arg3, int16_t &arg4) override;
    void nav_script_time_done(uint16_t id) override;
#endif
    // lua scripts use this to retrieve EKF failsafe state
    // returns true if the EKF failsafe has triggered
    bool has_ekf_failsafed() const override;
#endif // AP_SCRIPTING_ENABLED
    bool is_landing() const override;
    bool is_taking_off() const override;
    void rc_loop();
    void throttle_loop();
    void update_batt_compass(void);
    void loop_rate_logging();
    void ten_hz_logging_loop();
    void twentyfive_hz_logging();
    void three_hz_loop();
    void one_hz_loop();
    void init_simple_bearing();
    void update_simple_mode(void);
    void update_super_simple_bearing(bool force_update);
    void read_AHRS(void);
    void update_altitude();
    bool get_wp_distance_m(float &distance) const override;
    bool get_wp_bearing_deg(float &bearing) const override;
    bool get_wp_crosstrack_error_m(float &xtrack_error) const override;
    bool get_rate_ef_targets(Vector3f& rate_ef_targets) const override;

    // Attitude.cpp
    void update_throttle_hover();
    float get_pilot_desired_climb_rate();
    float get_non_takeoff_throttle();
    void set_accel_throttle_I_from_pilot_throttle();
    void rotate_body_frame_to_NE(float &x, float &y);
    uint16_t get_pilot_speed_dn() const;
    void run_rate_controller_main();

    // if AP_INERTIALSENSOR_FAST_SAMPLE_WINDOW_ENABLED
    struct RateControllerRates {
        uint8_t fast_logging_rate;
        uint8_t medium_logging_rate;
        uint8_t filter_rate;
        uint8_t main_loop_rate;
    };

    uint8_t calc_gyro_decimation(uint8_t gyro_decimation, uint16_t rate_hz);
    void rate_controller_thread();
    void rate_controller_filter_update();
    void rate_controller_log_update();
    void rate_controller_set_rates(uint8_t rate_decimation, RateControllerRates& rates, bool warn_cpu_high);
    void enable_fast_rate_loop(uint8_t rate_decimation, RateControllerRates& rates);
    void disable_fast_rate_loop(RateControllerRates& rates);
    void update_dynamic_notch_at_specified_rate_main();
    // endif AP_INERTIALSENSOR_FAST_SAMPLE_WINDOW_ENABLED

#if AC_CUSTOMCONTROL_MULTI_ENABLED
    void run_custom_controller() { custom_control.update(); }
#endif

    // avoidance.cpp
    void low_alt_avoidance();

#if HAL_ADSB_ENABLED || AP_ADSB_AVOIDANCE_ENABLED
    // avoidance_adsb.cpp
    void avoidance_adsb_update(void);
#endif  // HAL_ADSB_ENABLED || AP_ADSB_AVOIDANCE_ENABLED

    // baro_ground_effect.cpp
    void update_ground_effect_detector(void);
    void update_ekf_terrain_height_stable();

    // commands.cpp
    void update_home_from_EKF();
    void set_home_to_current_location_inflight();
    bool set_home_to_current_location(bool lock) override WARN_IF_UNUSED;
    bool set_home(const Location& loc, bool lock) override WARN_IF_UNUSED;

    // compassmot.cpp
    MAV_RESULT mavlink_compassmot(const GCS_MAVLINK &gcs_chan);

    // crash_check.cpp
    void crash_check();
    void thrust_loss_check();
    void yaw_imbalance_check();
    LowPassFilterFloat yaw_I_filt{0.05f};
    uint32_t last_yaw_warn_ms;
    void parachute_check();
    void parachute_release();
    void parachute_manual_release();

    // ekf_check.cpp
    void ekf_check();
    bool ekf_over_threshold();
    void failsafe_ekf_event();
    void failsafe_ekf_off_event(void);
    void failsafe_ekf_recheck();
    void check_ekf_reset();
    void check_vibration();

    // esc_calibration.cpp
    void esc_calibration_startup_check();
    void esc_calibration_passthrough();
    void esc_calibration_auto();
    void esc_calibration_notify();
    void esc_calibration_setup();

    // events.cpp
    bool failsafe_option(FailsafeOption opt) const;
    void failsafe_radio_on_event();
    void failsafe_radio_off_event();
    void handle_battery_failsafe(const char* type_str, const int8_t action);
    void failsafe_gcs_check();
    void failsafe_gcs_on_event(void);
    void failsafe_gcs_off_event(void);
    void failsafe_terrain_check();
    void failsafe_terrain_set_status(bool data_ok);
    void failsafe_terrain_on_event();
    void gpsglitch_check();
    void failsafe_deadreckon_check();
    void set_mode_RTL_or_land_with_pause(ModeReason reason);
    void set_mode_SmartRTL_or_RTL(ModeReason reason);
    void set_mode_SmartRTL_or_land_with_pause(ModeReason reason);
    void set_mode_auto_do_land_start_or_RTL(ModeReason reason);
    void set_mode_brake_or_land_with_pause(ModeReason reason);
    bool should_disarm_on_failsafe();
    void do_failsafe_action(FailsafeAction action, ModeReason reason);
    void announce_failsafe(const char *type, const char *action_undertaken=nullptr);

    // failsafe.cpp
    void failsafe_enable();
    void failsafe_disable();
#if AP_COPTER_ADVANCED_FAILSAFE_ENABLED
    void afs_fs_check(void);
#endif

    // fence.cpp
#if AP_FENCE_ENABLED
    void fence_check();
    void fence_checks_async() override;
#endif

    // heli.cpp
    void heli_init();
    void check_dynamic_flight(void);
    bool should_use_landing_swash() const;
    void update_heli_control_dynamics(void);
    void heli_update_landing_swash();
    float get_pilot_desired_rotor_speed() const;
    void heli_update_rotor_speed_targets();
    void heli_update_autorotation();
    void update_collective_low_flag(int16_t throttle_control);

    // inertia.cpp
    void read_inertia();

    // landing_detector.cpp
    void update_land_and_crash_detectors();
    void update_land_detector();
    void set_land_complete(bool b);
    void set_land_complete_maybe(bool b);
    void update_throttle_mix();
    bool get_force_flying() const;
#if HAL_LOGGING_ENABLED
    enum class LandDetectorLoggingFlag : uint16_t {
        LANDED               = 1U <<  0,
        LANDED_MAYBE         = 1U <<  1,
        LANDING              = 1U <<  2,
        STANDBY_ACTIVE       = 1U <<  3,
        WOW                  = 1U <<  4,
        RANGEFINDER_BELOW_2M = 1U <<  5,
        DESCENT_RATE_LOW     = 1U <<  6,
        ACCEL_STATIONARY     = 1U <<  7,
        LARGE_ANGLE_ERROR    = 1U <<  8,
        LARGE_ANGLE_REQUEST  = 1U <<  8,
        MOTOR_AT_LOWER_LIMIT = 1U <<  9,
        THROTTLE_MIX_AT_MIN  = 1U << 10,
    };
    struct {
        uint32_t last_logged_ms;
        uint32_t last_logged_count;
        uint16_t last_logged_flags;
    } land_detector;
    void Log_LDET(uint16_t logging_flags, uint32_t land_detector_count);
#endif

#if AP_LANDINGGEAR_ENABLED
    // landing_gear.cpp
    void landinggear_update();
#endif

    // standby.cpp
    void standby_update();

#if HAL_LOGGING_ENABLED
    // methods for AP_Vehicle:
    const AP_Int32 &get_log_bitmask() override { return g.log_bitmask; }
    const struct LogStructure *get_log_structures() const override {
        return log_structure;
    }
    uint8_t get_num_log_structures() const override;

    // Log.cpp
    void Log_Write_Control_Tuning();
    void Log_Write_Attitude();
    void Log_Write_Rate();
    void Log_Write_EKF_POS();
    void Log_Write_PIDS();
    void Log_Write_Data(LogDataID id, int32_t value);
    void Log_Write_Data(LogDataID id, uint32_t value);
    void Log_Write_Data(LogDataID id, int16_t value);
    void Log_Write_Data(LogDataID id, uint16_t value);
    void Log_Write_Data(LogDataID id, float value);
    void Log_Write_Parameter_Tuning(uint8_t param, float tuning_val, float tune_min, float tune_max);
    void Log_Video_Stabilisation();
    void Log_Write_Guided_Position_Target(ModeGuided::SubMode submode, const Vector3f& pos_target, bool terrain_alt, const Vector3f& vel_target, const Vector3f& accel_target);
    void Log_Write_Guided_Attitude_Target(ModeGuided::SubMode target_type, float roll, float pitch, float yaw, const Vector3f &ang_vel, float thrust, float climb_rate);
    void Log_Write_SysID_Setup(uint8_t systemID_axis, float waveform_magnitude, float frequency_start, float frequency_stop, float time_fade_in, float time_const_freq, float time_record, float time_fade_out);
    void Log_Write_SysID_Data(float waveform_time, float waveform_sample, float waveform_freq, float angle_x, float angle_y, float angle_z, float accel_x, float accel_y, float accel_z);
    void Log_Write_Vehicle_Startup_Messages();
    void Log_Write_Rate_Thread_Dt(float dt, float dtAvg, float dtMax, float dtMin);
#endif  // HAL_LOGGING_ENABLED

    // mode.cpp
    bool set_mode(Mode::Number mode, ModeReason reason);
    bool set_mode(const uint8_t new_mode, const ModeReason reason) override;
    ModeReason _last_reason;
    // called when an attempt to change into a mode is unsuccessful:
    void mode_change_failed(const Mode *mode, const char *reason);
    uint8_t get_mode() const override { return (uint8_t)flightmode->mode_number(); }
    bool current_mode_requires_mission() const override;
    void update_flight_mode();
    void notify_flight_mode();

    // Check if this mode can be entered from the GCS
    bool gcs_mode_enabled(const Mode::Number mode_num);

    // mode_land.cpp
    void set_mode_land_with_pause(ModeReason reason);
    bool landing_with_GPS();

    // motor_test.cpp
    void motor_test_output();
    bool mavlink_motor_control_check(const GCS_MAVLINK &gcs_chan, bool check_rc, const char* mode);
    MAV_RESULT mavlink_motor_test_start(const GCS_MAVLINK &gcs_chan, uint8_t motor_seq, uint8_t throttle_type, float throttle_value, float timeout_sec, uint8_t motor_count);
    void motor_test_stop();

    // motors.cpp
    void auto_disarm_check();
    void motors_output(bool full_push = true);
    void motors_output_main();
    void lost_vehicle_check();

    // navigation.cpp
    void run_nav_updates(void);
    int32_t home_bearing();
    uint32_t home_distance();

    // Parameters.cpp
    void load_parameters(void) override;
    void convert_pid_parameters(void);
#if HAL_PROXIMITY_ENABLED
    void convert_prx_parameters();
#endif
    void convert_lgr_parameters(void);

    // precision_landing.cpp
    void init_precland();
    void update_precland();

    // radio.cpp
    void default_dead_zones();
    void init_rc_in();
    void init_rc_out();
    void read_radio();
    void set_throttle_and_failsafe(uint16_t throttle_pwm);
    void set_throttle_zero_flag(int16_t throttle_control);
    void radio_passthrough_to_motors();
    int16_t get_throttle_mid(void);

    // sensors.cpp
    void read_barometer(void);
    void init_rangefinder(void);
    void read_rangefinder(void);
    bool rangefinder_alt_ok() const;
    bool rangefinder_up_ok() const;
    void update_rangefinder_terrain_offset();
    void update_optical_flow(void);

    // takeoff_check.cpp
    void takeoff_check();

    // system.cpp
    void init_ardupilot() override;
    void startup_INS_ground();
    bool position_ok() const;
    bool ekf_has_absolute_position() const;
    bool ekf_has_relative_position() const;
    bool ekf_alt_ok() const;
    void update_auto_armed();
    bool should_log(uint32_t mask);
    const char* get_frame_string() const;
    void allocate_motors(void);
    bool is_tradheli() const;

    // terrain.cpp
    void terrain_update();
    void terrain_logging();

    // tuning.cpp
    void tuning();

    // UserCode.cpp
    void userhook_init();
    void userhook_FastLoop();
    void userhook_50Hz();
    void userhook_MediumLoop();
    void userhook_SlowLoop();
    void userhook_SuperSlowLoop();
    void userhook_auxSwitch1(const RC_Channel::AuxSwitchPos ch_flag);
    void userhook_auxSwitch2(const RC_Channel::AuxSwitchPos ch_flag);
    void userhook_auxSwitch3(const RC_Channel::AuxSwitchPos ch_flag);

#if MODE_ACRO_ENABLED
#if FRAME_CONFIG == HELI_FRAME
    ModeAcro_Heli mode_acro;
#else
    ModeAcro mode_acro;
#endif
#endif
    ModeAltHold mode_althold;
#if MODE_AUTO_ENABLED
    ModeAuto mode_auto;
#endif
#if AUTOTUNE_ENABLED
    ModeAutoTune mode_autotune;
#endif
#if MODE_BRAKE_ENABLED
    ModeBrake mode_brake;
#endif
#if MODE_CIRCLE_ENABLED
    ModeCircle mode_circle;
#endif
#if MODE_DRIFT_ENABLED
    ModeDrift mode_drift;
#endif
#if MODE_FLIP_ENABLED
    ModeFlip mode_flip;
#endif
#if MODE_FOLLOW_ENABLED
    ModeFollow mode_follow;
#endif
#if MODE_GUIDED_ENABLED
    ModeGuided mode_guided;
#if AP_SCRIPTING_ENABLED
    // Custom modes registered at runtime
    ModeGuidedCustom *mode_guided_custom[5];
#endif
#endif
    ModeLand mode_land;
#if MODE_LOITER_ENABLED
    ModeLoiter mode_loiter;
#endif
#if MODE_POSHOLD_ENABLED
    ModePosHold mode_poshold;
#endif
#if MODE_RTL_ENABLED
    ModeRTL mode_rtl;
#endif
#if FRAME_CONFIG == HELI_FRAME
    ModeStabilize_Heli mode_stabilize;
#else
    ModeStabilize mode_stabilize;
#endif
#if MODE_SPORT_ENABLED
    ModeSport mode_sport;
#endif
#if MODE_SYSTEMID_ENABLED
    ModeSystemId mode_systemid;
#endif
#if AP_ADSB_AVOIDANCE_ENABLED
    ModeAvoidADSB mode_avoid_adsb;
#endif  // AP_ADSB_AVOIDANCE_ENABLED
#if MODE_THROW_ENABLED
    ModeThrow mode_throw;
#endif
#if MODE_GUIDED_NOGPS_ENABLED
    ModeGuidedNoGPS mode_guided_nogps;
#endif
#if MODE_SMARTRTL_ENABLED
    ModeSmartRTL mode_smartrtl;
#endif
#if MODE_FLOWHOLD_ENABLED
    ModeFlowHold mode_flowhold;
#endif
#if MODE_ZIGZAG_ENABLED
    ModeZigZag mode_zigzag;
#endif
#if MODE_AUTOROTATE_ENABLED
    ModeAutorotate mode_autorotate;
#endif
#if MODE_TURTLE_ENABLED
    ModeTurtle mode_turtle;
#endif

    // mode.cpp
    Mode *mode_from_mode_num(const Mode::Number mode);
    void exit_mode(Mode *&old_flightmode, Mode *&new_flightmode);

    bool started_rate_thread;
    bool using_rate_thread;

public:
    void failsafe_check();      // failsafe.cpp
};

extern Copter copter;

using AP_HAL::millis;
using AP_HAL::micros;
