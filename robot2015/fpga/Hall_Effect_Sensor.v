/*
This module reads the inputs from the hall effect sensors on bus 'h' and
outputs buses 'u' for which phase to drive high and 'z' for which phase will be 
high impedance (disconected from Vdd and gnd), the third non-specified phase 
where, h[n] and z[n] are both zero, is driven low. h[2] is hall 1, h[1] is 
hall 2, etc. u[2] and z[2] correspond to motor phase 1 or A, u[1] and z[1] 
correspond to motor phase 2 or B, etc. 

With only three bits in input bus h there are only eight posible states for the 
hall sensors, two of which indcate an error: '000' and '111'. If these states 
are ever encountere, all three motor phasese signaled to go into the high 
impedance (disconnected) state. Otherwise, the high phase, low phase, and high 
impedance phase for each hall state is specified in the document linked below.

//http://www.maxonmotor.com/medias/sys_master/root/8815461662750/EC-Technology-short-and-to-the-point-14-EN-32-35.pdf?attachment=true
--Doho*/

module Hall_Effect_Sensor(clock, h, u, z);
input clock;
input	[2:0]	h; 	//Hall Effect sensor input
output 	[2:0]	u; 	//High phase output
output	[2:0]	z;	//High Impedance output

reg	[2:0]	u = 3'b000;
reg	[2:0]	z = 3'b111;


always @(posedge clock)
		case(h)
		3'b000	: 	begin
					u = 3'b000;
					z = 3'b111;
					end

		3'b111	: 	begin
					u = 3'b000;
					z = 3'b111;
					end

		3'b101	:	begin
					u = 3'b100;
					z = 3'b001;
					end

		3'b100	:	begin
					u = 3'b100;
					z = 3'b010;
					end

		3'b110	:	begin
					u = 3'b010;
					z = 3'b100;
					end

		3'b010	:	begin
					u = 3'b010;
					z = 3'b001;
					end

		3'b011	:	begin
					u = 3'b001;
					z = 3'b010;
					end

		3'b001	:	begin
					u = 3'b001;
					z = 3'b100;
					end
		endcase

endmodule


