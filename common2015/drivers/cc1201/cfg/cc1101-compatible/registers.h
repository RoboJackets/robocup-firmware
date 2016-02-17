
static const registerSetting_t preferredSettings[]= 
{
  {CC1201_IOCFG3,              0x06},
  {CC1201_IOCFG2,              0x06},
  {CC1201_IOCFG1,              0x30},
  {CC1201_IOCFG0,              0x3C},
  {CC1201_SYNC3,               0xD3},
  {CC1201_SYNC2,               0x91},
  {CC1201_SYNC1,               0xD3},
  {CC1201_SYNC0,               0x91},
  {CC1201_SYNC_CFG1,           0xA8},
  {CC1201_SYNC_CFG0,           0x03},
  {CC1201_DEVIATION_M,         0x9A},
  {CC1201_MODCFG_DEV_E,        0x0D},
  {CC1201_DCFILT_CFG,          0x5D},
  {CC1201_PREAMBLE_CFG1,       0x14},
  {CC1201_PREAMBLE_CFG0,       0x8A},
  {CC1201_IQIC,                0x4B},
  {CC1201_CHAN_BW,             0x03},
  {CC1201_MDMCFG1,             0x40},
  {CC1201_MDMCFG0,             0x0D},
  {CC1201_SYMBOL_RATE2,        0xB9},
  {CC1201_SYMBOL_RATE1,        0x99},
  {CC1201_SYMBOL_RATE0,        0x9A},
  {CC1201_AGC_REF,             0x40},
  {CC1201_AGC_CS_THR,          0xEC},
  {CC1201_AGC_GAIN_ADJUST,     0x00},
  {CC1201_AGC_CFG3,            0xB1},
  {CC1201_AGC_CFG2,            0x20},
  {CC1201_AGC_CFG1,            0x51},
  {CC1201_AGC_CFG0,            0xC7},
  {CC1201_FIFO_CFG,            0x00},
  {CC1201_DEV_ADDR,            0x00},
  {CC1201_SETTLING_CFG,        0x0B},
  {CC1201_FS_CFG,              0x12},
  {CC1201_WOR_CFG1,            0x08},
  {CC1201_WOR_CFG0,            0x21},
  {CC1201_WOR_EVENT0_MSB,      0x00},
  {CC1201_WOR_EVENT0_LSB,      0x00},
  {CC1201_RXDCM_TIME,          0x00},
  {CC1201_PKT_CFG2,            0x00},
  {CC1201_PKT_CFG1,            0x03},
  {CC1201_PKT_CFG0,            0x20},
  {CC1201_RFEND_CFG1,          0x3F},
  {CC1201_RFEND_CFG0,          0x30},
  {CC1201_PA_CFG1,             0x3F},
  {CC1201_PA_CFG0,             0x51},
  {CC1201_ASK_CFG,             0x0F},
  {CC1201_PKT_LEN,             0xFF},
  {CC1201_IF_MIX_CFG,          0x1C},
  {CC1201_FREQOFF_CFG,         0x22},
  {CC1201_TOC_CFG,             0x0B},
  {CC1201_MARC_SPARE,          0x00},
  {CC1201_ECG_CFG,             0x00},
  {CC1201_MDMCFG2,             0x02},
  {CC1201_EXT_CTRL,            0x01},
  {CC1201_RCCAL_FINE,          0x00},
  {CC1201_RCCAL_COARSE,        0x00},
  {CC1201_RCCAL_OFFSET,        0x00},
  {CC1201_FREQOFF1,            0x00},
  {CC1201_FREQOFF0,            0x00},
  {CC1201_FREQ2,               0x5A},
  {CC1201_FREQ1,               0x66},
  {CC1201_FREQ0,               0x66},
  {CC1201_IF_ADC2,             0x02},
  {CC1201_IF_ADC1,             0xEE},
  {CC1201_IF_ADC0,             0x10},
  {CC1201_FS_DIG1,             0x07},
  {CC1201_FS_DIG0,             0xAA},
  {CC1201_FS_CAL3,             0x00},
  {CC1201_FS_CAL2,             0x20},
  {CC1201_FS_CAL1,             0x40},
  {CC1201_FS_CAL0,             0x0E},
  {CC1201_FS_CHP,              0x28},
  {CC1201_FS_DIVTWO,           0x03},
  {CC1201_FS_DSM1,             0x00},
  {CC1201_FS_DSM0,             0x33},
  {CC1201_FS_DVC1,             0xFF},
  {CC1201_FS_DVC0,             0x17},
  {CC1201_FS_LBI,              0x00},
  {CC1201_FS_PFD,              0x00},
  {CC1201_FS_PRE,              0x6E},
  {CC1201_FS_REG_DIV_CML,      0x1C},
  {CC1201_FS_SPARE,            0xAC},
  {CC1201_FS_VCO4,             0x14},
  {CC1201_FS_VCO3,             0x00},
  {CC1201_FS_VCO2,             0x00},
  {CC1201_FS_VCO1,             0x00},
  {CC1201_FS_VCO0,             0xB5},
  {CC1201_GBIAS6,              0x00},
  {CC1201_GBIAS5,              0x02},
  {CC1201_GBIAS4,              0x00},
  {CC1201_GBIAS3,              0x00},
  {CC1201_GBIAS2,              0x10},
  {CC1201_GBIAS1,              0x00},
  {CC1201_GBIAS0,              0x00},
  {CC1201_IFAMP,               0x05},
  {CC1201_LNA,                 0x01},
  {CC1201_RXMIX,               0x01},
  {CC1201_XOSC5,               0x0E},
  {CC1201_XOSC4,               0xA0},
  {CC1201_XOSC3,               0x03},
  {CC1201_XOSC2,               0x04},
  {CC1201_XOSC1,               0x03},
  {CC1201_XOSC0,               0x00},
  {CC1201_ANALOG_SPARE,        0x00},
  {CC1201_PA_CFG3,             0x00},
  {CC1201_WOR_TIME1,           0x00},
  {CC1201_WOR_TIME0,           0x00},
  {CC1201_WOR_CAPTURE1,        0x00},
  {CC1201_WOR_CAPTURE0,        0x00},
  {CC1201_BIST,                0x00},
  {CC1201_DCFILTOFFSET_I1,     0x00},
  {CC1201_DCFILTOFFSET_I0,     0x00},
  {CC1201_DCFILTOFFSET_Q1,     0x00},
  {CC1201_DCFILTOFFSET_Q0,     0x00},
  {CC1201_IQIE_I1,             0x00},
  {CC1201_IQIE_I0,             0x00},
  {CC1201_IQIE_Q1,             0x00},
  {CC1201_IQIE_Q0,             0x00},
  {CC1201_RSSI1,               0x80},
  {CC1201_RSSI0,               0x00},
  {CC1201_MARCSTATE,           0x41},
  {CC1201_LQI_VAL,             0x00},
  {CC1201_PQT_SYNC_ERR,        0xFF},
  {CC1201_DEM_STATUS,          0x00},
  {CC1201_FREQOFF_EST1,        0x00},
  {CC1201_FREQOFF_EST0,        0x00},
  {CC1201_AGC_GAIN3,           0x00},
  {CC1201_AGC_GAIN2,           0xD1},
  {CC1201_AGC_GAIN1,           0x00},
  {CC1201_AGC_GAIN0,           0x3F},
  {CC1201_CFM_RX_DATA_OUT,     0x00},
  {CC1201_CFM_TX_DATA_IN,      0x00},
  {CC1201_ASK_SOFT_RX_DATA,    0x30},
  {CC1201_RNDGEN,              0x7F},
  {CC1201_MAGN2,               0x00},
  {CC1201_MAGN1,               0x00},
  {CC1201_MAGN0,               0x00},
  {CC1201_ANG1,                0x00},
  {CC1201_ANG0,                0x00},
  {CC1201_CHFILT_I2,           0x02},
  {CC1201_CHFILT_I1,           0x00},
  {CC1201_CHFILT_I0,           0x00},
  {CC1201_CHFILT_Q2,           0x00},
  {CC1201_CHFILT_Q1,           0x00},
  {CC1201_CHFILT_Q0,           0x00},
  {CC1201_GPIO_STATUS,         0x00},
  {CC1201_FSCAL_CTRL,          0x01},
  {CC1201_PHASE_ADJUST,        0x00},
  {CC1201_PARTNUMBER,          0x00},
  {CC1201_PARTVERSION,         0x00},
  {CC1201_SERIAL_STATUS,       0x00},
  {CC1201_MODEM_STATUS1,       0x01},
  {CC1201_MODEM_STATUS0,       0x00},
  {CC1201_MARC_STATUS1,        0x00},
  {CC1201_MARC_STATUS0,        0x00},
  {CC1201_PA_IFAMP_TEST,       0x00},
  {CC1201_FSRF_TEST,           0x00},
  {CC1201_PRE_TEST,            0x00},
  {CC1201_PRE_OVR,             0x00},
  {CC1201_ADC_TEST,            0x00},
  {CC1201_DVC_TEST,            0x0B},
  {CC1201_ATEST,               0x40},
  {CC1201_ATEST_LVDS,          0x00},
  {CC1201_ATEST_MODE,          0x00},
  {CC1201_XOSC_TEST1,          0x3C},
  {CC1201_XOSC_TEST0,          0x00},
  {CC1201_AES,                 0x00},
  {CC1201_MDM_TEST,            0x00},
  {CC1201_RXFIRST,             0x00},
  {CC1201_TXFIRST,             0x00},
  {CC1201_RXLAST,              0x00},
  {CC1201_TXLAST,              0x00},
  {CC1201_NUM_TXBYTES,         0x00},
  {CC1201_NUM_RXBYTES,         0x00},
  {CC1201_FIFO_NUM_TXBYTES,    0x0F},
  {CC1201_FIFO_NUM_RXBYTES,    0x00},
  {CC1201_RXFIFO_PRE_BUF,      0x00},
};