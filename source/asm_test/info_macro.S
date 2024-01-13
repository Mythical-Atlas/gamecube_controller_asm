.syntax unified
.thumb_func

.macro print_cpu_info
	ldr r5, CPUID               // Get the address of CPUID
	ldr r5, [r5]                // Get the contents of CPUID

	// Write out the CPUID register
	ldr r0, =TEXT0              // Set the printf format string
	movs r1, r5                 // Set the printf value
	bl printf

	// Write out the part number
	ldr r0, =TEXT1              // Set the format string
	movs r1, r5                 // Set the printf value to the CPUID value
	movs r3, #0xFF              // Set the AND mask
	lsls r3, #8
	adds r3, #0xF0
	ands r1, r3                 // Mask CPUID bits 15-4
	lsrs r3, #4
	bl printf

	// Write out the revision
	ldr r0, =TEXT2              // Set the format string
	movs r1, r5                 // Set the printf value to the CPUID value
	movs r3, #0x0F              // Set the AND mask
	lsls r3, #20
	ands r1, r3                 // Mask CPUID bits 23-20
	lsrs r3, #20
	bl printf

	// Write out the variant number
	ldr r0, =TEXT3              // Set the format string
	movs r1, r5                 // Set the printf value to the CPUID value
	movs r3, #0x0F              // Set the AND mask
	ands r1, r3                 // Mask CPUID bits 3-0
	bl printf

	// FROM 1.0.1 -- unified syntax test
	movs r0, #0                 // Clear r0
	mvns r0, r0                 // NOT r0 0x00000000 -> 0xFFFFFFFF
	movs r1, #1                 // r1 = 1
	rsbs r1, #0                 // r1 = 0 - r1 = 0xFFFFFFFF
	cmp r0, r1                  // r1 == r2?
	bne stop_printing_info      // Jump out if r1 != r2

	ldr r0, =TEXT4              // Output match message
	bl printf

stop_printing_info:				// exit the macro
.endm

/*
.align 4
CPUID:          .word           0xE000ED00          // Memory map address of CPUID register
TEXT0:          .asciz          "CPUID: 0x%08X\n"
TEXT1:          .asciz          "Part:  0x%08X\n"
TEXT2:          .asciz          "Rev:   0x%08X\n"
TEXT3:          .asciz          "Var:   0x%08X\n"
TEXT4:          .asciz          "MATCH\n"
*/