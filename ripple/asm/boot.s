
.memorymap
   defaultslot     1
   
   ;===============================================
   ; RAM area
   ;===============================================
   
   slotsize        $800
   slot            0       $0000
   slotsize        $2000
   slot            1       $6000
   
   ;===============================================
   ; ROM area
   ;===============================================
   
   slotsize        $4000
   slot            2       $8000
   slot            3       $C000
.endme

.rombankmap
  bankstotal 16
  banksize $4000
  banks 16
.endro

.emptyfill $FF

.background "ripple.nes"

; unbackground expanded ROM space
.unbackground $2C000 $3BFFF

; Existing bankswitching routines
.define switchBank02 $EEF8
.define switchBank03 $EEFC
.define switchBank05 $EF00
.define switchBank06 $EF04
;.define switchBank   $EF06

.define PPUCTRL $2000
.define PPUMASK $2001
.define PPUSTATUS $2002
.define OAMADDR $2003
.define OAMDATA $2004
.define PPUSCROLL $2005
.define PPUADDR $2006
.define PPUDATA $2007
.define OAMDMA $4014

.define scratch $0012
.define scratchL $0012
.define scratchH $0013

.define newFontPpuAddr $0040

; Location of new buffer to hold raw strings
.define newRawStrBuf $0740
.define newRawStrBufSize $B4

;===============================================
; Include new graphics banks
;===============================================

.bank 0 slot 2
.org $0000
.section "new graphics 0" overwrite
  .incbin "out/grp/bank0.bin" FSIZE grpBank0Size
.ends

.bank 1 slot 2
.org $0000
.section "new graphics 1" overwrite
  .incbin "out/grp/bank1.bin" FSIZE grpBank1Size
.ends

.bank 2 slot 2
.org $0000
.section "new graphics 2" overwrite
  .incbin "out/grp/bank2.bin" FSIZE grpBank2Size
.ends

; bank 2 contains other data after the graphics; make sure we
; don't overwrite it
.if grpBank2Size >= $2E93
  .fail
.endif

;===============================================
; Include new script.
; Due to increased size and lack of compression,
; the new script is split across two banks.
;===============================================

; Area number at which scripts are split.
; An important point to note here: the game relies on
; empty regions being redirected to the next region
; with actual content. Region 3 is empty, and thus
; the split gets redirected to Region 4; therefore,
; the split cannot occur at Region 4.
;.define scriptSplitPoint 3
.define scriptSplitPoint1 2
.define scriptSplitPoint2 3
.define scriptSplitPoint3 5

; part 1: ROM $10000

.bank 4 slot 2
.org $0000
.section "Script, part 1" overwrite
  scriptPart1: .incbin "out/script/script_0.bin"
.ends

; part 2: ROM $20000

.bank 8 slot 2
.org $0000
.section "Script, part 2" overwrite
  scriptPart2: .incbin "out/script/script_1.bin"
.ends

; part 3: ROM $24000

.bank 9 slot 2
.org $0000
.section "Script, part 3" overwrite
  scriptPart3: .incbin "out/script/script_2.bin"
.ends

; part 4: ROM $28000

.bank 10 slot 2
.org $0000
.section "Script, part 4" overwrite
  scriptPart4: .incbin "out/script/script_3.bin"
.ends

.bank 15 slot 3
.org $35B4
.section "Use script splitting logic" overwrite
  jsr calcScriptSplit
.ends

.slot 3
.section "Script splitting logic" free

  ;===============================================
  ; Decide which bank target string is in and
  ; switch to it.
  ;===============================================
  calcScriptSplit:
    lda $00F0   ; get area num
    cmp #scriptSplitPoint1
    bcs @highScript1
		
		  lda #:scriptPart1
		  jmp switchBank
    
    @highScript1:
    cmp #scriptSplitPoint2
    bcs @highScript2
    
      lda #:scriptPart2
      jmp switchBank
    
    @highScript2:
    cmp #scriptSplitPoint3
    bcs @highScript3
    
      lda #:scriptPart3
      jmp switchBank
  
    @highScript3:
    lda #:scriptPart4
    jmp switchBank
    
    
.ends

;===============================================
; Adjust y-position of multiple-choice cursor
;===============================================

.bank 15 slot 3
.org $298C
.section "Multiple choice cursor y-position 1" overwrite
  ldy #$AF    ; option 1 y-position (orig #$AD)
.ends

.bank 15 slot 3
.org $2992
.section "Multiple choice cursor y-position 2" overwrite
  ldy #$BF    ; option 2 y-position (orig #$CD)
.ends

;===============================================
; New logic for writing item names.
;===============================================

.define newItemNameSize $0C

  ;===============================================
  ; Target new buffer.
  ;===============================================

  .bank 15 slot 3
  .org $2F59
  .section "New item name write logic: target new buffer 1" overwrite
    sta newRawStrBuf,X
  .ends

  .bank 15 slot 3
  .org $2F75
  .section "New item name write logic: target new buffer 2" overwrite
    sta newRawStrBuf,X
  .ends

  ;===============================================
  ; Start of routine.
  ;===============================================

  .bank 15 slot 3
  .org $2F3B
  .section "New item name write logic 1" overwrite
    jsr triggerNewItemLogic
  .ends

  .bank 15 slot 3
  .section "New item name write logic (free)" free
    triggerNewItemLogic:
    
      ; make up work
      jsr clearRawStrBuf
      
      lda #:newItemLogicStart
      jsr switchBank
      jmp newItemLogicStart
      
  .ends

  ;===============================================
  ; Don't have items.
  ;===============================================

  .bank 15 slot 3
  .org $2F8A
  .section "New item name write logic 2" overwrite
    jmp endNewItemLogic
  .ends

  ;===============================================
  ; Have items.
  ;===============================================

  .bank 15 slot 3
  .org $2F80
  .section "New item name write logic 3" overwrite
    jmp successEndNewItemLogic
  .ends

  .bank 15 slot 3
  .section "New item name write logic successful end (free)" free
    successEndNewItemLogic:
      
      jsr switchBank06
      jmp $F07A
      
  .ends
  
.slot 2
.section "New item name write logic (superfree)" superfree
  ;===============================================
  ; Translated item names.
  ;===============================================
  areaItems0: .incbin "out/rawstr/items_0.bin" FSIZE areaItems0Size
  areaItems1: .incbin "out/rawstr/items_1.bin" FSIZE areaItems1Size
  areaItems2: .incbin "out/rawstr/items_2.bin" FSIZE areaItems2Size
  areaItems3: .incbin "out/rawstr/items_3.bin" FSIZE areaItems3Size
  areaItems4: .incbin "out/rawstr/items_4.bin" FSIZE areaItems4Size
  
  ;===============================================
  ; New item writing logic.
  ;===============================================
  newItemLogicStart:
  
    ; make up work
    jmp clearRawStrBuf
    
  endNewItemLogic:
  
    ; make up work
    lda #$00
    sta $005B
    
    jmp switchBank06
    
    
.ends

.bank 15 slot 3
.org $2F94
.section "Update item name list pointers" overwrite
  .dw areaItems0
  .dw areaItems1
  .dw areaItems2
  .dw areaItems3
  .dw areaItems4
.ends

.bank 15 slot 3
.org $2F50
.section "Expanded item names 1" overwrite
  lda #newItemNameSize  ; 12 bytes instead of 8
.ends

.bank 15 slot 3
.org $2F6B
.section "Expanded item names 2" overwrite
  adc #newItemNameSize  ; 12 bytes instead of 8
.ends

;===============================================
; Clear our new text buffer with spaces instead
; of the original raw text buffer.
;===============================================

.bank 15 slot 3
.org $3109
.section "Clear raw string buffer" overwrite
  clearRawStrBuf:
    lda #$00
    ldy #newRawStrBufSize
    -:
      dey
      sta newRawStrBuf
      bne -
    rts
.ends

;===============================================
; Adjust item menu cursor locations to allow
; for expanded names.
; (original is 3-column with 7 characters per
; name; ours is 2-column with 11 characters per
; name)
;===============================================

.bank 15 slot 3
.org $30F9
.section "New item menu cursor positions" overwrite
  ; format: x1, y1, x2, y2, ...
  
  ; double-spaced version
  ; might end up doing this if I decide I really, really
  ; need 2-line item names
;  .db $17, $A7  ; position 1
;  .db $77, $A7  ; position 2
;  .db $17, $B7  ; position 3
;  .db $77, $B7  ; position 4
;  .db $17, $C7  ; position 5
;  .db $77, $C7  ; position 6
;  .db $17, $D7  ; position 7
;  .db $77, $D7  ; position 8
  
  ; single-spaced
  .db $17, $A7  ; position 1
  .db $77, $A7  ; position 2
  .db $17, $AF  ; position 3
  .db $77, $AF  ; position 4
  .db $17, $B7  ; position 5
  .db $77, $B7  ; position 6
  .db $17, $BF  ; position 7
  .db $77, $BF  ; position 8
  
.ends

;===============================================
; Adjust item menu cursor logic to account for
; new layout.
;===============================================

.bank 15 slot 3
.org $30A2
.section "New item menu cursor logic 1" overwrite
  nop   ; down pressed: 2 slots instead of 3
.ends

.bank 15 slot 3
.org $30AB
.section "New item menu cursor logic 2" overwrite
  nop   ; up pressed: 2 slots instead of 3
.ends

;===============================================
; Use new, expanded buffer for raw strings.
;===============================================

.bank 15 slot 3

.org $3606
.section "New raw string buffer 1" overwrite
  sta newRawStrBuf,X
.ends

.org $361E
.section "New raw string buffer 2" overwrite
  sta newRawStrBuf,X
.ends

.org $363A
.section "New raw string buffer 3" overwrite
  sta newRawStrBuf,X
.ends

.org $3665
.section "New raw string buffer 4" overwrite
  lda newRawStrBuf,X
.ends

.org $364B
.section "New raw string buffer 5" overwrite
  ; normally, X = strlen, but for password, decompression is skipped and
  ; it will be FF
  cpx #$FF
  beq +
    ; add #$FF terminator to raw string
    dex
    lda #$FF
    sta newRawStrBuf,X
    +:
    
.ends

;===============================================
; Don't use 8x16 font for passwords.
;===============================================

.bank 2 slot 2
.org $3387
.section "No 8x16 passwords" overwrite
  ; don't multiply index by 2
  nop
  clc
  ; add 5 instead of 0x24 to get target tile
  adc #$5
.ends

.bank 2 slot 2
.org $339A
.section "No 8x16 passwords 2" overwrite
  ; don't write lower half of character
  nop
  nop
  nop
.ends

;===============================================
; Hardcoded password strings.
;===============================================

; Number of bytes from start of passwordLabel to the position
; at which the password content should be written.
.define passwordContentOffset $20
.define passwordLength $14

.slot 2
.section "Password strings" superfree
  passwordLabel:
    .incbin "out/rawstr/password.bin" FSIZE passwordLabelSize
  passwordBadLabel:
    .incbin "out/rawstr/password_bad.bin" FSIZE passwordBadLabelSize
.ends

.bank 15 slot 3
.org $36DE
.section "Use new password string" overwrite

  ; switch bank
  lda #:passwordLabel
  jsr switchBank

    ; copy in raw "password" label
    ldy #(passwordLabelSize - 1)
    -:
      lda passwordLabel,Y
      sta newRawStrBuf,Y
      dey
      bpl -

  ; switch to old bank
  jsr switchBank06
  
  ; insert formatted password into string
  ldx #$00
  ldy #passwordContentOffset
  -:
    ; fetch next byte of password
    lda $0710,X
    
    ; add 1 to get actual character index for password value
    clc
    adc #$01
    
    sta newRawStrBuf,Y
    
    inx
    iny
    
    cpx #passwordLength
    bne -
  
  ; finish up
  inc $003F
  jmp $F696
  
.ends

;===============================================
; Write "password is wrong" message.
;===============================================

.bank 15 slot 3
.org $3727
.section "Password wrong message" overwrite

  ; switch bank
  lda #:passwordBadLabel
  jsr switchBank
  
    ; copy in raw "password wrong" label
    ldy #(passwordBadLabelSize - 1)
    -:
      lda passwordBadLabel,Y
      sta newRawStrBuf,Y
      dey
      bpl -

  ; switch to old bank
  jsr switchBank06
  
  jmp $F717
    
.ends

;===============================================
; Shift password text when modifying
; characters.
;===============================================

.bank 2 slot 2
.org $337F
.section "Move password down" overwrite
  ; orig: #$44
  adc #$44
.ends

;===============================================
; Move password character selection cursor up.
;===============================================

.bank 6 slot 2
.org $33FE
.section "Move password cursor up" overwrite
  lda #$C1
.ends

;===============================================
; Skip string formatting step.
;===============================================

.bank 15 slot 3
.org $3670
.section "Skip string formatting" overwrite

  jmp $F68C

;  nop
;  jmp stringFormatSkipCheck
  

.ends

;.bank 15 slot 3
;.section "String formatting skip check" free
;  
;  stringFormatSkipCheck:
;    
;    ; make up work
;    sta $0000
;    
;    ; check if roomnum == A0 (credits)
;    lda $0057
;    cmp #$A0
;    beq +
;      
;      ; not room A0: no string formatting
;      jmp $F68C
;    
;    +:
;    
;      ; do string formatting
;      
;      ldy #$00
;      jmp $F674
;
;.ends

;===============================================
; Send raw strings directly to PPU.
;===============================================

.bank 2 slot 2
.org $3244
.section "Raw strings to PPU 1" overwrite
  ; original code sets up pointer access to $0500
  lda #<newRawStrBuf
  sta $001A
  lda #>newRawStrBuf
  sta $001B
.ends

.bank 2 slot 2
.org $3283
.section "Raw strings to PPU 2" overwrite
  nop
  sta $000C
  ; redirect from $001C to $001A
  lda ($001A),Y
.ends

;===============================================
; When bankswitching, do useless shift-outs
; before saving XY so we can simply call
; switchBank with A as bank number.
;===============================================
.bank 15 slot 3
.org $2F06
.section "Change bankswitching XY save order" overwrite

  lsr
  lsr
  lsr
  lsr
  
  switchBank:
    stx $0027
    sty $002A
  
.ends

;===============================================
; Load fixed 8x8 font during game startup.
;===============================================
;.bank 6 slot 2
;.org $003B
;.section "Call font loading code during startup" overwrite
;  jsr loadFont 
;.ends

;===============================================
; Adjust title screen cursor position 8 pixels
; left so longer English strings are centered
;===============================================
;.bank 3 slot 1
;.org $01DF
;.section "titlecursor" overwrite
;  ld d,$58  ; x-position (orig: 60)
;.ends

;===============================================
; Don't copy font to VRAM when getting ready
; to print strings (since we're now using an
; always-loaded 8x8 font)
;
; This removes the check during NMI to the
; $0031 flag and corresponding call to the
; font VRAM transfer routine at 1FA3B-1FAC9.
;===============================================
;.bank 15 slot 3
;.org $3F60
;.section "Don't copy font" overwrite
;
;  jmp $FF6A
;  .rept 7
;    nop
;  .endr
;  
;  jmp fontCopyCheck
;
;.ends

;.bank 15 slot 3
;.section "Font copy check" free
;  
;  fontCopyCheck:
;
;  ; branch if roomnum == A0 (credits)
;  lda $0057
;  cmp #$A0
;  beq @doLoadFontCheck
;  
;    ; not at credits
;    ; shut off any pending character transfer requests
;    lda #$00
;    sta $0031
;    @nullEnd:
;      jmp $FF6A   ; jump to next NMI update check
;  
;  ; at credits
;  ; handle any pending requests
;  @doLoadFontCheck:
;    
;    lda $0031
;    beq @nullEnd  ; if no transfer requested, do nothing
;      jsr $FA3B   ; transfer a character
;      jmp $FF85   ; skip other possible NMI updates
;    
;
;.ends

; freed by above
;.unbackground $3FA3B $3FAC9

; free the dictionary
.unbackground $3F760 $3F91F
  
;===============================================
; Advance 1 row instead of 2 after each line
; of text.
;===============================================
.bank 2 slot 2
.org $3260
.section "1 row per line" overwrite
  adc #$20    ; advance 0x20 tiles instead of 0x40
.ends
  
;===============================================
; Don't write lower half of character.
;===============================================
.bank 2 slot 2
.org $32D0
.section "Don't write lower half of character" overwrite
  inc $0043
  inc $0042
  rts
.ends
  ;.bank 2 slot 2
  ;.org $32D0
  ;.section "Don't write lower half of character" overwrite
  ;  jmp $B2E9
  ;.ends
  
;===============================================
; Extend linebreak calculation to allow for up
; to 8 lines of text.
;===============================================

.bank 2 slot 2
.org $328A
.section "Branch to newLinebreakCalc" overwrite
  beq newLinebreakCalc
.ends

.bank 2 slot 2
.org $32D5
.section "Extend number of lines possible by linebreak" overwrite
  newLinebreakCalc:
    -:
      ; advance putpos until it reaches the next 0x18-byte boundary
      ; goes up to 0xA8 for 7 linebreaks = 8 lines total
      lda $0042
      
      cmp #$18
      beq +
      cmp #$30
      beq +
      cmp #$48
      beq +
      cmp #$60
      beq +
      cmp #$78
      beq +
      cmp #$90
      beq +
      cmp #$A8
      beq +
      
      ; advance putpos
      inc $0042
      jmp -
    ; skip getpos past linebreak character
    +:
      inc $0043
      rts 
.ends
  
;===============================================
; Allow strings longer than 0x80 characters.
;===============================================

.bank 2 slot 2
.org $3258
.section "Correct line length calculation 1" overwrite
  jmp newNumLinesCheck
.ends

.bank 15 slot 3
.section "Correct line length calculation 2" free
  ;===============================================
  ; code that happens to fit here
  ;===============================================
  
  newNumLinesCheck:
    sbc #$18    ; xpos -= 0x18
    cmp #-($18 + 1)
    bcs +
    
      ; next iteration
      jmp $B25C
      
    +:
    ; done
    jmp $B26D
.ends
  
;===============================================
; Double text speed to 1 character per frame.
;===============================================

;.bank 2 slot 2
;.org $3235
;.section "Increase text speed" overwrite
;;  and #$01
;  and #$00    ; AND with #$00 instead of #$01 so check for
;;              ; cleared low bit of frame counter always succeeds
;.ends

.bank 2 slot 2
.org $3235
.section "Increase text speed" overwrite
  jmp textSpeedCheck
.ends

.bank 15 slot 3
.section "new text speed check" free
  textSpeedCheck:
    ; intro (rooms 74-77) uses half-speed text to keep music synchronized
    ; get current room id
    lda $0057
    
;    ; if >= 0x9B (true ending), quarter speed
;    cmp #$9B
;    bcc +
;      ; if in half-speed range, check low bit of frame counter
;      lda $0026
;      and #$03
;      beq @doUpdate
;      bne @noUpdate
;    +:
    
    ; if >= 0x9B (true ending), half speed
    cmp #$9B
    bcs @doHalfUpdate
    
    ; if == 97 (neutral ending), half speed
    cmp #$97
    bcs @doHalfUpdate
    
    ; if not 74-77 inclusive, always print next character
    cmp #$74
    bcc @doUpdate
      cmp #$78
      bcs @doUpdate
        
        @doHalfUpdate:
        ; if in half-speed range, check low bit of frame counter
        lda $0026
        and #$01
        beq @doUpdate
    
    @noUpdate:
    jmp $B30C
    
    @doUpdate:
    jmp $B23C
    
.ends

; only do text tone updates on alternate frames

.bank 2 slot 2
.org $32AF
.section "call text tone halfspeed" overwrite
  jmp textToneCheck
.ends

.bank 15 slot 3
.section "new text tone check" free
  textToneCheck:
    ; check low bit of frame counter
    lda $0026
    and #$01
    beq @noUpdate
    
    ; make up work
    ldx #$02
    ldy #$20
    jmp $B2B3
    
  @noUpdate:
  jmp $B2BE
    
    
.ends

;===============================================
; Deal with intro music sync
;===============================================

; delay between boxes for intro parts 1-3

.bank 15 slot 3
.org $1BA2
.section "call new intro main timer update" overwrite
  jmp newIntroMainTimerUpdate
.ends

.bank 15 slot 3
.section "new intro main timer" free
  newIntroMainTimerUpdate:
  
    lda $002F
    tax
    
    ; this routine is used to compute delay for both the intro and the true
    ; ending, but the true ending needs a different delay in the translation
    ; (since it doesn't take into account the time spent printing the messages,
    ; unlike the intro)
    
    lda $0057
    
    ; check if in "happy" ending (roomnum == 0xA2)
;    cmp #$A2
;    beq @happyend
;    beq +
;      @happyend:
;      cpx #$FF
;      bne +
;        jsr $DB88
;        rts
;    +:
    
    ; check if in true ending (roomnum >= 0x9B)
    cmp #$9B
    bcc @notEnding
    
    @ending:
      cpx #$00
      jmp @end
    @notEnding:
      cpx #$D8    ; timer must reach this value before proceeding
    
    @end:
    
    ; branch if delay not finished
    bne +
      lda #$00
      sta $002F
      jsr $DB88
    +:
    rts
.ends

; delay between boxes for intro part 4

.bank 15 slot 3
.org $1B75
.section "adjust intro part 4 autodelay" overwrite
  cmp #$A0      ; value upcounter must reach before next dialogue starts
                ; (orig: D8)
;  cmp #$D8
.ends

;===============================================
; *** SUPERFREE CODE ***
;===============================================

.slot 2
.section "New font" superfree
  ;===============================================
  ; New font graphics
  ;===============================================
  newFont:
;    .incbin "out/font/font_8x8.bin" FSIZE newFontSize
    .incbin "out/cmp/font_8x8.bin" FSIZE newFontSize
  
  .fopen "../out/precmp/font_8x8.bin" fp
    .fsize fp rawNewFontSize
  .fclose fp
  .define numNewFontTiles (rawNewFontSize/$10)
  
  ;===============================================
  ; "New" credits font
  ;===============================================
  newCreditsFont:
    .incbin "out/font_credits/font.bin" FSIZE newCreditsFontSize
  
  .define numNewCreditsFontChars (newCreditsFontSize/$20)
  
  ;===============================================
  ; New title graphics
  ;===============================================
  newTitleGrp:
;    .incbin "out/font/font_8x8.bin" FSIZE newFontSize
    .incbin "out/cmp/title_grp.bin" FSIZE newTitleGrpSize
  
  .fopen "../out/precmp/title_grp.bin" fp
    .fsize fp rawNewTitleGrpSize
  .fclose fp
  .define numNewTitleGrpTiles (rawNewTitleGrpSize/$10)
  
  ;===============================================
  ; "New" interface graphics (same as old)
  ;===============================================
  newInterfaceGrp:
;    .incbin "out/font/font_8x8.bin" FSIZE newFontSize
    .incbin "out/cmp/interface_grp.bin" FSIZE newInterfaceGrpSize
  
  .fopen "../out/precmp/interface_grp.bin" fp
    .fsize fp rawNewInterfaceGrpSize
  .fclose fp
  .define numNewInterfaceGrpTiles (rawNewInterfaceGrpSize/$10)
  
  ;===============================================
  ; New title tilemap
  ;===============================================
  newTitleTilemap:
    .incbin "out/maps/title.bin" FSIZE newTitleTilemapSize
  
  ;===============================================
  ; Load the new font graphics to VRAM
  ;===============================================
/*  newFontLoad:
    ; set PPU address
    lda #>newFontPpuAddr
    sta PPUADDR
    lda #<newFontPpuAddr
    sta PPUADDR
    
    ; set up font pointer
    lda #<newFont
    sta scratchL
    lda #>newFont
    sta scratchH
    
    ; copy in font
    ldx #(newFontSize/$10)
    
    ; pattern copy loop
    @patCopyLoop:
      ldy #$00
      
      ; copy pattern at a time
      .rept ($10 - 1)
        ; send next byte to PPU
        lda (scratch),Y
        sta PPUDATA
        iny
      .endr
      ; send last byte to PPU
      lda (scratch),Y
      sta PPUDATA
      
      ; increment getpos
      clc
      lda scratchL
      adc #$10
      sta scratchL
      lda scratchH
      adc #$00
      sta scratchH
      
      ; loop
      dex
      bne @patCopyLoop
    
    ; restore bank 6
    jmp switchBank06*/
    
.ends

;.define newStuffBank :newFont

;===============================================
; Load new title screen resources
;===============================================

.define interfaceGrpPpuAddr $1000
.define titleGrpPpuAddr $1400
.define fontGrpPpuAddr $0000

.bank 15 slot 3
.org $2E0A
.section "updated title screen load" overwrite

  jsr switchToNewStuffBank
  
  ; load interface
  ldy #>interfaceGrpPpuAddr
  lda #<interfaceGrpPpuAddr
  ldx #numNewInterfaceGrpTiles
  jsr $EE31     ; decompress
    .dw newInterfaceGrp
  
  ; load title graphics
  ldy #>titleGrpPpuAddr
  lda #<titleGrpPpuAddr
  ldx #numNewTitleGrpTiles
  jsr $EE31     ; decompress
    .dw newTitleGrp
  
  ; load font
  ldy #>fontGrpPpuAddr
  lda #<fontGrpPpuAddr
  ldx #numNewFontTiles
  jsr $EE31     ; decompress
    .dw newFont
  
  ; switch to bank 6
  jmp $EF04
  
.ends

;===============================================
; Load new title screen tilemap
;===============================================

.bank 6 slot 2
.org $00A9
.section "trigger new title tilemap load" overwrite
  jsr triggerNewTitleTilemapLoad
.ends

.define tilemapScratch   $0059
.define tilemapScratchLo $0059
.define tilemapScratchHi $005A
.define ppuScratch   $0061
.define ppuScratchLo $0061
.define ppuScratchHi $0062
.define titleTilemapPpuDst $2480

.define ppuTransferDataSize $40

.define initPpuTransfer $89DC

.bank 15 slot 3
.section "start new title tilemap load" free

  ;===============================================
  ; Copy of routine that transfers byte to
  ; pending ppu write command
  ;===============================================
  
  queuePpuByte:
    stx $0058
    ldx $0053
    sta $0400,X
    inc $0053
    ldx $0058
    rts
  
  finalizePpuCommand:
    ; update current command start pos?
    lda $0055
    clc
    adc $0052
    sta $0055
    ; increment number of pending transfers?
    inc $0035
    rts
    
  triggerNewTitleTilemapLoad:
    ; make up work: load original tilemap
    jsr $8354
    
    ; set up srcaddr
    lda #>newTitleTilemap
    sta tilemapScratchHi
    lda #<newTitleTilemap
    sta tilemapScratchLo
    
    lda #>titleTilemapPpuDst
    sta ppuScratchHi
    lda #<titleTilemapPpuDst
    sta ppuScratchLo

    ldx #$07
    
      @loop:
      txa
      pha
        
        jsr queueTilemapRowLoad
        
        ; loop
        pla
        tax
        dex
        bne @loop
    
    ; wait for pending transfers?
    -:
      lda $0035
      bne -
    
    ; finished: switch to bank 6
    jmp switchBank06
  
  queueTilemapRowLoad:
    ; switch to bank with setup code
    jsr switchBank06
      
      ; write command header and prep for data write
      lda #ppuTransferDataSize
      ldy ppuScratchHi
      ldx ppuScratchLo
      jsr initPpuTransfer
  
    ; switch to bank with tilemap
    jsr switchToNewStuffBank
    
      ; write transfer data to command
      ldy #$00
      -:
        ; fetch and queue next byte
        lda (tilemapScratch),Y
        jsr queuePpuByte
        iny
        cpy #ppuTransferDataSize
        bne -
    
    ; update srcaddr
    tya
    clc
    adc tilemapScratchLo
    sta tilemapScratchLo
    lda #$00
    adc tilemapScratchHi
    sta tilemapScratchHi
    
    ; update dstaddr
    tya
    clc
    adc ppuScratchLo
    sta ppuScratchLo
    lda #$00
    adc ppuScratchHi
    sta ppuScratchHi
    
    jsr switchBank06
    
    ; finalize command for execution
    jmp finalizePpuCommand
    
.ends

;===============================================
; fix cloud overlapping hidden title screen
;===============================================

.bank 6 slot 2
.org $0171
.section "fix cloud overlap" overwrite
  ; set initial cloud x-position (orig: E8?)
  lda #$18
.ends

;===============================================
; New credits loading
;===============================================

; Since the $0031 flag for loading new font characters to VRAM is no
; longer needed, we disable it and repurpose it as a flag for loading
; the credits font.

  ;===============================================
  ; disable use of 0031 flag for pending string
  ; pattern VRAM transfer
  ;===============================================

  .bank 15 slot 3
  .org $36AC
  .section "no 0031 increment" overwrite
    nop
    nop
  .ends

  .bank 15 slot 3
  .org $3AC5
  .section "no 0031 decrement" overwrite
    nop
    nop
  .ends

  ;===============================================
  ; Set 0031 flag when on credits screen
  ;===============================================
  
  .bank 15 slot 3
  .org $369C
  .section "set new credits start flag" overwrite
    inc $0031
    jmp $F6A6
  .ends

  ;===============================================
  ; Trigger new 0031 handler at NMI
  ;===============================================
  
  .bank 15 slot 3
  .org $3F64
  .section "trigger new 0031 handler" overwrite
    jsr creditsLoadUpdate
  .ends
  
  .define creditsScratch1 $001C
  .define creditsScratch1Lo $001C
  .define creditsScratch1Hi $001D
  .define creditsScratch2 $000A
  .define creditsScratch2Lo $000A
  .define creditsScratch2Hi $000B
  
  .define creditsBasePpuPos $0040
  
  .bank 15 slot 3
  .section "new credits load logic" free
    creditsLoadUpdate:
      
      tax
      dex
      
      ; stop when all characters loaded
      cpx #numNewCreditsFontChars
      bne +
        ; clear load flag
        lda #$00
        sta $0031
        rts
      +:
      
      ;=======================================================
      ; compute src/dst addresses ((index * 0x20) + baseaddr)
      ;=======================================================
      ; write 16-bit index
      txa
      sta creditsScratch1Lo
      lda #$00
      sta creditsScratch1Hi
      
      ; multiply index by 0x20
      .rept 5
        asl creditsScratch1Lo
        rol creditsScratch1Hi
      .endr
      
      ; compute PPU putpos to scratch2
      clc
      lda #<creditsBasePpuPos
      adc creditsScratch1Lo
      sta creditsScratch2Lo
      lda #>creditsBasePpuPos
      adc creditsScratch1Hi
      sta creditsScratch2Hi
      
      ; add to address to get srcpos in scratch1
      clc
      lda creditsScratch1Lo
      adc #<newCreditsFont
      sta creditsScratch1Lo
      lda creditsScratch1Hi
      adc #>newCreditsFont
      sta creditsScratch1Hi
      
      ;=======================================================
      ; copy 2 patterns to VRAM
      ;=======================================================
      
      ; set PPU address
      ; reset address latch
      lda PPUSTATUS
      ; set PPU address
      lda creditsScratch2Hi
      sta PPUADDR
      lda creditsScratch2Lo
      sta PPUADDR
      
      ; load bank containing font
      jsr switchToNewStuffBank
      
      ; pattern copy loop
      ldy #$00
      @patCopyLoop:
        ; send next byte to PPU
        lda (creditsScratch1),Y
        sta PPUDATA
        
        ; loop
        iny
        ; copy 2 patterns
        cpy #$20
        bne @patCopyLoop
      
      ;=======================================================
      ; finish
      ;=======================================================
      
      ; increment index
      inc $0031
      jmp switchBank06
      
  .ends

;===============================================
; Change cheat password for good ending
;===============================================

.bank 6 slot 2
.org $3464
.section "Trigger new cheat password check" overwrite
  jmp newCheatPasswordCheck
.ends

.bank 15 slot 3
.section "New cheat password check" free
  newCheatPassword:
    .incbin "out/rawstr/password_cheat.bin" FSIZE cheatPasswordSize

  newCheatPasswordCheck:
    ldx #(cheatPasswordSize - 1)
    -:
      ldy $0710,X
      iny
      tya
      cmp newCheatPassword.W,X
      bne @fail
      dex
      bpl -
    
    @success:
    jmp $B471
    
    @fail:
    jmp $B477
    
    
.ends

;===============================================
; Startup splash screen
;===============================================

/*.bank 6 slot 2
.org $0053
.section "trigger startup splash screen" overwrite
  jsr doStartupSplashScreen
  nop
.ends

.bank 15 slot 3
.section "startup splash screen" free
  doStartupSplashScreen:
    lda #$FF
    sta $07FF
    
    lda #$20
    ldx #$00
    ldy #$00
    jsr $FC63
        
    ; clear tilemap
    lda #$24
    ldx #$05
    ldy #$00
    jsr $FC63
    
    ; load title graphics
    jsr $EE0A
    
    lda #$01
    --:
      pha
;      jsr $83F2
      
      -:
        lda $2002
        bpl -
      
      -:
        lda $2002
        bpl -

      pla
      tax
      dex
      txa
      bne --
    
    @done:
    ; make up work
    lda #$2A
    sta $0046
    rts
.ends */

; to hell with it, I don't feel like implementing this, let's just shove
; the names in the rom somewhere for posterity and call it a day
.slot 3
.section "hacking credits" superfree
  .asc "  Translation   "
  .asc "    Credits     "
  .asc "                "
  .asc "Hacking         "
  .asc "  Supper        "
  .asc "                "
  .asc "Translation     "
  .asc "  TheMajinZenki "
  .asc "                "
  .asc "Editing         "
  .asc "  cccmar        "
.ends

;===============================================
; *** NEW FIXED BANK CODE ***
;===============================================

.bank 15 slot 3
.section "New fixed bank code" free

  switchToNewStuffBank:
    lda #:newFont
    jmp switchBank

  ;===============================================
  ; Load the new 8x8 font starting at tile 04.
  ;===============================================
;  loadFont:
;    ; make up work
;    jsr $82CC
;    
;    ; switch to target bank
;    lda #:newFont
;    jsr switchBank
;    
;    ; call font load code
;    jmp newFontLoad
  
  
    
.ends


