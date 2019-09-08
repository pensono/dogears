
;  .cdecls "main.c"
  .clink
  .global START
  .global signalToHost
  .asg  32, PRU0_R31_VEC_VALID  ; allows notification of program completion
  .asg  3,  PRU_EVTOUT_0        ; the event number that is sent back
  
 ; Pin mappings. tX maps to PRU0_X
  .asg "r31.t14", DRDY ; Pin P8_16
  .asg 14, DRDY_BIT
  .asg "r30.t0", SCLK ; Pin P9_31
  .asg 0, SCLK_BIT
  .asg "r31.t2", MISO ; Pin P9_30
  .asg 2, MISO_BIT
  .asg "r30.t14", SYNC ; Pin P8_12
  .asg 14, SYNC_BIT
  .asg "r30.t5", DBUG ; Pin P9_27
  .asg 5, DBUG_BIT

; Constants
  .asg "r25", INTERRUPT_SIGNAL
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
   LOOP READ_CHANNEL_END?, BITS_PER_CHANNEL
   LSL out_reg, out_reg, 1 ; Shift
   NOP
   NOP
   NOP
   NOP
   NOP ; Bonus
   SET r30, r30, SCLK_BIT ; Clock high
   AND r7, r31, 1<<MISO_BIT ; Read in/mask our bit to the right
   LSR r7, r7, MISO_BIT ; Shift temp reg back
   OR out_reg, out_reg, r7 ; Copy into buffer
   NOP
   NOP
   NOP
   CLR r30, r30, SCLK_BIT ; Clock low
   ; Must read one bit every 7 cycles
READ_CHANNEL_END?:
   .endm

START:
   ; Init pins
   SET r30, r30, SYNC_BIT ; Sync is normally pulled high

   ; Init constants
   LDI32 OUTPUT_BUFFER_START, BUFF_LOCATION
   LDI32 INTERRUPT_SIGNAL, PRU0_R31_VEC_VALID | PRU_EVTOUT_0
   LDI32 BUFFER_NUMBER_ADDR, BUFF_NUMBER_LOCATION
   LDI32 C_1024, 1024
   LDI32 C_2048, 2048
   LDI32 C_3072, 3072

   ; Init state
   LDI32 SAMPLE_OFFEST, 0
   
   LDI32 BUFFER_NUMBER, 137
   SBBO &BUFFER_NUMBER, BUFFER_NUMBER_ADDR, 0, 4

   ; Sync pulse
   CLR r30, r30, SYNC_BIT
   NOP
   NOP
   NOP
   NOP
   NOP
   NOP
   NOP
   NOP
   SET r30, r30, SYNC_BIT
   
MAINLOOP:
   WBC r31, DRDY_BIT ; Wait for /DRDY

   ; The ADC will always send out all four channels based on the board configuration
   read_channel r20
   read_channel r21
   read_channel r22
   read_channel r23
   XOR r30, r30, 1 << DBUG_BIT ; Debug pulse

   ; There's alot of delay between samples (~6us), so this sassembly code won't be written efficiently

   AND BUFFER_OFFSET, BUFFER_NUMBER, 1 ; Get the parity
   LSL BUFFER_OFFSET, BUFFER_OFFSET, 12 ; Make it 0 or 4096
   ADD BUFFER_OFFSET, BUFFER_OFFSET, SAMPLE_OFFEST

   SBBO &r20, BUFFER_OFFSET, 0,      SAMPLE_SIZE_BYTES
   SBBO &r21, BUFFER_OFFSET, C_1024, SAMPLE_SIZE_BYTES
   SBBO &r22, BUFFER_OFFSET, C_2048, SAMPLE_SIZE_BYTES
   SBBO &r23, BUFFER_OFFSET, C_3072, SAMPLE_SIZE_BYTES

   LDI32 r20, 0 ; Clear the input buffer. Not necessary once everything is 24 bits
   LDI32 r21, 0
   LDI32 r22, 0
   LDI32 r23, 0

   ADD SAMPLE_OFFEST, SAMPLE_OFFEST, SAMPLE_SIZE_BYTES

   ; Did we fill either buffer?
   QBNE MAINLOOP, SAMPLE_OFFEST, C_1024

   ; Communicate the buffer to the host
   SBBO &BUFFER_NUMBER, BUFFER_NUMBER_ADDR, 0, 4
   MOV R31.b0, INTERRUPT_SIGNAL
   ADD BUFFER_NUMBER, BUFFER_NUMBER, 1
   LDI32 SAMPLE_OFFEST, 0
   QBA MAINLOOP

