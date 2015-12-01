module lab3midi(data, clk, reset, LED0, LED1, LED2, LED3, LED4, LED5, LED6);

input data, clk, reset;
output reg LED0, LED1, LED2, LED3, LED4, LED5, LED6;

reg start;
reg[13:0] counter;

initial begin
	LED0 <= 0;
    LED1 <= 0;
    LED2 <= 0;
    LED3 <= 0;
    LED4 <= 0;
    LED5 <= 0;
    LED6 <= 0;
    counter <= 0;
    start <= 0;
end

always @ (posedge clk) begin
  if (reset == 0) begin // Asynchronus reset on negedge
    start = 1'b0;
    counter = 0;
    LED0 <= 0;
    LED1 <= 0;
    LED2 <= 0;
    LED3 <= 0;
    LED4 <= 0;
    LED5 <= 0;
    LED6 <= 0;
  end else begin
	if (start == 1'b0 && data == 1'b0) begin // check START OF BYTE
		start = 1'b1;
	end 
    else if (start == 1'b1) begin
	  counter = counter + 1;
	  case(counter)
		1216: start = 1'b0;	// Pseudo state change from STATUS to NOTE
		1408: LED0 <= data;	// Denotes start of NOTE, 1.5 BT + 9.5 BT
		1536: LED1 <= data;
		1664: LED2 <= data;
		1792: LED3 <= data;
		1920: LED4 <= data;
		2048: LED5 <= data;
		2176: LED6 <= data;
		2432: start = 1'b0; // Pseudo state change from NOTE to VELOCITY
		3648: begin		// Reset of states after VELOCITY byte read
			    start = 1'b0;
			    counter = 0;
			  end
	  endcase
    end
  end
end

endmodule