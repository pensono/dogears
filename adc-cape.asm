
  .cdecls "main.c"
  .clink
  .global START
  .asg  32, PRU0_R31_VEC_VALID  ; allows notification of program completion
  .asg  3,  PRU_EVTOUT_0        ; the event number that is sent back
  
 ; Pin mappings. tX maps to PRU0_X
  .asg "r31.t1", DRDY ; Pin 29 ; Make sure to change the WBC instructions
  .asg "r30.t0", SCLK ; Pin 31  
  .asg "r31.t2", MISO ; Pin 30
  .asg "r31.t5", SYNC ; Pin 27
  .asg "r30.t7", DEBUG ; Pin 25
  
; Variables
  .asg "r10", INPUT_BUFFER
  .asg "r15", OUTPUT_BUFFER_START
  .asg "r11", OUTPUT_BUFFER_INDEX

; Registers r20-r29 are for temporary use
  
  .asg 24, BITS_PER_CHANNEL
  .asg 4, CHANNELS
  .asg 4, SAMPLE_SIZE_BYTES
  
  .asg 0x00000000, BUFF_LOCATION ; Hooked up to main memory
  
  .asg  1000, TRIGGER_COUNT     ;
  .asg  100000, SAMPLE_DELAY_1MS

; Using register 0 for all temporary storage (reused multiple times)
START:
   ; Init pins
   SET r30, r30, 5 ; Sync is normally pulled high
   
   ; Read number of samples to read and inter-sample delay
   LDI32  r0, 0x00000000        ; load the memory location, number of samples
   LBBO   &r1, r0, 0, 4         ; load the value into memory - keep r1
   ; Read the sample delay
   LDI32  r0, 0x00000004        ; the sample delay is in the second 32-bits
   LBBO   &r2, r0, 0, 4         ; the sample delay is stored in r2
   LDI r10, 0; Zero out the input buffer register
   LDI32 OUTPUT_BUFFER_START, BUFF_LOCATION

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
   LDI32 OUTPUT_BUFFER_INDEX, 0
   WBC r31, 1 ; Wait for DRDY
   
READ_CHANNEL_START:
   LOOP READ_CHANNEL_END, BITS_PER_CHANNEL
   CLR r30, r30, 0 ; Clock low
   NOP; AND r22, r31, 1 ; Read in/mask our bit to the right
   NOP; OR INPUT_BUFFER, INPUT_BUFFER, r22 ; Copy into buffer
   ;SET r30, r30, 0 ; Clock high
   NOP ; LSL INPUT_BUFFER, INPUT_BUFFER, 1 ; Shift
   NOP
   XOR r30, r30, 1<<7 ; Debug pulse
   ; Must read one bit every 7 cycles
READ_CHANNEL_END:
   SBBO &INPUT_BUFFER, OUTPUT_BUFFER_START, OUTPUT_BUFFER_INDEX, SAMPLE_SIZE_BYTES
   ADD OUTPUT_BUFFER_INDEX, OUTPUT_BUFFER_INDEX, SAMPLE_SIZE_BYTES
   QBNE READ_CHANNEL_START, OUTPUT_BUFFER_INDEX, SAMPLE_SIZE_BYTES * CHANNELS

READ_END:
   QBA MAINLOOP

END:                               ; end of program, send back interrupt
   LDI32  R31, (PRU0_R31_VEC_VALID|PRU_EVTOUT_0)
   HALT                       ; halt the pru program


MEMTEST:
   LDI32 INPUT_BUFFER, 0
   LDI32 OUTPUT_BUFFER_INDEX, 0
   LOOP MEMTEST_END, 8
   ADD INPUT_BUFFER, INPUT_BUFFER, 1
   SET r30, r30, 7
   CLR r30, r30, 7
   ADD OUTPUT_BUFFER_INDEX, OUTPUT_BUFFER_INDEX, 4
MEMTEST_END:
   QBA MEMTEST