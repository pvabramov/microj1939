/**
 * @file j1939_std_addresses.h
 *
 * @brief
 */


#ifndef J1939_STD_ADDRESSES_H_
#define J1939_STD_ADDRESSES_H_

/* SAE J1939 base document: Appendix B, Table B2 */

#define J1939_STD_ADDR_ENGINE0                                  0
#define J1939_STD_ADDR_ENGINE1                                  1
#define J1939_STD_ADDR_TURBOCHARGER                             2
#define J1939_STD_ADDR_TRANSMITION1                             3
#define J1939_STD_ADDR_TRANSMITION2                             4
#define J1939_STD_ADDR_SHIFT_CONSOLE_PRIMARY                    5
#define J1939_STD_ADDR_SHIFT_CONSOLE_SECONDARY                  6
#define J1939_STD_ADDR_POWER_TAKEOFF_MAIN                       7
#define J1939_STD_ADDR_AXLE_STEERING                            8
#define J1939_STD_ADDR_AXLE_DRIVE1                              9
#define J1939_STD_ADDR_AXLE_DRIVE2                              10
#define J1939_STD_ADDR_BRAKES_SYS_CONTROLLER                    11
#define J1939_STD_ADDR_BRAKES_STEER_AXLE                        12
#define J1939_STD_ADDR_BRAKES_DRIVE_AXLE1                       13
#define J1939_STD_ADDR_BRAKES_DRIVE_AXLE2                       14
#define J1939_STD_ADDR_RETARDER_ENGINE                          15
#define J1939_STD_ADDR_RETARDER_DRIVELINE                       16
#define J1939_STD_ADDR_CRUISE_CONTROL                           17
#define J1939_STD_ADDR_FUEL_SYSTEM                              18
#define J1939_STD_ADDR_STEERING_CONTROLLER                      19
#define J1939_STD_ADDR_SUSPENSION_STEER_AXLE                    20
#define J1939_STD_ADDR_SUSPENSION_DRIVE_AXLE1                   21
#define J1939_STD_ADDR_SUSPENSION_DRIVE_AXLE2                   22
#define J1939_STD_ADDR_INSTRUMENT_CLUSTER                       23
#define J1939_STD_ADDR_TRIP_RECORDER                            24
#define J1939_STD_ADDR_PASSENGER_OPERATOR_CLIMATE_CONTROL1      25
#define J1939_STD_ADDR_ELECTRICAL_CHARGING_SYSTEM               26
#define J1939_STD_ADDR_AERODYNAMIC_CONTROL                      27
#define J1939_STD_ADDR_VEHICLE_NAVIGATION                       28
#define J1939_STD_ADDR_VEHICLE_SECURITY                         29
#define J1939_STD_ADDR_ELECTICAL_SYSTEM                         30
#define J1939_STD_ADDR_STARTER_SYSTEM                           31
#define J1939_STD_ADDR_TRACTOR_TRAILER_BRIDGE1                  32
#define J1939_STD_ADDR_BODY_CONTROLLER                          33
#define J1939_STD_ADDR_AUX_VALVE_CONTROL                        34
#define J1939_STD_ADDR_HITCH_CONTROL                            35
#define J1939_STD_ADDR_POWER_TAKEOFF_FRONT                      36
#define J1939_STD_ADDR_OFF_VEHICLE_GATEWAY                      37
#define J1939_STD_ADDR_VIRTUAL_TERMINAL                         38 // in cab.
#define J1939_STD_ADDR_MANAGEMENT_COMPUTER1                     39
#define J1939_STD_ADDR_CAB_DISPLAY                              40
#define J1939_STD_ADDR_CAB_DISPLAY_SPRAYING                     41
#define J1939_STD_ADDR_HEADWAY_CONTROLLER                       42
#define J1939_STD_ADDR_ON_BOARD_DIAGNOSTIC_UNIT                 43
// 44
#define J1939_STD_ADDR_ENDURANCE_BRAKING_SYSTEM                 45
#define J1939_STD_ADDR_HYDRAULIC_PUMP_CONTROLLER                46
#define J1939_STD_ADDR_SUSPENSION_SYSTEM_CONTROLLER1            47
#define J1939_STD_ADDR_PNEUMATIC_SYSTEM_CONTROLLER              48
#define J1939_STD_ADDR_CAB_CONTROLLER_PRIMARY                   49
#define J1939_STD_ADDR_CAB_CONTROLLER_SECONDARY                 50
#define J1939_STD_ADDR_TIRE_PRESSURE_CONTROLLER                 51
#define J1939_STD_ADDR_IGNITION_CONTROL_MODULE1                 52
#define J1939_STD_ADDR_IGNITION_CONTROL_MODULE2                 53
#define J1939_STD_ADDR_SEAT_CONTROLS                            54
#define J1939_STD_ADDR_LIGHTING_OPERATOR_CONTROLS               55
#define J1939_STD_ADDR_REAR_AXLE_STEERING_CONTROLLER1           56
#define J1939_STD_ADDR_WATER_PUMP_CONTROLLER                    57
#define J1939_STD_ADDR_PASSENGER_OPERATOR_CLIMATE_CONTROL2      58
#define J1939_STD_ADDR_TRANSMISSION_DISPLAY_PRIMARY             59
#define J1939_STD_ADDR_TRANSMISSION_DISPLAY_SECONDARY           60
#define J1939_STD_ADDR_SUSPENSION_SYSTEM_CONTROLLER2            61
#define J1939_STD_ADDR_INFORMATION_SYSTEM_CONTROLLER1           62
#define J1939_STD_ADDR_RAMP_CONTROL                             63
// 64-127 Reserved for future assignment by SAE
// 128-247 Industry Group Specific (see Tables B3 - B7, one per industry group)
// 248 Reserved for future use
#define J1939_STD_ADDR_OFF_BOARD_DIAGNOSTIC_SERVICE_TOOL1       249
#define J1939_STD_ADDR_OFF_BOARD_DIAGNOSTIC_SERVICE_TOOL2       250
#define J1939_STD_ADDR_ON_BOARD_DATA_LOGGER                     251
// 252 Reserved for Experimental Use
// 253 Reserved for OEM
#define J1939_NULL_ADDRESS                                      254
#define J1939_GLOBAL_ADDRESS                                    255

#endif /* J1939_STD_ADDRESSES_H_ */
