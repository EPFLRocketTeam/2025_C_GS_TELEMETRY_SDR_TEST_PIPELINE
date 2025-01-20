import numpy as np
from gnuradio import gr
import struct

##############################################################################
# Parser functions for each packet type
##############################################################################

def parse_gse_downlink(payload: bytes):
    """
    GSE Downlink packet.
    struct PacketGSE_downlink {
        float tankPressure;      // 4 bytes
        float tankTemperature;   // 4 bytes
        float fillingPressure;   // 4 bytes
        GSE_cmd_status status;   // 2 bytes (fillingN2O + vent)
        bool disconnectActive;   // 1 byte
        int32_t loadcellRaw;     // 4 bytes
    } => total 19 bytes (packed)
    """
    fmt = '<fffBBBi'  # 3 floats, 2 bytes, 1 byte (bool), 1 int32
    try:
        (
            tankPressure,
            tankTemperature,
            fillingPressure,
            fillingN2O,
            vent,
            disconnectActive,
            loadcellRaw
        ) = struct.unpack(fmt, payload)

        return {
            'tankPressure':       tankPressure,
            'tankTemperature':    tankTemperature,
            'fillingPressure':    fillingPressure,
            'fillingN2O':         fillingN2O,
            'vent':               vent,
            'disconnectActive':   bool(disconnectActive),
            'loadcellRaw':        loadcellRaw
        }
    except struct.error as e:
        print(f"Error unpacking GSE_downlink: {e}")
        return None


def parse_av_uplink(payload: bytes):
    """
    AV Uplink packet (e.g., GS_CMD).
    struct av_uplink_t {
        uint8_t order_id;    
        uint8_t order_value; 
    } => total 2 bytes
    """
    fmt = '<BB'
    try:
        (order_id, order_value) = struct.unpack(fmt, payload)
        return {
            'order_id':    order_id,
            'order_value': order_value
        }
    except struct.error as e:
        print(f"Error unpacking av_uplink: {e}")
        return None


def parse_av_downlink(payload: bytes):
    """
    AV Downlink packet (CAPSULE_ID = AV_TELEMETRY).
    (See PacketDefinition_Firehorn.h for completeness, size = 99 bytes)
    """
    fmt = '<II21f6BB'
    try:
        fields = struct.unpack(fmt, payload)
        # Break them out for clarity
        packet_nbr         = fields[0]
        timestamp          = fields[1]
        gnss_lon           = fields[2]
        gnss_lat           = fields[3]
        gnss_alt           = fields[4]
        gnss_vertical_speed= fields[5]
        N2_pressure        = fields[6]
        fuel_pressure      = fields[7]
        LOX_pressure       = fields[8]
        igniter_pressure   = fields[9]
        LOX_inj_pressure   = fields[10]
        fuel_inj_pressure  = fields[11]
        chamber_pressure   = fields[12]
        fuel_level         = fields[13]
        LOX_level          = fields[14]
        N2_temp            = fields[15]
        fuel_temp          = fields[16]
        LOX_temp           = fields[17]
        igniter_temp       = fields[18]
        fuel_inj_temp      = fields[19]
        fuel_inj_cool_temp = fields[20]
        LOX_inj_temp       = fields[21]
        engine_temp        = fields[22]
        igniter_LOX        = fields[23]
        igniter_fuel       = fields[24]
        main_LOX           = fields[25]
        main_fuel          = fields[26]
        vent_LOX           = fields[27]
        vent_fuel          = fields[28]
        av_state           = fields[29]

        return {
            'packet_nbr':          packet_nbr,
            'timestamp':           timestamp,
            'gnss_lon':            gnss_lon,
            'gnss_lat':            gnss_lat,
            'gnss_alt':            gnss_alt,
            'gnss_vertical_speed': gnss_vertical_speed,
            'N2_pressure':         N2_pressure,
            'fuel_pressure':       fuel_pressure,
            'LOX_pressure':        LOX_pressure,
            'igniter_pressure':    igniter_pressure,
            'LOX_inj_pressure':    LOX_inj_pressure,
            'fuel_inj_pressure':   fuel_inj_pressure,
            'chamber_pressure':    chamber_pressure,
            'fuel_level':          fuel_level,
            'LOX_level':           LOX_level,
            'N2_temp':             N2_temp,
            'fuel_temp':           fuel_temp,
            'LOX_temp':            LOX_temp,
            'igniter_temp':        igniter_temp,
            'fuel_inj_temp':       fuel_inj_temp,
            'fuel_inj_cool_temp':  fuel_inj_cool_temp,
            'LOX_inj_temp':        LOX_inj_temp,
            'engine_temp':         engine_temp,
            'engine_state': {
                'igniter_LOX':  igniter_LOX,
                'igniter_fuel': igniter_fuel,
                'main_LOX':     main_LOX,
                'main_fuel':    main_fuel,
                'vent_LOX':     vent_LOX,
                'vent_fuel':    vent_fuel
            },
            'av_state': av_state
        }

    except struct.error as e:
        print(f"Error unpacking av_downlink: {e}")
        return None


##############################################################################
# A lookup table that maps packet IDs (from the transmitter) to:
#   (human-readable name, parser function, expected_payload_size)
##############################################################################

capsule_info = {
    # ID 0x01 => GSE Downlink (19 bytes)
    0x01: ("GSE_TELEMETRY", parse_gse_downlink, 19),

    # ID 0x0A => AV Uplink (2 bytes)
    0x0A: ("GS_CMD", parse_av_uplink, 2),

    # AV Downlink with ID 0x08:
    0x08: ("AV_TELEMETRY", parse_av_downlink, 99),
}


##############################################################################
# Main GNURadio block
##############################################################################

class gse_downlink_decoder(gr.sync_block):
    """
    Example GNURadio block that detects Firehorn packets and
    decodes them based on the PacketDefinition.h structures.
    """

    def __init__(self):
        gr.sync_block.__init__(
            self,
            name='Firehorn Packet Decoder',
            in_sig=[np.uint8],
            out_sig=[]
        )

        # State machine variables
        self.state = 'PREAMBLE_A'
        self.buffer = []
        self.packet_id = None
        self.length = None
        self.bytes_received = 0

    def work(self, input_items, output_items):
        in0 = input_items[0]

        # Debug print
        print(f"Received Bytes: {' '.join(f'{byte:02x}' for byte in in0)}")

        for byte in in0:
            if self.state == 'PREAMBLE_A':
                # Look for first preamble byte (0xFF)
                if byte == 0xFF:
                    self.state = 'PREAMBLE_B'

            elif self.state == 'PREAMBLE_B':
                # Look for second preamble byte (0xFA)
                if byte == 0xFA:
                    self.state = 'PACKET_ID'
                    self.buffer.clear()
                    self.bytes_received = 0
                else:
                    # If it's not 0xFA, go back to searching for 0xFF
                    self.state = 'PREAMBLE_A'

            elif self.state == 'PACKET_ID':
                self.packet_id = byte
                print(f"Packet ID: 0x{self.packet_id:02X}")
                self.state = 'LENGTH'

            elif self.state == 'LENGTH':
                self.length = byte
                print(f"Received Length Byte: 0x{self.length:02X}")
                self.state = 'PAYLOAD'
                self.buffer.clear()
                self.bytes_received = 0

            elif self.state == 'PAYLOAD':
                self.buffer.append(byte)
                self.bytes_received += 1
                if self.bytes_received == self.length:
                    # We've received the full payload, next is CRC
                    print(f"Full Payload Received (Raw): {' '.join(f'{b:02x}' for b in self.buffer)}")
                    self.state = 'CRC'

            elif self.state == 'CRC':
                # Compute our local checksum
                calculated_checksum = sum(self.buffer) & 0xFF
                print(f"Calculated Checksum: 0x{calculated_checksum:02X}, Received Checksum: 0x{byte:02X}")

                if calculated_checksum == byte:
                    # Packet ID recognized?
                    if self.packet_id in capsule_info:
                        (name, parser_fn, expected_size) = capsule_info[self.packet_id]

                        # Check if the payload size matches what we expect
                        if len(self.buffer) == expected_size:
                            print(f"Detected packet type: {name}")
                            parsed_data = parser_fn(bytes(self.buffer))
                            if parsed_data is not None:
                                print(f"Decoded {name} Packet: {parsed_data}")
                            else:
                                print(f"Failed to parse {name} data.")
                        else:
                            print(f"Unexpected payload size: {len(self.buffer)}. Expected: {expected_size}")
                    else:
                        # If no matching parser is found, skip
                        print(f"Unknown packet ID: 0x{self.packet_id:02X}. No parser available.")
                else:
                    print("Checksum failed. Packet discarded.")

                # In any case, reset state for next packet
                self.state = 'PREAMBLE_A'

        return len(in0)
