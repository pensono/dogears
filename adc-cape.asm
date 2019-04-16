
  .cdecls "main.c"
  .clink
  .global START
  .asg  32, PRU0_R31_VEC_VALID  ; allows notification of program completion
  .asg  3,  PRU_EVTOUT_0        ; the event number that is sent back
  
 ; Pin mappings. tX maps to PRU0_X
  .asg "r31.t1", DRDY ; Pin 29 ; Make sure to change the WBC instructions
  .asg "r30.t0", SCLK ; Pin 31  
  .asg "r31.t2", MISO ; Pin 30
  
; Variables
  .asg "r12", CHANNELS_REMAINING
  .asg "r10", INPUT_BUFFER
  .asg "r15", OUTPUT_BUFFER_START
  .asg "r11", OUTPUT_BUFFER_INDEX
  
  .asg 24, BITS_PER_CHANNEL
  .asg 4, CHANNELS
  
  .asg 0x00000000, BUFF_LOCATION ; Hooked up to main memory
  
  .asg  1000, TRIGGER_COUNT     ;
  .asg  100000, SAMPLE_DELAY_1MS

; Using register 0 for all temporary storage (reused multiple times)
START:
   ; Read number of samples to read and inter-sample delay
   LDI32  r0, 0x00000000        ; load the memory location, number of samples
   LBBO   &r1, r0, 0, 4         ; load the value into memory - keep r1
   ; Read the sample delay
   LDI32  r0, 0x00000004        ; the sample delay is in the second 32-bits
   LBBO   &r2, r0, 0, 4         ; the sample delay is stored in r2
   LDI r10, 0; Zero out the input buffer register
   LDI32 OUTPUT_BUFFER_START, BUFF_LOCATION
   QBA MEMTEST

MAINLOOP:
   LDI32 OUTPUT_BUFFER_INDEX, 0
   WBC r31, 1 ; Wait for DRDY
   
   LOOP READ_END, CHANNELS
   LOOP READ_CHANNEL_END, BITS_PER_CHANNEL
   AND r22, r31, 1 ; Read in/mask our bit to the right
   ADD INPUT_BUFFER, INPUT_BUFFER, 1 ; OR INPUT_BUFFER, INPUT_BUFFER, r22 ; Copy into buffer
   SET r30, r30, 0 ; Clock high
   NOP;LSL INPUT_BUFFER, INPUT_BUFFER, 1 ; Shift
   NOP
   NOP
   CLR r30, r30, 0 ; Clock low
READ_CHANNEL_END:
   SBBO &INPUT_BUFFER, OUTPUT_BUFFER_START, OUTPUT_BUFFER_INDEX, 4
   ADD OUTPUT_BUFFER_INDEX, OUTPUT_BUFFER_INDEX, 4

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
   SBBO &INPUT_BUFFER, OUTPUT_BUFFER_START, OUTPUT_BUFFER_INDEX, 4
   ADD OUTPUT_BUFFER_INDEX, OUTPUT_BUFFER_INDEX, 4
MEMTEST_END:
   HALT