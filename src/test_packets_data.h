#include <stdint.h>
#include "PacketDefinition_Firehorn.h"


// Configuration of the different test packets


PacketGSE_downlink gse_down_packet = {
    .tankPressure = 101.3f,
    .tankTemperature = 25.0f,
    .fillingPressure = 100.0f,
    .status = {
        .fillingN2O = 1,
        .vent = 0
    },
    .disconnectActive = true,
    .loadcellRaw = 12345
};

av_uplink_t av_up_packet = {
    .order_id = AV_CMD_IGNITION,
    .order_value = 1
};

av_downlink_t av_down_packet = {
    .packet_nbr = 42,
    .timestamp = 123456,
    .gnss_lon = 6.1234f,
    .gnss_lat = 46.9876f,
    .gnss_alt = 500.0f,
    .gnss_vertical_speed = 5.5f,
    .N2_pressure = 50.0f,
    .fuel_pressure = 51.0f,
    .LOX_pressure = 52.0f,
    .igniter_pressure = 53.0f,
    .LOX_inj_pressure = 54.0f,
    .fuel_inj_pressure = 55.0f,
    .chamber_pressure = 56.0f,
    .fuel_level = 75.0f,
    .LOX_level = 80.0f,
    .N2_temp = 20.0f,
    .fuel_temp = 21.0f,
    .LOX_temp = 22.0f,
    .igniter_temp = 23.0f,
    .fuel_inj_temp = 24.0f,
    .fuel_inj_cool_temp = 25.0f,
    .LOX_inj_temp = 26.0f,
    .engine_temp = 75.5f,
    .engine_state = {1, 0, 1, 0, 1, 0},
    .av_state = 3
};

// Utilisée dans le main.cpp
extern PacketGSE_downlink gse_down_packet;
extern av_uplink_t av_up_packet;
extern av_downlink_t av_down_packet;