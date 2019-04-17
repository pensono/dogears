
  .cdecls "main.c"
  .clink
  .global START
  .asg  32, PRU0_R31_VEC_VALID  ; allows notification of program completion
  .asg  3,  PRU_EVTOUT_0        ; the event number that is sent back
  
 ; Pin mappings. tX maps to PRU0_X
  .asg "r31.t1", DRDY ; Pin 29 ; Make sure to change the WBC instructions
  .asg "r30.t0", SCLK ; Pin 31  
  .asg "r31.t2", MISO ; Pin 30
  .asg "r30.t5", SYNC ; Pin 27
  .asg "r30.t7", DEBUG ; Pin 25
  
; Variables
  .asg "r10", INPUT_BUFFER
  .asg "r12", CHANNELS_LEFT
  .asg "r15", OUTPUT_BUFFER_START
  .asg "r11", OUTPUT_BUFFER_INDEX
  
; Constants (must be in a register because the values are > 255)
  .asg "r9", BUFFER_SIZE_BYTES_REG
  .asg "r8", BUFFER_SIZE_BYTES_2_REG

; Registers r20-r29 are for temporary use
  
  .asg 24, BITS_PER_CHANNEL
  .asg 4, CHANNELS
  .asg 4, SAMPLE_SIZE_BYTES
  .asg 1024, BUFFER_SIZE_BYTES
  
  .asg 0x00000000, BUFF_LOCATION ; Hooked up to main memory
  
  .asg  1000, TRIGGER_COUNT     ;
  .asg  100000, SAMPLE_DELAY_1MS

; Using register 0 for all temporary storage (reused multiple times)
START:
   ; Init pins
   SET r30, r30, 5 ; Sync is normally pulled high
   
   LDI32 OUTPUT_BUFFER_INDEX, 0
   LDI32 OUTPUT_BUFFER_START, BUFF_LOCATION
   LDI32 BUFFER_SIZE_BYTES_REG, BUFFER_SIZE_BYTES
   LDI32 BUFFER_SIZE_BYTES_2_REG, BUFFER_SIZE_BYTES * 2

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
   LDI32 CHANNELS_LEFT, 4
   WBC r31, 1 ; Wait for DRDY
   XOR r30, r30, 1<<7 ; Debug pulse
   
READ_CHANNEL_START:
   LOOP READ_CHANNEL_END, BITS_PER_CHANNEL
   AND r22, r31, 1<<2 ; Read in/mask our bit to the right
   LSL INPUT_BUFFER, INPUT_BUFFER, 1 ; Shift
   SET r30, r30, 0 ; Clock high
   LSR r22, r22, 2 ; Shift temp reg
   OR INPUT_BUFFER, INPUT_BUFFER, r22 ; Copy into buffer
   NOP
   CLR r30, r30, 0 ; Clock low
   ; Must read one bit every 7 cycles
READ_CHANNEL_END:
   SBBO &INPUT_BUFFER, OUTPUT_BUFFER_START, OUTPUT_BUFFER_INDEX, SAMPLE_SIZE_BYTES
   ADD OUTPUT_BUFFER_INDEX, OUTPUT_BUFFER_INDEX, SAMPLE_SIZE_BYTES
   LDI32 INPUT_BUFFER, 0 ; Clear the input buffer. Not necessary once everything is 24 bits
   SUB CHANNELS_LEFT, CHANNELS_LEFT, 1
   QBNE READ_CHANNEL_START, CHANNELS_LEFT, 0

READ_END:
   XOR r30, r30, 1<<7 ; Debug pulse
   
   ; There's alot of delay between samples (~6us), so thi sassembly code won't be written efficiently
   
   ; Did we fill the buffer?
   QBEQ PUBLISH_BUF0, OUTPUT_BUFFER_INDEX, BUFFER_SIZE_BYTES_REG
   QBEQ PUBLISH_BUF1, OUTPUT_BUFFER_INDEX, BUFFER_SIZE_BYTES_2_REG
   QBA MAINLOOP

PUBLISH_BUF0:
   ;LDI32  r31, (PRU0_R31_VEC_VALID|PRU_EVTOUT_0)
   QBA MAINLOOP

   
PUBLISH_BUF1:
   ;LDI32  r31, (PRU0_R31_VEC_VALID|PRU_EVTOUT_0)
   LDI32  OUTPUT_BUFFER_INDEX, 0
   QBA MAINLOOP

