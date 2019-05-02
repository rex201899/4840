// CSEE 4840 Lab 1: Display and modify the contents of a memory
//
// Spring 2019
//
// By: 
// Lancelot Wathieu, lbw2148
// Zhongtai Ren, zr2208
//

// KEY[3] is decrement address, KEY[2] is increment address, 
// KEY[1] is decrement value, KEY[0] is increment value
// Assumptions - Each press-and-hold of the buttons counts as a single press, 
// and we ignore other buttons while one is pressed.
module lab1(input logic        CLOCK_50,
            input logic  [3:0] KEY, // Pushbuttons; KEY[0] is rightmost
            output logic [6:0] HEX0, HEX1, HEX2, HEX3, HEX4, HEX5 ); 
            // 7-segment LED displays; HEX0 is rightmost

   logic [3:0] a;         // Address
   logic [7:0] din, dout; // RAM data in and out
   logic       we;        // RAM write enable
   logic       clk;


   assign clk = CLOCK_50;

   hex7seg h0( .a(a),         .y(HEX5) ), // Leftmost digit
           h1( .a(dout[7:4]), .y(HEX3) ), // left middle
           h2( .a(dout[3:0]), .y(HEX2) ); // right middle

   controller c( .* ); // Connect everything with matching names
   memory m( .* );

   assign HEX4 = 7'b111_1111; // Display a blank; LEDs are active-low
   assign HEX1 = 7'b111_1111;
   assign HEX0 = 7'b111_1111;
  
endmodule




module controller(input logic        clk,
                  input logic  [3:0] KEY,
                  input logic  [7:0] dout,
                  output logic [3:0] a,
                  output logic [7:0] din,
                  output logic 	    we);

   /*
      - key_old[i] is used to remember last cycles value of key[i]
      - key_filtered[i] is a registered value that turns a long key[i] press 
         into a 1 for a single cycle only if key[i] is the only key 
         being pressed
      - key_is_unique checks if there is only 1 button being pressed.
      - a_updated is the next address value (input to a register)
   */

   logic [3:0] key_old, key_filtered;
   logic key_is_unique;

   logic [3:0] a_updated;

   always_ff @(posedge clk) begin
      a            <= a_updated;
      key_old      <= KEY;
      key_filtered <= ~KEY & key_old & {4{key_is_unique}};
   end


   assign key_is_unique = (KEY == 4'b0111 | KEY == 4'b1011 | 
      KEY == 4'b1101 | KEY == 4'b1110);

   assign we = key_filtered[0] | key_filtered[1];

   always_comb begin
      a_updated = a;
      if (key_filtered[2])
         a_updated = a + 1;
      else if (key_filtered[3])
         a_updated = a - 1;
   end

   always_comb begin
      din = 0;
      if (key_filtered[0])
         din = dout + 1;
      else if (key_filtered[1])
         din = dout - 1;
   end

   
endmodule





module hex7seg(input logic  [3:0] a,
               output logic [6:0] y);
   always_comb
      case (a)
         4'd0:    y = 7'b100_0000; // the "_" is ignored
         4'd1:    y = 7'b111_1001; // 1
         4'd2:    y = 7'b010_0100; // 2
         4'd3:    y = 7'b011_0000; // 3 
         4'd4:    y = 7'b001_1001;
         4'd5:    y = 7'b001_0010;
         4'd6:    y = 7'b000_0010;
         4'd7:    y = 7'b111_1000;
         4'd8:    y = 7'b000_0000;
         4'd9:    y = 7'b001_0000;
         4'd10:   y = 7'b000_1000;
         4'd11:   y = 7'b000_0011;
         4'd12:   y = 7'b100_0110;
         4'd13:   y = 7'b010_0001;
         4'd14:   y = 7'b000_0110;
         4'd15:   y = 7'b000_1110; // F
         default: y = 7'b000_0000;
      endcase
endmodule




// 16 X 8 synchronous RAM with old data read-during-write behavior
module memory( input logic         clk,
               input logic  [3:0]  a,
               input logic  [7:0]  din,
               input logic         we,
               output logic [7:0]  dout);
   
   logic [7:0]  mem [15:0];

   always_ff @(posedge clk) begin
      if (we) mem[a] <= din;
      dout <= mem[a];
   end
        
endmodule
