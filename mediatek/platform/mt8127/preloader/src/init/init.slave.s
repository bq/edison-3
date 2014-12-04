.equ SRAMROM_BOOT_ADDR ,0x10001800
.equ GIC_CPU_BASE      ,0x10212000
.equ SLAVE_JUMP_REG    ,0x10202034
.equ SLAVE1_MAGIC_REG  ,0x10202038
.equ SLAVE2_MAGIC_REG  ,0x1020203C
.equ SLAVE3_MAGIC_REG  ,0x10202040
.equ SLAVE1_MAGIC_NUM  ,0x534C4131
.equ SLAVE2_MAGIC_NUM  ,0x4C415332
.equ SLAVE3_MAGIC_NUM  ,0x41534C33

.globl	slave_core
.type	slave_core, function
slave_core:
    @ enable GIC
    movw r2, #:lower16:GIC_CPU_BASE
    movt r2, #:upper16:GIC_CPU_BASE
    mov r1,#0xF0
    str r1,[r2,#4]
    mov r1,#1
    str r1,[r2,#0]

    ldr r1, [r2]
    orr r1, #1
    str r1, [r2]

    ldr r9, =load_magic_num
    ldm r9, {r1-r8}
    ldr r9, =ramBase
    stm r9, {r1-r8}

    and r0, r0, #0x3
    teq r0, #0x2
    beq ca7_core2
    teq r0, #0x3
    beq ca7_core3

    @ bootrom power dowm mode
    movw r2, #:lower16:SRAMROM_BOOT_ADDR
    movt r2, #:upper16:SRAMROM_BOOT_ADDR
    ldr r4, =_start
    str r4, [r2]

ca7_core1:
    @ ca7 core 1
    movw r0, #:lower16:SLAVE1_MAGIC_REG
    movt r0, #:upper16:SLAVE1_MAGIC_REG
    movw r1, #:lower16:SLAVE1_MAGIC_NUM
    movt r1, #:upper16:SLAVE1_MAGIC_NUM
    sev
    b ramBase
ca7_core2:
    @ ca7 core 2
    movw r0, #:lower16:SLAVE2_MAGIC_REG
    movt r0, #:upper16:SLAVE2_MAGIC_REG
    movw r1, #:lower16:SLAVE2_MAGIC_NUM
    movt r1, #:upper16:SLAVE2_MAGIC_NUM
    wfe
    b ramBase
ca7_core3:
    @ ca7 core 3
    movw r0, #:lower16:SLAVE3_MAGIC_REG
    movt r0, #:upper16:SLAVE3_MAGIC_REG
    movw r1, #:lower16:SLAVE3_MAGIC_NUM
    movt r1, #:upper16:SLAVE3_MAGIC_NUM
    wfe
    b ramBase

load_magic_num:
    wfi
    ldr r2, [r0]
    cmp r2, r1
    bne load_magic_num
    movw r0, #:lower16:SLAVE_JUMP_REG
    movt r0, #:upper16:SLAVE_JUMP_REG
    ldr r1, [r0]
    mov pc, r1
