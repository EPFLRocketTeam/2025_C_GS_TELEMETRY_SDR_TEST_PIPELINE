import numpy as np
from gnuradio import gr

class byte_to_hex_list(gr.sync_block):
    """
    A Python Embedded Block that:
    - Takes a stream of bytes (uint8) as input
    - Converts them to a human-readable hex list
    - Writes the list to a text file
    """

    def __init__(self, file_path="C:/Users/nv7pr/Desktop/byte_list_output.txt"):
        # Use np.uint8 for the input to ensure correct data type interpretation
        gr.sync_block.__init__(
            self,
            name='all_communication_raw_log',
            in_sig=[np.uint8],  
            out_sig=[]
        )

        self.file_path = file_path
        self.fh = open(self.file_path, "a")

    def work(self, input_items, output_items):
        in0 = input_items[0]  # This is a numpy array of dtype uint8

        # Convert each byte to a hex string like '0x1f'
        hex_list = ["0x{:02x}".format(b) for b in in0]

        # Create a string representation of the list
        line = "[" + ",".join(hex_list) + "]\n"

        # Write to file and flush
        self.fh.write(line)
        self.fh.flush()

        return len(in0)

    def stop(self):
        # Close the file when the block stops
        if self.fh:
            self.fh.close()
        return super().stop()
