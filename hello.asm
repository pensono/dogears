  .cdecls "main.c"
  .clink
  .global START
  .asg  32, PRU0_R31_VEC_VALID  ; allows notification of program completion
  .asg  3,  PRU_EVTOUT_0        ; the event number that is sent back
 
START:
TOP:
  ; Wait 100K cycles
  LDI32  r0, 49999999
WAIT1:
  SUB r0, r0, 1
  QBNE WAIT1, r0, 0
   
  SET     r30, r30.t5          ; Turn on the LED
   
  ; Wait 100K cycles
  LDI32  r0, 49999999
WAIT2:
  SUB r0, r0, 1
  QBNE WAIT2, r0, 0
   
  CLR     r30, r30.t5          ; Turn off the LED
   
  QBA TOP