#define OK              0
#define KBC_IRQ         1    // Keyboard Interrupt Channel      

#define KBC_OUTBUF_REG  0x60 // read    Output Buffer    - Read scancodes or returned values from KBC commands
#define KBC_INBUF_REG   0x60 // write   Input Buffer     - Write arguments of KBC commands
#define KBC_STATUS_REG  0x64 // read    Status Register  - Read the KBC status
#define KBC_CMD_REG     0x64 // write   Command Register - Write KBC commands


/*
    STATUS REGISTER BYTE

    Bits 7 and 6 should always be checked in the Interrupt Handler since they signal communication errors between the keyboard and the KBC
    Even if an error occur, the Output Buffer must always be read but the value must be discarded

*/

#define ST_PARITY_BIT  (1 << 7)  // Parity error - invalid data
#define ST_TIMEOUT_BIT (1 << 6)  // Timeout error - invalid data
#define ST_AUX_BIT     (1 << 5)  // Mouse data
#define ST_INH_BIT     (1 << 4)  // Inhibit flag: 0 if keyboard is inhibited
#define ST_A2_BIT      (1 << 3)  // A2 input line: irrelevant for LCOM
#define ST_SYS_BIT     (1 << 2)  // System flag: irrelevant for LCOM
#define ST_IBF_BIT     (1 << 1)  // Input buffer full: don’t write commands or arguments
#define ST_OBF_BIT     (1 << 0)  // Output buffer full: data available for reading

 
//  CHECKERS for the Status Register bits 

#define ERROR_PARITY(status)  ((status) & ST_PARITY_BIT)
#define ERROR_TIMEOUT(status) ((status) & ST_TIMEOUT_BIT)
#define KBC_AUX_DATA(status)  ((status) & ST_AUX_BIT)
#define KBC_IBF_FULL(status)  ((status) & ST_IBF_BIT)
#define KBC_OBF_FULL(status)  ((status) & ST_OBF_BIT)

/*
    1 - When a key is pressed or released, the C@KBD generates the corresponding scancode and stores it temporarily in its buffer
    
    2 - If the KBC Output Buffer is empty, the C@KBD transmits the scancode at the head of its buffer to the KBC
    
    3 - The KBC then places this scancode into its Output Buffer, making it accessible
    
    4 - Finally, the KBC signals the arrival of new keyboard data to the CPU by raising interrupt IRQ1
    
    5 - After reading the scancode in the Interrupt Handler, KBC signals the C@KBD via serial bus that the Output Buffer is now empty
*/

int kbc_subscribe_int(uint8_t *bit_no);
int kbc_unsubscribe_int();

void (kbc_ih)();
uint8_t get_current_scancode();
bool check_kbc_error();

