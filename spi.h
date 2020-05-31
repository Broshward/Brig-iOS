uint32_t *NSS; // Bit banding address for NSS pin
char spi_buf[256];

void SPI_initialization();
void USART_as_SPI_initializathion();

char* SPI_rw(char* spi_buf, uint8_t command_len, uint8_t data_len);
char* J_ID_read(char *buf);
char* RDID(char *buf);
char* RDMDID(char *buf);
void WREN();
uint8_t RDSR();
void WRSR(char value);
char* READ(uint32_t addr, char *buf, uint8_t count);
void PAGE_PROG(uint32_t addr, char *buf, uint8_t count);
void SECTOR_ER(uint32_t addr);
void BLOCK_ER(uint32_t addr);
void CHIP_ER(uint32_t addr);
uint32_t search_end_of_data(uint32_t first, uint32_t last, uint32_t data_size);

