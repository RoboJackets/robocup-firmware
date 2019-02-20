// Current Sensor Simulator

`include "SPI_Slave.v"

// SPI Slave module
// Change SPI slave length of DATA_OUT and DATA_IN

module CS_sim(sysclk, spi_slave_do, spi_slave_di, spi_slave_byte_done);

localparam FPGA_CMD_LEN = 16;
localparam NUM_OF_SENSORS = 8;

input sysclk;
input [FPGA_CMD_LEN - 1:0] spi_slave_do;
input spi_slave_byte_done;
output reg [FPGA_CMD_LEN - 1:0] spi_slave_di;


SPI_Slave #(.DATA_BIT_WIDTH(16)) spi_slave_module (
    .clk            ( sysclk                ) ,
    .SCK            ( spi_slave_sck         ) ,
    .MOSI           ( spi_slave_mosi_s      ) ,
    .MISO           ( spi_slave_miso_o      ) ,
    .SSEL           ( spi_slave_ncs_s       ) ,
    .DONE           ( spi_slave_byte_done   ) ,
    .DATA_IN        ( spi_slave_do          ) ,
    .DATA_OUT       ( spi_slave_di          )
);




// Registers:

reg [FPGA_CMD_LEN - 1:0] FPGA_CMD;                          // DATA_OUT from the SPI_SLAVE
reg REPEAT;                                                 // It doesn't stop reading the sensors, until further command
reg [FPGA_CMD_LEN - 1:0] FAKE_VALUES [0:NUM_OF_SENSORS - 1];// FAKE data from the sensor
reg FAKE_VALUES_FLAG [0:NUM_OF_SENSORS - 1];                // It indicates which sensors can send the data

reg [1:0] TWO_PERIODS = 0;                                  // The sensor does 2 periods of sending invalid data frames after every call

reg ADC_DONE = 0;                                           // This tells when the ADC is done with its actions
// Creating fake values
initial begin
    FAKE_VALUES [0] <= 16'h0001;
    FAKE_VALUES [1] <= 16'h1002;
    FAKE_VALUES [2] <= 16'h2003;
    FAKE_VALUES [3] <= 16'h3004;
    FAKE_VALUES [4] <= 16'h4005;
    FAKE_VALUES [5] <= 16'h5006;
    FAKE_VALUES [6] <= 16'h6007;
    FAKE_VALUES [7] <= 16'h7008;
end

integer i;
always @(negedge sysclk or negedge sysclk) begin
    if(spi_slave_byte_done == 1 && ADC_DONE == 0) begin

        FPGA_CMD = spi_slave_do;

        // Updating the chip with the FPGA command
        if(FPGA_CMD[15] == 1) begin
            // To update the sensor, the first bit must be 1
            if (FPGA_CMD[14] == 0) begin
                REPEAT <= 0;
            end else begin
                REPEAT <= 1;
            end
            // From the 13th bit to the 6th bit of the FPGA command we set flags to determine if we are going to send data from each sensor
            for (i = 0;i < NUM_OF_SENSORS;i = i + 1) begin
                FAKE_VALUES_FLAG[i] <= FPGA_CMD[13 - i];
            end
            TWO_PERIODS = 2'b01;
        end


        if (TWO_PERIODS == 2'b11) begin
            for(i = 0;i <= NUM_OF_SENSORS; i = i + 1) begin
                if (FAKE_VALUES_FLAG[i] == 1) begin
                    spi_slave_di <= FAKE_VALUES[i];
                    FAKE_VALUES_FLAG[i] <= 0;
                    i <= i + NUM_OF_SENSORS + 1;
                end

                if (i == NUM_OF_SENSORS) begin
                    TWO_PERIODS <= 0;
                end
            end
        end

        if (TWO_PERIODS == 2'b01) begin
            TWO_PERIODS[1] = 1;
        end

        ADC_DONE = 1; // We set a flag on to say that the ADC has sent everything to the SPI_MODULE
    end
end




// We set the done to stop being done during the period in which the ADC is sending data to the FPGA
always @(posedge sysclk or negedge sysclk) begin
    if(ADC_DONE == 1 && spi_slave_byte_done == 0) begin
        ADC_DONE <= 0;
    end
end


endmodule