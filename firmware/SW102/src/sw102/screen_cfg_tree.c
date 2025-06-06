#include "screen_cfg_utils.h"
#include "state.h"
#include "eeprom.h"
#include "ui.h"

extern const struct screen screen_main;

static void do_reset_trip_a(const struct configtree_t *ign);
//static void do_reset_trip_b(const struct configtree_t *ign);
static void do_set_Z2motor(const struct configtree_t *ign);
static void do_set_Z8motor(const struct configtree_t *ign);
static void do_set_TS85motor(const struct configtree_t *ign);
static void do_reset_ble(const struct configtree_t *ign);
static void do_reset_all(const struct configtree_t *ign);
void cfg_push_assist_screen(const struct configtree_t *ign);
void cfg_push_walk_assist_screen(const struct configtree_t *ign);
void cfg_push_calibration_screen(const struct configtree_t *ign);

static bool do_set_wh(const struct configtree_t *ign, int wh);
static bool do_set_odometer(const struct configtree_t *ign, int wh);

static const char *disable_enable[] = { "disable", "enable", 0 };
static const char *off_on[] = { "off", "on", 0 };
static const char *left_right[] = { "left", "right", 0 };

static const struct configtree_t cfgroot_tsdz2[] = {
	{ "Trip", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 18, 0, 128, (const struct configtree_t[]) {
		{ "Reset trip", F_BUTTON, .action = do_reset_trip_a },
		//{ "Reset trip B", F_BUTTON, .action = do_reset_trip_b },
		{},
	}}},
	{ "Bike", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
		{ "Max speed", F_NUMERIC, .numeric = &(const struct cfgnumeric_t){ PTRSIZE(ui_vars.wheel_max_speed_x10), 1, "km/h", 10, 990, 10 }},
		{ "Circumference", F_NUMERIC, .numeric = &(const struct cfgnumeric_t){ PTRSIZE(ui_vars.ui16_wheel_perimeter), 0, "mm", 1000, 3000, 10 }},
		//{ "Max Power limit", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_target_max_battery_power), 0, "W", 25, 1000, 25 }},
		{ "Assist with error", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_assist_whit_error_enabled), disable_enable }},
		{ "Throttle", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_throttle_feature_enabled), (const char*[]){ "disable", "6km/h only", 0 } }},
		//{ "Cruise", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_cruise_feature_enabled), (const char*[]){ "disable", "pedaling", "w/o pedaling", 0 } }},
    { "Cooling down", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_cooling_down_enabled), disable_enable } },
    { "Cooldown disabled time", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_cooldown_disabled_time), 0, "s" }},
    { "Cooldown enabled time", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_cooldown_enabled_time), 0, "s" }},
		{ "Password enable", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_password_enabled), disable_enable }},
		{ "Password", F_NUMERIC, .numeric = &(const struct cfgnumeric_t){ PTRSIZE(ui_vars.ui16_entered_password), 0, "", 1000, 9999 }},
		{ "Confirm", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_confirm_password), (const char*[]){ "logout", "login", "wait", "change", 0 } }},
		{},
	}}},
	{ "Battery", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
		{ "Max current", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_battery_max_current), 0, "A", 1, 20 }},
 		{ "Cut-off voltage", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_battery_low_voltage_cut_off_x10), 1, "V", 290, 430 }},
 		{ "Voltage cal %", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_battery_voltage_calibrate_percent), 0, "%", 50, 150 }},
		{ "Resistance", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_battery_pack_resistance_x1000), 0, "mohm", 0, 1000 }},
		{ "Voltage", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_battery_voltage_soc_x10), 1, "V" }},
		//{ "Est. resistance", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_battery_pack_resistance_estimated_x1000), 0, "mohm" }},
		{ "Power loss", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_battery_power_loss), 0, "W" }},
		{},
	}}},
	{ "SOC", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
		{ "Display", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_battery_soc_enable), (const char*[]){ "none", "charge %", "voltage", 0 }}},
		{ "Calculation", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_battery_soc_percent_calculation), (const char*[]){ "auto", "Wh", "volts", 0 }}},
		{ "Reset voltage", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_battery_voltage_reset_wh_counter_x10), 1, "V", 400, 600 }},
		{ "Total capacity", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui32_wh_x10_100_percent), 1, "Wh", 0, 9990, 100 }},
		{ "Used Wh", F_NUMERIC|F_CALLBACK,  .numeric_cb = &(const struct cfgnumeric_cb_t) { { PTRSIZE(ui_vars.ui32_wh_x10), 1, "Wh", 0, 9990, 100 }, do_set_wh }},
		//{ "Manual reset", F_OPTIONS, .options = &(const struct cfgoptions_t){ PTRSIZE(ui_vars.ui8_configuration_battery_soc_reset), (const char*[]){ "no", "yes", 0}}},
    { "Auto reset %", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_battery_soc_auto_reset), 0, "%", 0, 100 }},
		{},
	}}},
	{ "Motor", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
		{ "Motor voltage", F_OPTIONS, .options = &(const struct cfgoptions_t){ PTRSIZE(ui_vars.ui8_motor_type), (const char*[]){ "48V", "36V", 0}}},
		{ "Max current", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_motor_max_current), 0, "A", 1, 20 }},
		{ "Motor acceleration", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_motor_acceleration_adjustment), 0, "%", 0, 100 }},
		{ "Motor deceleration", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_motor_deceleration_adjustment), 0, "%", 0, 100 }},
		//{ "Current ramp", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_ramp_up_amps_per_second_x10), 1, "A", 4, 100 }},
		//{ "Control mode", F_OPTIONS, .options = &(const struct cfgoptions_t){ PTRSIZE(ui_vars.ui8_motor_current_control_mode), (const char*[]){ "power", "torque", "cadence", "eMTB", "hybrid", 0}}},
		//{ "Min current", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_motor_current_min_adc), 0, "steps", 0, 13 }},
		{ "Field weakening", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_field_weakening), disable_enable } },
		{},
	}}},
	//{ "Torque sensor", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
	//	{ "ADC Threshold", F_NUMERIC, .numeric = &(const struct cfgnumeric_t){ PTRSIZE(ui_vars.ui8_torque_sensor_adc_threshold), 0, "", 5, 100 }},
	//	{ "Assist w/o pedal", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_motor_assistance_startup_without_pedal_rotation), disable_enable }},
	//	{ "Coast brake", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_coast_brake_enable), disable_enable }},
	//	{ "Coast brake ADC", F_NUMERIC, .numeric = &(const struct cfgnumeric_t){ PTRSIZE(ui_vars.ui8_coast_brake_adc), 0, "", 5, 50 }},
	//	{ "Calibration", F_SUBMENU, .submenu = &(const struct scroller_config) { 20, 58, 36, 0, 128, (const struct configtree_t[]) {
	//		{ "Feature", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_torque_sensor_calibration_feature_enabled), disable_enable }},
	//		{ "Torque adc step", F_NUMERIC, .numeric = &(const struct cfgnumeric_t){ PTRSIZE(ui_vars.ui8_pedal_torque_per_10_bit_ADC_step_x100), 0, "", 20, 120 }},
	//		{ "Torque adc step adv", F_NUMERIC, .numeric = &(const struct cfgnumeric_t){ PTRSIZE(ui_vars.ui8_pedal_torque_per_10_bit_ADC_step_adv_x100), 0, "", 20, 50 }},
	//		{ "Torque offset adj", F_NUMERIC, .numeric = &(const struct cfgnumeric_t){ PTRSIZE(ui_vars.ui8_adc_pedal_torque_offset_adj), 0, "", 0, 34 }},
	//		{ "Torque range adj", F_NUMERIC, .numeric = &(const struct cfgnumeric_t){ PTRSIZE(ui_vars.ui8_adc_pedal_torque_range_adj), 0, "", 0, 40 }},
	//		{ "Torque angle adj", F_NUMERIC, .numeric = &(const struct cfgnumeric_t){ PTRSIZE(ui_vars.ui8_adc_pedal_torque_angle_adj_index), 0, "", 0, 40 }},
	//		{ "Torque adc offset", F_NUMERIC, .numeric = &(const struct cfgnumeric_t){ PTRSIZE(ui_vars.ui16_adc_pedal_torque_offset), 0, "", 0, 300 }},
	//		{ "Torque adc max", F_NUMERIC, .numeric = &(const struct cfgnumeric_t){ PTRSIZE(ui_vars.ui16_adc_pedal_torque_max), 0, "", 0, 500 }},
	//		{ "Weight on pedal", F_NUMERIC, .numeric = &(const struct cfgnumeric_t){ PTRSIZE(ui_vars.ui8_weight_on_pedal), 0, "kg", 20, 80 }},
	//		{ "Torque adc on weight", F_NUMERIC, .numeric = &(const struct cfgnumeric_t){ PTRSIZE(ui_vars.ui16_adc_pedal_torque_with_weight), 0, "", 100, 500 }},
	//		{ "ADC torque step calc", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_pedal_torque_ADC_step_calc_x100), 0, "" }},
			//{ "Left side", F_BUTTON, .action = cfg_push_calibration_screen },
			//{ "Right side", F_BUTTON, .action = cfg_push_calibration_screen },
	//		{},
	//	}}},
  //  { "Sensor filter", F_NUMERIC, .numeric = &(const struct cfgnumeric_t){ PTRSIZE(ui_vars.ui8_torque_sensor_filter), 0, "", 0, 100 }},
  //  { "Start pedal ground", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_torque_sensor_calibration_pedal_ground), left_right }},
	//	{},
	//}}},
	{ "Assist", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
	  { "Control mode", F_OPTIONS, .options = &(const struct cfgoptions_t){ PTRSIZE(ui_vars.ui8_motor_current_control_mode), (const char*[]){ "power", "torque", "cadence", "eMTB", "hybrid", 0}}},
	  { "Power Assist", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
	    { "Level 1", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[POWER_MODE][0]), 0, "", 1, 254 }},
	    { "Level 2", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[POWER_MODE][1]), 0, "", 1, 254 }},
	    { "Level 3", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[POWER_MODE][2]), 0, "", 1, 254 }},
	    { "Level 4", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[POWER_MODE][3]), 0, "", 1, 254 }},
	    { "Level 5", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[POWER_MODE][4]), 0, "", 1, 254 }},
	    {},
	  }}},
	  { "Torque Assist", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
	    { "Level 1", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[TORQUE_MODE][0]), 0, "", 1, 254 }},
	    { "Level 2", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[TORQUE_MODE][1]), 0, "", 1, 254 }},
	    { "Level 3", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[TORQUE_MODE][2]), 0, "", 1, 254 }},
	    { "Level 4", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[TORQUE_MODE][3]), 0, "", 1, 254 }},
	    { "Level 5", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[TORQUE_MODE][4]), 0, "", 1, 254 }},
	    {},
	  }}},
    { "Cadence Assist", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
      { "Level 1", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[CADENCE_MODE][0]), 0, "", 1, 254 }},
      { "Level 2", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[CADENCE_MODE][1]), 0, "", 1, 254 }},
      { "Level 3", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[CADENCE_MODE][2]), 0, "", 1, 254 }},
      { "Level 4", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[CADENCE_MODE][3]), 0, "", 1, 254 }},
      { "Level 5", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[CADENCE_MODE][4]), 0, "", 1, 254 }},
      {},
    }}},
    { "eMTB Assist", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
      { "Level 1", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[eMTB_MODE][0]), 0, "", 1, 20 }},
      { "Level 2", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[eMTB_MODE][1]), 0, "", 1, 20 }},
      { "Level 3", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[eMTB_MODE][2]), 0, "", 1, 20 }},
      { "Level 4", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[eMTB_MODE][3]), 0, "", 1, 20 }},
      { "Level 5", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_assist_level_factor[eMTB_MODE][4]), 0, "", 1, 20 }},
      {},
    }}},
	  {},
	}}},
	//{ "Assist-old", F_BUTTON, .action = cfg_push_assist_screen },
	//{ "Walk assist", F_BUTTON, .action = cfg_push_walk_assist_screen },
	{ "Startup power", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
	  { "Feature", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_startup_motor_power_boost_feature_enabled), disable_enable } },
	  { "Boost torque factor", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_startup_boost_torque_factor), 0, "%", 1, 500 }},
	  { "Boost cadence step", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_startup_boost_cadence_step), 0, "", 10, 50 }},
	  { "Boost at zero", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_startup_boost_at_zero), (const char*[]){ "cadence", "speed", 0 } } },
	  { "Startup assist", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_startup_assist_feature_enabled), disable_enable } },
	  { "Smooth start", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_smooth_start_enabled), disable_enable } },
	  { "Smooth start ramp", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_smooth_start_counter_set), 0, "%", 0, 100 }},
	  {},
	}}},
	//{ "Temperature", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
	//	{ "Feature", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_temperature_limit_feature_enabled), (const char*[]){ "disable", "temperature", "throttle", 0 } } },
	//	{ "Min limit", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_motor_temperature_min_value_to_limit), 0, "C", 30, 100 }},
	//	{ "Max limit", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_motor_temperature_max_value_to_limit), 0, "C", 30, 100 }},
	//	{},
	//}}},
	{ "Street mode", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
		{ "Feature", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_street_mode_function_enabled), disable_enable } },
		{ "Current status", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_street_mode_enabled), off_on } },
		{ "At startup", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_street_mode_enabled_on_startup), (const char*[]){ "no change", "activate", 0 } }},
		//{ "Hotkey", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_street_mode_hotkey_enabled), disable_enable } },
		{ "Speed limit", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_street_mode_speed_limit), 0, "km/h", 1, 99 }},
		//{ "Power limit", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_street_mode_power_limit), 0, "W", 25, 1000, 25 }},
		//{ "Throttle", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_street_mode_throttle_enabled), (const char*[]){ "disable", "6km/h only", 0 } } },
		//{ "Cruise", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_street_mode_cruise_enabled), (const char*[]){ "disable", "pedaling", "w/o pedaling", 0 } }},
		{},
	}}},
	{ "Various", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
		//{ "Fast stop", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_pedal_cadence_fast_stop), disable_enable } },
	  { "Screen size", F_OPTIONS, .options = &(const struct cfgoptions_t){ PTRSIZE(ui_vars.ui8_screen_size), (const char*[]){ "small", "big", 0}}},
    { "Motor type", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
      { "Current motor type", F_OPTIONS|F_RO, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_motor_version), (const char*[]){ "Z2", "Z8", "TS85", 0}}},
      { "Set motor type", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 18, 0, 128, (const struct configtree_t[]) {
        { "Z2", F_BUTTON, .action = do_set_Z2motor },
        { "Z8", F_BUTTON, .action = do_set_Z8motor },
        { "TS85", F_BUTTON, .action = do_set_TS85motor },
        {}
      }}},
      {}
    }}},
	  //{ "Light config", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_lights_configuration), 0, "", 0, 8 }},
		{ "Odometer", F_NUMERIC|F_CALLBACK, .numeric_cb = &(const struct cfgnumeric_cb_t) { { PTRSIZE(ui_vars.ui32_odometer_x10), 1, "km", 0, UINT32_MAX }, do_set_odometer }},
		{ "Auto power off", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_lcd_power_off_time_minutes), 0, "min", 0, 255 }},
		{ "Reset BLE peers", F_BUTTON, .action = do_reset_ble },
		{ "Reset all settings", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 18, 0, 128, (const struct configtree_t[]) {
			{ "Confirm reset all", F_BUTTON, .action = do_reset_all },
			{}
		}}},
		{}
	}}},
	{ "Technical", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
		{ "ADC battery current", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_adc_battery_current), 0, "" }},
		{ "ADC throttle sensor", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_adc_throttle), 0, ""}},
		{ "Throttle sensor", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_throttle), 0, ""}},
		{ "ADC torque sensor", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_adc_pedal_torque_sensor), 0, ""}},
		{ "ADC torque delta", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_adc_pedal_torque_delta), 0, ""}},
		{ "ADC torque boost", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_adc_pedal_torque_delta_boost), 0, ""}},
		{ "Pedal side", F_OPTIONS|F_RO, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_pas_pedal_right), left_right }},
		{ "Weight with offset", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_pedal_weight_with_offset), 0, "kg" }},
		{ "Weight", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_pedal_weight), 0, "kg" }},
		{ "Cadence", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_pedal_cadence), 0, "rpm" }},
		{ "PWM duty-cycle", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_duty_cycle), 0, "" }},
		{ "Motor speed", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_motor_speed_erps), 0, "" }},
		{ "Motor FOC", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_foc_angle), 0, "" }},
		{ "Hall sensors", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_motor_hall_sensors), 0, "" }},
		{},
	}}},
	{}
};

static const struct configtree_t cfgroot_tsdz8[] = {
  { "Trip", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 18, 0, 128, (const struct configtree_t[]) {
    { "Reset trip", F_BUTTON, .action = do_reset_trip_a },
    //{ "Reset trip B", F_BUTTON, .action = do_reset_trip_b },
    {},
  }}},
  { "Bike", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
    { "Max speed", F_NUMERIC, .numeric = &(const struct cfgnumeric_t){ PTRSIZE(ui_vars.wheel_max_speed_x10), 1, "km/h", 10, 990, 10 }},
    { "Circumference", F_NUMERIC, .numeric = &(const struct cfgnumeric_t){ PTRSIZE(ui_vars.ui16_wheel_perimeter), 0, "mm", 1000, 3000, 10 }},
    //{ "Assist with error", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_assist_whit_error_enabled), disable_enable }},
    //{ "Throttle", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_throttle_feature_enabled), (const char*[]){ "disable", "6km/h only", 0 } }},
    //{ "Cruise", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_cruise_feature_enabled), (const char*[]){ "disable", "pedaling", "w/o pedaling", 0 } }},
    { "Cooling down", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_cooling_down_enabled), disable_enable } },
    { "Cooldown disabled time", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_cooldown_disabled_time), 0, "s" }},
    { "Cooldown enabled time", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_cooldown_enabled_time), 0, "s" }},
    { "Password enable", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_password_enabled), disable_enable }},
    { "Password", F_NUMERIC, .numeric = &(const struct cfgnumeric_t){ PTRSIZE(ui_vars.ui16_entered_password), 0, "", 1000, 9999 }},
    { "Confirm", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_confirm_password), (const char*[]){ "logout", "login", "wait", "change", 0 } }},
    {},
  }}},
  { "Battery", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
    { "Max current", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_battery_max_current), 0, "A", 1, 20 }},
    { "Cut-off voltage", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_battery_low_voltage_cut_off_x10), 1, "V", 290, 430 }},
    { "Voltage cal %", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_battery_voltage_calibrate_percent), 0, "%", 50, 150 }},
    { "Resistance", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_battery_pack_resistance_x1000), 0, "mohm", 0, 1000 }},
    { "Voltage", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_battery_voltage_soc_x10), 1, "V" }},
    //{ "Est. resistance", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_battery_pack_resistance_estimated_x1000), 0, "mohm" }},
    { "Power loss", F_NUMERIC|F_RO, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_battery_power_loss), 0, "W" }},
    {},
  }}},
  { "SOC", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
    { "Display", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_battery_soc_enable), (const char*[]){ "none", "charge %", "voltage", 0 }}},
    { "Calculation", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_battery_soc_percent_calculation), (const char*[]){ "auto", "Wh", "volts", 0 }}},
    { "Reset voltage", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_battery_voltage_reset_wh_counter_x10), 1, "V", 400, 600 }},
    { "Total capacity", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui32_wh_x10_100_percent), 1, "Wh", 0, 9990, 100 }},
    { "Used Wh", F_NUMERIC|F_CALLBACK,  .numeric_cb = &(const struct cfgnumeric_cb_t) { { PTRSIZE(ui_vars.ui32_wh_x10), 1, "Wh", 0, 9990, 100 }, do_set_wh }},
    //{ "Manual reset", F_OPTIONS, .options = &(const struct cfgoptions_t){ PTRSIZE(ui_vars.ui8_configuration_battery_soc_reset), (const char*[]){ "no", "yes", 0}}},
    { "Auto reset %", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_battery_soc_auto_reset), 0, "%", 0, 100 }},
    {},
  }}},
  { "Motor", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
    { "Motor voltage", F_OPTIONS, .options = &(const struct cfgoptions_t){ PTRSIZE(ui_vars.ui8_motor_type), (const char*[]){ "48V", "36V", 0}}},
    { "Max current", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_motor_max_current), 0, "A", 1, 20 }},
    //{ "Motor acceleration", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_motor_acceleration_adjustment), 0, "%", 0, 100 }},
    //{ "Motor deceleration", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_motor_deceleration_adjustment), 0, "%", 0, 100 }},
    //{ "Current ramp", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_ramp_up_amps_per_second_x10), 1, "A", 4, 100 }},
    //{ "Control mode", F_OPTIONS, .options = &(const struct cfgoptions_t){ PTRSIZE(ui_vars.ui8_motor_current_control_mode), (const char*[]){ "power", "torque", "cadence", "eMTB", "hybrid", 0}}},
    //{ "Min current", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_motor_current_min_adc), 0, "steps", 0, 13 }},
    //{ "Field weakening", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_field_weakening), disable_enable } },
    {},
  }}},
  { "Street mode", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
    { "Feature", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_street_mode_function_enabled), disable_enable } },
    { "Current status", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_street_mode_enabled), off_on } },
    { "At startup", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_street_mode_enabled_on_startup), (const char*[]){ "no change", "activate", 0 } }},
    //{ "Hotkey", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_street_mode_hotkey_enabled), disable_enable } },
    { "Speed limit", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_street_mode_speed_limit), 0, "km/h", 1, 99 }},
    //{ "Power limit", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui16_street_mode_power_limit), 0, "W", 25, 1000, 25 }},
    //{ "Throttle", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_street_mode_throttle_enabled), (const char*[]){ "disable", "6km/h only", 0 } } },
    //{ "Cruise", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_street_mode_cruise_enabled), (const char*[]){ "disable", "pedaling", "w/o pedaling", 0 } }},
    {},
  }}},
  { "Various", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
    //{ "Fast stop", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_pedal_cadence_fast_stop), disable_enable } },
    { "Screen size", F_OPTIONS, .options = &(const struct cfgoptions_t){ PTRSIZE(ui_vars.ui8_screen_size), (const char*[]){ "small", "big", 0}}},
    { "Motor type", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 36, 0, 128, (const struct configtree_t[]) {
      { "Current motor type", F_OPTIONS|F_RO, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_motor_version), (const char*[]){ "Z2", "Z8", "TS85", 0}}},
      { "Set motor type", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 18, 0, 128, (const struct configtree_t[]) {
        { "Z2", F_BUTTON, .action = do_set_Z2motor },
        { "Z8", F_BUTTON, .action = do_set_Z8motor },
        { "TS85", F_BUTTON, .action = do_set_TS85motor },
        {}
      }}},
      {}
    }}},
    //{ "Light config", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_lights_configuration), 0, "", 0, 8 }},
    { "Odometer", F_NUMERIC|F_CALLBACK, .numeric_cb = &(const struct cfgnumeric_cb_t) { { PTRSIZE(ui_vars.ui32_odometer_x10), 1, "km", 0, UINT32_MAX }, do_set_odometer }},
    { "Auto power off", F_NUMERIC, .numeric = &(const struct cfgnumeric_t) { PTRSIZE(ui_vars.ui8_lcd_power_off_time_minutes), 0, "min", 0, 255 }},
    { "Reset BLE peers", F_BUTTON, .action = do_reset_ble },
    { "Reset all settings", F_SUBMENU, .submenu = &(const struct scroller_config){ 20, 58, 18, 0, 128, (const struct configtree_t[]) {
      { "Confirm reset all", F_BUTTON, .action = do_reset_all },
      {}
    }}},
    {}
  }}},
  {}
};


const struct scroller_config cfg_rootz2 = { 20, 58, 18, 0, 128,  cfgroot_tsdz2 };

const struct scroller_config cfg_rootz8 = { 20, 58, 18, 0, 128,  cfgroot_tsdz8 };


static int tmp_rescale = 100;
bool enumerate_assist_levels(const struct scroller_config *cfg, int index, const struct scroller_item_t **it);
bool enumerate_walk_assist_levels(const struct scroller_config *cfg, int index, const struct scroller_item_t **it);

bool do_change_assist_levels(const struct configtree_t *ign, int newv);
bool rescale_update(const struct configtree_t *it, int value);
void rescale_preview(const struct configtree_t *it, int value);
void rescale_revert(const struct configtree_t *it);
void do_resize_assist_levels(const struct configtree_t *ign);
void do_interpolate_assist_levels(const struct configtree_t *ign);

const struct assist_scroller_config cfg_assist = { { 20, 26, 36, 0, 76, (const struct configtree_t[]) {
	{ "Assist levels", F_NUMERIC | F_CALLBACK, .numeric_cb = &(const struct cfgnumeric_cb_t) { { PTRSIZE(ui_vars.ui8_number_of_assist_levels), 0, "", 1, 20 }, do_change_assist_levels }},
	{ "Rescale all", F_NUMERIC | F_CALLBACK, .numeric_cb = &(const struct cfgnumeric_cb_t) { { PTRSIZE(tmp_rescale), 0, "%", 25, 400, 5 }, rescale_update, rescale_preview, rescale_revert }},
	// this is a template
	{ (char[10]){}, F_NUMERIC | F_CALLBACK, .numeric_cb = &(struct cfgnumeric_cb_t) { { { 0, 0 }, 0, "%", 1, 3200 /* we could go up to about 300x assist, but even 30x is absurd */ } }}
}, enumerate_assist_levels }, 2 };

const struct scroller_config cfg_levels_extend = { 20, 26, 18, 0, 76, (const struct configtree_t[]) {
	{ "Interpolate", F_BUTTON, .action = do_interpolate_assist_levels },
	{ "Add higher", F_BUTTON, .action = do_resize_assist_levels },
	{}
}};

const struct scroller_config cfg_levels_truncate = { 20, 26, 18, 0, 76, (const struct configtree_t[]) {
	{ "Interpolate", F_BUTTON, .action = do_interpolate_assist_levels },
	{ "Keep lowest", F_BUTTON, .action = do_resize_assist_levels },
	{}
}};

const struct assist_scroller_config cfg_walk_assist = { { 20, 26, 36, 0, 76, (const struct configtree_t[]) {
	{ "Feature", F_OPTIONS, .options = &(const struct cfgoptions_t) { PTRSIZE(ui_vars.ui8_walk_assist_feature_enabled), disable_enable } },
	// this is a template
	{ (char[10]){}, F_NUMERIC | F_CALLBACK, .numeric_cb = &(struct cfgnumeric_cb_t) { { { 0, 0 }, 0, "%", 1, 100 } }}
}, enumerate_assist_levels }, 1};

bool enumerate_calibration(const struct scroller_config *cfg, int index, const struct scroller_item_t **it);
const struct scroller_config cfg_calibration = { 20, 26, 36, 0, 76, (const struct configtree_t[]) {
	// these are templates
	{ (char[10]){}, F_NUMERIC | F_CALLBACK, .numeric_cb = &(struct cfgnumeric_cb_t) { { { 0, 0 }, 0, "kg", 0, 200 }}},
	{ (char[10]){}, F_NUMERIC | F_CALLBACK, .numeric_cb = &(struct cfgnumeric_cb_t) { { { 0, 0 }, 0, "", 0, 1023 }}},
}, enumerate_calibration };

static void do_reset_trip_a(const struct configtree_t *ign)
{
  ui_vars.ui8_flag_reset_trip = 1;
	sstack_pop();
}

/*static void do_reset_trip_b(const struct configtree_t *ign)
{
	rt_vars.ui32_trip_b_distance_x1000 = 0;
	rt_vars.ui32_trip_b_time = 0;
	rt_vars.ui16_trip_b_avg_speed_x10 = 0;
	rt_vars.ui16_trip_b_max_speed_x10 = 0;
	sstack_pop();
}*/

static void do_set_Z2motor(const struct configtree_t *ign)
{
  ui_vars.ui8_battery_max_current = 15;
  ui_vars.ui16_battery_low_voltage_cut_off_x10 = 300;
  ui_vars.ui16_battery_voltage_reset_wh_counter_x10 = 415;
  ui_vars.ui8_motor_type = 1;
  ui_vars.ui8_motor_max_current = 15;
  ui_vars.ui8_motor_version = 0;
  ui_vars.ui32_wh_x10_100_percent = 5000;
  sstack_pop();
}

static void do_set_Z8motor(const struct configtree_t *ign)
{
  ui_vars.ui8_battery_max_current = 23;
  ui_vars.ui16_battery_low_voltage_cut_off_x10 = 400;
  ui_vars.ui16_battery_voltage_reset_wh_counter_x10 = 540;
  ui_vars.ui8_motor_type = 0;
  ui_vars.ui8_motor_max_current = 23;
  ui_vars.ui8_motor_version = 1;
  ui_vars.ui32_wh_x10_100_percent = 7000;
  ui_vars.ui8_cooling_down_enabled = 1;
  sstack_pop();
}

static void do_set_TS85motor(const struct configtree_t *ign)
{
  ui_vars.ui8_battery_max_current = 15;
  ui_vars.ui16_battery_low_voltage_cut_off_x10 = 300;
  ui_vars.ui16_battery_voltage_reset_wh_counter_x10 = 415;
  ui_vars.ui8_motor_type = 1;
  ui_vars.ui8_motor_max_current = 15;
  ui_vars.ui8_motor_version = 2;
  ui_vars.ui32_wh_x10_100_percent = 2500;
  sstack_pop();
}

#if defined(NRF51)
#include "peer_manager.h"
#endif

static void do_reset_ble(const struct configtree_t *ign)
{
#if defined(NRF51)
	// TODO: fist disable any connection
	// Warning: Use this (pm_peers_delete) function only when not connected or connectable. If a peer is or becomes connected
	// or a PM_PEER_DATA_FUNCTIONS function is used during this procedure (until the success or failure event happens),
	// the behavior is undefined.
	pm_peers_delete();
#endif
	sstack_pop();
}

static void do_reset_all(const struct configtree_t *ign)
{
	eeprom_init_defaults();
	showScreen(&screen_main);
}

static bool do_set_wh(const struct configtree_t *ign, int wh)
{
	reset_wh();
	ui_vars.ui32_wh_x10_offset = wh;
	return true;
}

static bool do_set_odometer(const struct configtree_t *ign, int v)
{
	// FIXME rt_vars?
	rt_vars.ui32_odometer_x10 = v;
	return true;
}


