                        PRESERVE8
                        THUMB

                        AREA    |.text|, CODE, READONLY

PendSV_Handler          PROC
                        IMPORT      kevent_schedule
                        EXPORT		PendSV_Handler

                        ;PendSV异常处理程序通过插入伪造的异常返回上下文实现抢占被中断的线程的CPU
                        ;并在抢占结束后手动的回复被中断线程的上下文

                        ;当发生异常时，当前的LR与被压栈的xPSR指示了被中断的线程某些配置和状态，
                        ;LR:
                        ;BIT3: 当这个位被设置时，指示异常返回到Thread模式，否则返回到Handler模式，PendSV异常发生时，这个位应该为1
                        ;BIT2: 当这个位被设置时，指示异常返回使用PSP恢复线程上下文，否则使用MSP
                        ;
                        ;已压栈的xPSR:
                        ;BIT24: 当这个位被设置时，异常返回后进入Thumb模式，否则返回到ARM模式
                        ;BIT9: 当这个位被设置时，表示栈已开启8Byte对齐，并指示在弹出线程上下文后SP+4

                        ;备份LR的状态
                        MOV         R0, LR

                        ;我们需要插入异常返回现场需要使用的空间，并获取它的栈顶

                        ;检查LR BIT2确定异常返回使用PSP还是MSP
                        LSLS        R1, R0, #29
                        BMI         INSERT_FAKE_EXC_RETURN_CONTEXT_SPACE_USE_PSP

                        ;因为当前处于Handler模式，因此我们可以直接使用SP来操作MSP
                        SUB         SP, #32
                        MOV         R2, SP
                        B           INSERT_FAKE_EXC_RETURN_CONTEXT_SPACE_ENDIF

INSERT_FAKE_EXC_RETURN_CONTEXT_SPACE_USE_PSP
                        ;在Handler模式下，需要使用MRS与MSR指令操作PSP
                        MRS         R2, PSP
                        SUBS        R2, #32
                        MSR         PSP, R2
INSERT_FAKE_EXC_RETURN_CONTEXT_SPACE_ENDIF

                        ;我们设置堆栈中返回后的位置为PENDSV_EXCRETURNHANDLER
                        ADR         R0, PENDSV_EXC_RETURN_HANDLER
                        STR         R0, [R2, #24]

                        ;我们设置堆栈中xPSR，使异常返回后进入Thumb模式并恢复以前的IPSR状态
                        MOVS        R1, #1
                        LSLS        R1, #24
                        ;读取返回上下文中的IPSR
                        LDR         R0, [R2, #60]
                        LSLS        R0, #26
                        LSRS        R0, #26
                        ;设置xPSR为BIT24 + IPSR
                        ORRS        R1, R0
                        STR         R1, [R2, #28]

                        ;异常返回，进入PENDSV_EXCRETURNHANDLER
                        BX          LR
                        NOP
PENDSV_EXC_RETURN_HANDLER
                        ;调用调度程序
                        BL          kevent_schedule

                        ;当抢占程序执行结束之后，我们需要恢复抢占前的上下文
                        ;我们还需要检查xPSR的BIT9，确定恢复之后的栈顶位置

                        ;我们将R3指向线程恢复后栈顶的前两个字的位置，用于存放R4与PC
                        ADD         R3, SP, #24

                        ;读取xPSR
                        LDR         R0, [SP, #28]

                        ;判断xPSR的BIT9，若置位则栈顶需要+4
                        LSLS        R1, R0, #22
                        LSRS        R1, #31
                        LSLS        R1, #2
                        ADDS        R3, R1

                        ;读取线程的PC，并根据xPSR的BIT24设置Thumb或者ARM模式
                        LDR         R1, [SP, #24]
                        LSLS        R2, R0, #7
                        LSRS        R2, #31
                        ORRS        R1, R2

                        ;我们总是需要在R0-R12，LR，SP恢复之后才能执行返回
                        ;此时，我们没有可以使用的寄存器保存PC，
                        ;因此需要将PC存入栈中，使用POP指令进行恢复
                        ;同时我们也无法在恢复R0-R3之后，再保存栈顶的位置
                        ;因此我们必须使用R4记录栈顶，因此还需要对R4做保存
                        STR         R1, [R3, #4]
                        STR         R4, [R3, #0]
                        MOV         R4, R3

                        ;恢复R12，LR，xPSR
                        ;为了兼容M0，我们不使用LDRD指令来恢复R12与LR
                        MSR         xPSR, R0
                        ADD         R0, SP, #16
                        LDM         R0!, {R1-R2}
                        MOV         R12, R1
                        MOV         LR, R2

                        ;恢复R0-R3，SP
                        POP         {R0-R3}
                        MOV         SP, R4

                        ;返回被抢占的线程
                        POP         {R4, PC}

                        ENDP
                        END
