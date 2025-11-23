section .data
    divisor: dq 255.0          ; Double precision constant for division

section .text
    global imgCvtGrayIntToDouble

; Function: imgCvtGrayIntToDouble
; Parameters (Windows x64 calling convention):
;   RCX = height (int)
;   RDX = width (int)
;   R8 = pointer to input uint8 array
;   R9 = pointer to output double array

imgCvtGrayIntToDouble:
    push rbp
    mov rbp, rsp
    
    ; Calculate total number of pixels: height * width
    mov rax, rcx               ; rax = height
    imul rax, rdx              ; rax = height * width (total pixels)
    
    ; Check if size is zero
    test rax, rax
    jz .done
    
    ; Load the divisor (255.0) into xmm1
    movsd xmm1, [rel divisor]  ; xmm1 = 255.0
    
    ; Initialize loop counter
    xor r10, r10               ; r10 = 0 (loop index)
    
.loop:
    ; Load one byte (uint8) from input array
    movzx r11d, byte [r8 + r10] ; r11d = intImg[i] (zero-extended to 32-bit)
    
    ; Convert integer to double using scalar SIMD instruction
    cvtsi2sd xmm0, r11d         ; xmm0 = (double)r11d
    
    ; Divide by 255.0 using scalar SIMD floating-point instruction
    divsd xmm0, xmm1            ; xmm0 = xmm0 / 255.0
    
    ; Store result to output array
    ; Calculate output offset: index * 8 (since double is 8 bytes)
    mov rbx, r10
    shl rbx, 3                  ; rbx = r10 * 8
    movsd [r9 + rbx], xmm0      ; floatImg[i] = xmm0
    
    ; Increment loop counter
    inc r10
    
    ; Check if we've processed all pixels
    cmp r10, rax
    jl .loop                    ; Continue if r10 < total_pixels
    
.done:
    pop rbp
    ret