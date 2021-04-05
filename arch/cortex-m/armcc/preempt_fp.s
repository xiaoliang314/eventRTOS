                        ;Copyright (C) 2021 xiaoliang<1296283984@qq.com>.

                        PRESERVE8
                        THUMB

                        AREA        |.text|, CODE, READONLY

PendSV_Handler          PROC
                        IMPORT      kevent_schedule
                        EXPORT      PendSV_Handler

                        ;PendSV异常处理程序通过插入伪造的异常返回上下文实现抢占被中断的线程的CPU
                        ;并在抢占结束后手动的回复被中断线程的上下文

                        ;当发生异常时，当前的LR与被压栈的xPSR指示了被中断的线程某些配置和状态，
                        ;LR:
                        ;BIT4: 当这个位被清除时，指示被中断线程使用了浮点上下文，并在栈中预留的浮点上下文的空间
                        ;BIT3: 当这个位被设置时，指示异常返回到Thread模式，否则返回到Handler模式，PendSV异常发生时，这个位应该为1
                        ;BIT2: 当这个位被设置时，指示异常返回使用PSP恢复线程上下文，否则使用MSP
                        ;
                        ;已压栈的xPSR:
                        ;BIT24: 当这个位被设置时，异常返回后进入Thumb模式，否则返回到ARM模式
                        ;BIT9: 当这个位被设置时，表示栈已开启8Byte对齐，并指示在弹出线程上下文后SP+4

                        ;备份LR的状态
                        MOV         R0, LR

                        ;检查LR BIT4确定线程是否使用了浮点上下文
                        LSLS        R1, R0, #27
                        BMI         THREAD_USE_FP_ENDIF

                        ;当线程使用了浮点功能时，我们需要执行一条浮点指令，触发Lazy Stacking做浮点上下文保存
                        VCMP.F32    S0, #0.0

                        ;我们需要插入伪造的无浮点上下文的异常返回现场，因此我们设置LR的BIT4和清除CONTROL的BIT2(FPCA)
                        ORRS        LR, #0x10
                        MRS         R1, CONTROL
                        BICS        R1, #4
                        MSR         CONTROL, R1
THREAD_USE_FP_ENDIF
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

                        ;我们需要保存LR的状态，在异常返回后使用它，因此将LR设置到返回后的R0中
                        STR         R0, [R2, #0]

                        ;我们设置堆栈中返回后的位置为PendSV_ExcReturnHandler
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

                        ;异常返回，进入PendSV_ExcReturnHandler
                        BX          LR
PENDSV_EXC_RETURN_HANDLER
                        ;保存异常时的LR的状态
                        PUSH        {R0}

                        ;调用调度程序
                        BL          k_schedule

                        ;当抢占程序执行结束之后，我们需要恢复抢占前的上下文
                        ;我们先对浮点上下文做恢复，此后再恢复非浮点部分的上下文
                        ;我们还需要检查栈对齐STKALIGN标志，确定恢复之后的栈顶位置

                        POP         {R0}

                        ;我们使用R3表示线程上下文恢复之后的栈顶位置
                        ;现在我们先跳过非浮点部分的上下文
                        ADD         R3, SP, #32

                        ;检查是否使用浮点上下文
                        LSLS        R1, R0, #27
                        BMI         THREAD_RESTORE_USE_FP_ENDIF

                        ;恢复浮点上下文，并将空位弹出
                        VLDMIA.F32  R3!, {S0-S15}
                        LDMIA       R3!, {R1}
                        VMSR        FPSCR, R1
                        ADDS        R3, #4

                        ;清除浮点使用状态
                        MRS         R1, CONTROL
                        BICS        R1, #4
                        MSR         CONTROL, R1
THREAD_RESTORE_USE_FP_ENDIF

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
                        STMDB       R3!, {R1}
                        STMDB       R3!, {R4}
                        MOV         R4, R3

                        ;恢复R12，LR，xPSR
                        MSR         xPSR, R0
                        LDRD        R12, LR, [SP, #16]

                        ;恢复R0-R3，SP
                        POP         {R0-R3}
                        MOV         SP, R4

                        ;返回抢占线程
                        POP         {R4, PC}

                        ENDP
                        END
