
RESET_ADDR  = 0xD000
CONFIG_ADDR = 0xDFFF

.globl _soft_reset

_soft_reset:
    ; copy to ram and jump
    ld de, #RESET_ADDR   ; Carrega 0xD000 no registrador DE, esse endereço vai ter o soft reset
    ld hl, #SoftReset      ; inicio do soft de rest
    ld bc, #(SoftResetEnd - SoftReset)   ; tamanho do soft de reset
CopyToRamLoop:
    ld a, (hl)          ; pega um valor do softreset
    ld (de), a          ; manda pra ram em $D000
    inc de              ; inc endereço da memoria
    inc hl              ; inc endereço do codigo de soft reset
    dec bc              ; decrementa contador
    ld a, b             ; carrega A com B, e abaixo faz um OR com C, se BC for zero,
    or c                ; a flaf f é zerada, terminando o loop
    jp nz, CopyToRamLoop  ; Loop while the result is non-zero

    jp RESET_ADDR
SoftReset:
    di     ; disable interrupt
    ld hl,   #0xFFFE
    ld (hl),  #0x01    ; reset slot1 mapper
    ld hl,   #0xFFFF
    ld (hl),  #0x02    ; reset slot2 mapper

    ;Port bits
    ;Bit	Function	
    ;7	    Expansion slot enable
    ;6*	    Cartridge slot enable
    ;5	    Card slot enable
    ;4*	    RAM enable	 
    ;3	    BIOS ROM enable	
    ;2*	    I/O enable
    ;1	    Unknown	 
    ;0	    Unknown	 
    ld a, #~(1 << 6  | 1 << 4 | 1 << 2) 
    ld (0xC000), a
    out (0x3E), a

    ld hl,   #0x4004  ; endereço do CPLD q desativa as magic port
    ; Byte bits
    ;Bit Function
    ;3      SD Turbo enable em 1
    ;2      START ROM SLOT2 enable em 0
    ;1      KOREN MAP ENABLE enable em 1
    ;0      SD chip select
    ;ld (hl), #0x01    ; 0000-0001 = START 0(enable rom slot2), KOREAN_MAP 0, SDCS 1,... rom no slot2
    ;ld (hl), #0x03    ; 0000-0001 = START 0(enable rom slot2), KOREAN_MAP 0, SDCS 1,... rom no slot2
    
    ld de, #CONFIG_ADDR
    ld a, (de)
    ld (hl), a
    jp 0              ; pula pro inicio da rom
SoftResetEnd: