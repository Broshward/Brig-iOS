//
//Software SPI :)
uint32_t *NSS, *MOSI, *MISO, *SCK, DR_tx, DR_rx, *DR_tx_bit_band, *DR_rx_bit_band, *DR_max;
char CPOL;

void SPI_initialization();
char SPI_rw(char DR_tx);
void _SPI_rw(void);

