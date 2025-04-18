///////////////////////////////////////////////////////////////////////////////////////                                                                                                                                             
//  Packet definitions for Firehorn Project 2024-2025
//
//  Maxime Rochat (TL C-GS) & Cyprien Lacassagne (TL C-AV) & Samuel Wahba (SE C-AV C-GS) 
///////////////////////////////////////////////////////////////////////////////////////
#ifndef PACKET_FIREHORN_H
#define PACKET_FIREHORN_H

#include <stdint.h> // for uint8_t
#include <stddef.h> // for size_t
#include <stdbool.h>

///////////////////////////////////////////////////////////////////////////////////////

// This enum is used by the motherboard and the radioboard's softwares
enum CAPSULE_ID {
	//////////////////////////////////
	// Rocket & GSE
	AV_TELEMETRY = 8,
	GSE_TELEMETRY,
	GS_CMD, // uplink from GS
	//////////////////////////////////
	// Tracker
	BINOC_ATTITUDE,
	BINOC_POSITION,
	BINOC_STATUS,
	BINOC_GLOBAL_STATUS,
	//////////////////////////////////
	TRACKER_CMD,
	//////////////////////////////////
	CALIBRATE_TELEMETRY
};

enum CMD_ID {
	AV_CMD_CALIBRATE = 3,
	AV_CMD_RECOVER,
	AV_CMD_ARM,
	AV_CMD_IGNITION,
	AV_CMD_ABORT,
	AV_CMD_MANUAL_DEPLOY,
	AV_CMD_IGNITER_LOX,
	AV_CMD_IGNITER_FUEL,
	AV_CMD_MAIN_LOX,
	AV_CMD_MAIN_FUEL,
	AV_CMD_VENT_LOX,
	AV_CMD_VENT_FUEL,
	AV_CMD_PRESSURIZE,
	/* GSE commands left untouched, just replaced N20 with LOX */
	GSE_CMD_FILLING_LOX,
	GSE_CMD_VENT,
	GSE_CMD_DISCONNECT
};


/////////////////////////////////////////////////////////////////
// Here is a template for writing new packet structures 
typedef struct __attribute__((__packed__)) {
	uint8_t data1;
	uint8_t data2;
	uint16_t data3;
} PacketTemplate;
#ifdef __cplusplus
const uint32_t packetTemplateSize = sizeof(PacketTemplate);
#endif

/////////////////////////////////////////////////////////////////
// ---------------------- AV PACKETS ------------------------  // 
/////////////////////////////////////////////////////////////////

typedef struct __attribute__((__packed__)) {
	uint8_t igniter_LOX;    // V3
	uint8_t igniter_fuel;   // V4
	uint8_t main_LOX;       // V5
	uint8_t main_fuel;      // V6
	uint8_t vent_LOX;       // V1
	uint8_t vent_fuel;      // V2
	/* Commented out because didn't know what to do with it
	uint8_t pressurize;
	uint8_t purge;
	uint8_t reserve;
	*/
} engine_state_t;
#ifdef __cplusplus
const uint32_t engine_state_size = sizeof(engine_state_t);
#endif

// AV UPLINK PACKET
typedef struct __attribute__((__packed__)) {
	uint8_t order_id;    // from CMD_ID
	uint8_t order_value; // only ACTIVE or INACTIVE  	254 other possibilities unconsidered
} av_uplink_t;
#ifdef __cplusplus
const size_t av_uplink_size = sizeof(av_uplink_t);
#endif

// AV DOWNLINK PACKET
typedef struct __attribute__((__packed__)) {
	uint32_t packet_nbr;
	uint32_t timestamp;
	float	 gnss_lon;   // dd.dddddd
	float	 gnss_lat;   // dd.dddddd
	float	 gnss_alt;   // m
	float 	 gnss_vertical_speed; // m/s
	/* Engine sensors */
	float    N2_pressure;       // P1
	float    fuel_pressure;     // P2
	float    LOX_pressure;      // P3
	float    igniter_pressure;  // P4
	float    LOX_inj_pressure;  // P5
	float    fuel_inj_pressure; // P6
	float    chamber_pressure;  // P7
	float    fuel_level;        // L1
	float    LOX_level;         // L2
	float	 N2_temp;	    // T1
	float 	 fuel_temp;	    // T2
	float  	 LOX_temp; 	    // T3
	float 	 igniter_temp;	    // T4
	float 	 fuel_inj_temp;     // T5
	float    fuel_inj_cool_temp; // T6
	float 	 LOX_inj_temp; 	    // T7
	float    engine_temp;       // T8
	engine_state_t engine_state; // binary states of the valves
	uint8_t  av_state; // flightmode
} av_downlink_t;
#ifdef __cplusplus
const uint32_t av_downlink_size = sizeof(av_downlink_t);
#endif

/////////////////////////////////////////////////////////////////
// ---------------------- GSE PACKETS ---------------------- // 

typedef struct __attribute__((__packed__)) {
	uint8_t fillingN2O;
	uint8_t vent;
} GSE_cmd_status;
#ifdef __cplusplus
const uint32_t GSE_cmd_status_size = sizeof(GSE_cmd_status);
#endif

typedef struct __attribute__((__packed__)) {
	float tankPressure;
	float tankTemperature;
	float fillingPressure;
    GSE_cmd_status status;
	bool disconnectActive;
	int32_t loadcellRaw;
} PacketGSE_downlink;
#ifdef __cplusplus
const uint32_t packetGSE_downlink_size = sizeof(PacketGSE_downlink);
#endif

/*
/////////////////////////////////////////////////////////////////
// ---------------------- BINOC PACKETS ---------------------- // 

typedef struct __attribute__((__packed__)) {
	float azm;
	float elv;
} PacketBinocAttitude;
#ifdef __cplusplus
const uint32_t packetBinocAttitudeSize = sizeof(PacketBinocAttitude);
#endif

typedef struct __attribute__((__packed__)) {
	float lon;
	float lat;
	float alt;
} PacketBinocPosition;
#ifdef __cplusplus
const uint32_t packetBinocPositionSize = sizeof(PacketBinocPosition);
#endif

typedef struct __attribute__((__packed__)) {
	bool isInView;
	bool isCalibrated;
} PacketBinocStatus;
#ifdef __cplusplus
const uint32_t packetBinocStatusSize = sizeof(PacketBinocStatus);
#endif

typedef struct __attribute__((__packed__)) {
	PacketBinocAttitude attitude;
    PacketBinocPosition position;
    PacketBinocStatus status;
} PacketBinocGlobalStatus;
#ifdef __cplusplus
const uint32_t packetBinocGlobalStatusSize = sizeof(PacketBinocGlobalStatus);
#endif

/////////////////////////////////////////////////////////////////
// ---------------------- TRACKER PACKETS ---------------------- // 

typedef struct __attribute__((__packed__)) {
	float azm;
	float elv;
	int mode;
	float cutoffFreq;
	unsigned maxTimeWindow;
	unsigned timeStamp;
} PacketTrackerCmd;
#ifdef __cplusplus
const uint32_t packetTrackerCmdSize = sizeof(PacketTrackerCmd);
#endif
*/

#endif /* PACKET_FIREHORN_H */
