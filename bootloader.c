#define LED 19

#define _REG32(p, i) (*(volatile unsigned int *) ((p) + (i)))

#define GPIO_CTRL_ADDR 0x10012000UL
#define GPIO_IOF_EN     (0x38)
#define GPIO_IOF_SEL    (0x3C)
#define GPIO_REG(offset) _REG32(GPIO_CTRL_ADDR, offset)


#define UART0_CTRL_ADDR 0x10013000UL
#define UART_REG_TXFIFO 0x00
#define UART_REG_RXFIFO 0x04
#define UART_REG_TXCTRL 0x08
#define UART_REG_RXCTRL 0x0c
#define UART_REG_DIV    0x18
#define IOF0_UART0_MASK 0x00030000UL
#define UART_TXEN 0x1
#define UART_RXEN 0x1
#define UART0_REG(offset) _REG32(UART0_CTRL_ADDR, offset)

void init() {
    GPIO_REG(GPIO_IOF_SEL) &= ~IOF0_UART0_MASK;
    GPIO_REG(GPIO_IOF_EN) |= IOF0_UART0_MASK;
    UART0_REG(UART_REG_DIV) = 1000; // 57600bps
    UART0_REG(UART_REG_TXCTRL) |= UART_TXEN;
    UART0_REG(UART_REG_RXCTRL) |= UART_RXEN;
}

char getchar() {
    unsigned int rxFifo;
    while (1) {
        rxFifo = UART0_REG(UART_REG_RXFIFO);
        if ((rxFifo & 0x80000000) == 0) return rxFifo & 0xFF;
    }
}

void putchar(char c) {
    while (UART0_REG(UART_REG_TXFIFO) & 0x80000000);
    UART0_REG(UART_REG_TXFIFO) = c;
}


void puts(char *s) {
    while (*s != '\0') {
        putchar(*s++);
    }
}

int gets(char *s) {
    int cnt = 0;
    while (1) {
        *s = getchar();
        if (*s == '\n') break;
        s++;
        cnt++;
    }
    s++;
    *s = '\0';
    return cnt;
}

typedef struct {
    unsigned int addr;
    unsigned int size;
    unsigned int type;
    unsigned char* data;
    unsigned char* mem;
} recbuf;

static unsigned int h2i(int len, unsigned char* h) {
    unsigned int tmp = 0;
    unsigned char c;

    for (int i = 0; i < len; i++) {
        c = h[i];
        if (c >= 'A') c = c - 'A' + 10;
        else          c -= '0';
        tmp = (tmp << 4) | c;
    }
    return tmp;
}

void decode_srec(unsigned char* r, recbuf* rb) {
    int alen;

    rb->type = 3;
    alen = 0;
    if ((r[1] > '0') && (r[1] < '4')){ // data
      alen = 2 * (h2i(1, &r[1]) + 1);
      rb->type = 1;
    } else if ((r[1] >= '7') && (r[1] <= '9')){ // termination
      alen = 2 * (11 - h2i(1, &r[1]));
      rb->type = 2;
    } else if (r[1] == '0'){ // header
      alen = 4;
      rb->type = 0;
    }
    rb->addr = h2i(alen, &r[4]);
    rb->size = h2i(2, &r[2]) * 2 - alen - 2;
    rb->data = (unsigned char*) &r[4 + alen];
}

void extract_srec(recbuf *rb){
    void (*prog) ();

    switch (rb->type) {
        case 0: // header, do nothing
            break;
        case 1: // data, copy to specified location
            for (int i = 0; i < (rb->size / 2); i++) {
                ((unsigned char*)rb->addr)[i] = (unsigned char)h2i(2, &rb->data[2 * i]);
            }
            break;
        case 2:
            prog = (void*)rb->addr;
            puts("Starting Program...\n");
            prog();
            break;
    }
}

void main() {
    unsigned char buf[256];
    recbuf srec;
    init();
    puts("===YARC S-Record Loader===\nReady...\n");
    do {
        gets(buf);
        switch (buf[0]) {
            case 'S':
            case 's':
                decode_srec(buf, &srec);
                extract_srec(&srec);
                break;
        }
    } while (1);
}
