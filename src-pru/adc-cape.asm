
  .cdecls "main.c"
  .clink
  .global START
  .global signalToHost
  .asg  32, PRU0_R31_VEC_VALID  ; allows notification of program completion
  .asg  3,  PRU_EVTOUT_0        ; the event number that is sent back
  
 ; Pin mappings. tX maps to PRU0_X
  .asg "r31.t1", DRDY ; Pin 29 ; Make sure to change the WBC instructions
  .asg "r30.t0", SCLK ; Pin 31  
  .asg "r31.t2", MISO ; Pin 30
  .asg "r30.t5", SYNC ; Pin 27
  .asg "r30.t7", DEBUG ; Pin 25

; Constants
  .asg "r27", C_1024
  .asg "r28", C_2048
  .asg "r29", C_3072
  
; Variables
  .asg "r12", BUFFER_OFFSET
  .asg "r13", OUTPUT_BUFFER_START
  .asg "r11", SAMPLE_OFFEST ; Index of the sample within each channel. In multiples of SAMPLE_SIZE_BYTES
  .asg "r6", BUFFER_NUMBER_ADDR
  .asg "r5", BUFFER_NUMBER

  
  .asg 24, BITS_PER_CHANNEL
  .asg 4, CHANNELS
  .asg 4, SAMPLE_SIZE_BYTES
  .asg 2048, BUFFER_SIZE_BYTES
  
  .asg 0x00000000, BUFF_LOCATION ; Hooked up to main memory
  .asg 0x00010000, BUFF_NUMBER_LOCATION ; Hooked up to main memory


read_channel .macro out_reg
   LDI32 out_reg, 0 ; Clear the input buffer. Not necessary once everything is 24 bits
   LOOP READ_CHANNEL_END?, BITS_PER_CHANNEL
   AND r7, r31, 1<<2 ; Read in/mask our bit to the right
   LSL out_reg, out_reg, 1 ; Shift
   SET r30, r30, 0 ; Clock high
   LSR r7, r7, 1 ; Shift temp reg
   OR out_reg, out_reg, r7 ; Copy into buffer
   NOP
   CLR r30, r30, 0 ; Clock low
   ; Must read one bit every 7 cycles
READ_CHANNEL_END?:
   .endm


; Using register 0 for all temporary storage (reused multiple times)
START:
   XOR r30, r30, 1<<7 ; Debug pulse
   XOR r30, r30, 1<<7 ; Debug pulse
   XOR r30, r30, 1<<7 ; Debug pulse
   ; Init pins
   SET r30, r30, 5 ; Sync is normally pulled high
   
   LDI32 OUTPUT_BUFFER_START, BUFF_LOCATION
   LDI32 SAMPLE_OFFEST, 0
   LDI32 BUFFER_NUMBER_ADDR, BUFF_NUMBER_LOCATION
   LDI32 C_1024, 1024
   LDI32 C_2048, 2048
   LDI32 C_3072, 3072
   
   LDI32 BUFFER_NUMBER, 0
   SBBO &BUFFER_NUMBER, BUFFER_NUMBER_ADDR, 0, 4

   ; Sync pulse
   CLR r30, r30, 5
   NOP
   NOP
   NOP
   NOP
   NOP
   NOP
   NOP
   NOP
   SET r30, r30, 5
   
MAINLOOP:
   WBC r31, 1 ; Wait for DRDY

   ; The ADC will always send out all four channels based on the board configuration
   XOR r30, r30, 1<<7 ; Debug pulse
   read_channel r20
   read_channel r21
   read_channel r22
   read_channel r23
   XOR r30, r30, 1<<7 ; Debug pulse

   ; There's alot of delay between samples (~6us), so this sassembly code won't be written efficiently

   AND BUFFER_OFFSET, BUFFER_NUMBER, 1 ; Get the parity
   LSL BUFFER_OFFSET, BUFFER_OFFSET, 12 ; Make it 0 or 4096
   ADD BUFFER_OFFSET, BUFFER_OFFSET, SAMPLE_OFFEST

   SBBO &r20, BUFFER_OFFSET, 0,      SAMPLE_SIZE_BYTES
   SBBO &r21, BUFFER_OFFSET, C_1024, SAMPLE_SIZE_BYTES
   SBBO &r22, BUFFER_OFFSET, C_2048, SAMPLE_SIZE_BYTES
   SBBO &r23, BUFFER_OFFSET, C_3072, SAMPLE_SIZE_BYTES

   ADD SAMPLE_OFFEST, SAMPLE_OFFEST, SAMPLE_SIZE_BYTES

   ; Did we fill either buffer?
   QBNE MAINLOOP, SAMPLE_OFFEST, C_1024

   ; Communicate the buffer to the host
   SBBO &BUFFER_NUMBER, BUFFER_NUMBER_ADDR, 0, 4
   ADD BUFFER_NUMBER, BUFFER_NUMBER, 1
   LDI32 SAMPLE_OFFEST, 0
   QBA MAINLOOP

