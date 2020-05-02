#include "adrv9009_receiver.h"
#include "fft.h"
#include <thread>
#include <string>

ADRV9009Receiver::ADRV9009Receiver(long long bw, long long fs, long long flo )
{
	int i = 0;

	for(i=0;i<64;i++)
		tmpstr[i] = 'a';

	/* IIO structs required for streaming */
	ctx   = NULL;
	rx0_i = NULL;
	rx0_q = NULL;
    rx1_i = NULL;
    rx1_q = NULL;
	tx0_i = NULL;
	tx0_q = NULL;
	rxbuf = NULL;
	txbuf = NULL;

	stop = 0;
	threadActive = 0;

	// Streaming devices
	rx = NULL;

	// RX and TX sample counters
	nrx = 0;

	// Stream configurations
	// RX stream config
    trxcfg.bw_hz = bw;
    trxcfg.fs_hz = fs;
    trxcfg.lo_hz = flo;
    trxcfg.rfport = "A_BALANCED"; // port A (select for rf freq.)
}

ADRV9009Receiver::ADRV9009Receiver()
{
    int i = 0;

    for(i=0;i<64;i++)
        tmpstr[i] = 'a';

    /* IIO structs required for streaming */
    ctx   = NULL;
    rx0_i = NULL;
    rx0_q = NULL;
    rx1_i = NULL;
    rx1_q = NULL;
    tx0_i = NULL;
    tx0_q = NULL;
    rxbuf = NULL;
    txbuf = NULL;

    stop = 0;
    threadActive = 0;

    // Streaming devices
    rx = NULL;

    // RX and TX sample counters
    nrx = 0;
}

/* cleanup and exit */
void ADRV9009Receiver::shutdown(void)
{
    printf("* Destroying buffers\n");
    if (rxbuf) { iio_buffer_destroy(rxbuf); }
    if (txbuf) { iio_buffer_destroy(txbuf); }

    printf("* Disabling streaming channels\n");
    if (rx0_i) { iio_channel_disable(rx0_i); }
    if (rx0_q) { iio_channel_disable(rx0_q); }
    if (rx1_i) { iio_channel_disable(rx1_i); }
    if (rx1_q) { iio_channel_disable(rx1_q); }
    if (tx0_i) { iio_channel_disable(tx0_i); }
    if (tx0_q) { iio_channel_disable(tx0_q); }

    printf("* Destroying context\n");
    if (ctx) { iio_context_destroy(ctx); }
    exit(0);
}

void ADRV9009Receiver::setStartupParams(long long bw, long long fs, long long flo)
{
    // Stream configurations
    // RX stream config
    trxcfg.bw_hz = bw;
    trxcfg.fs_hz = fs;
    trxcfg.lo_hz = flo;
    trxcfg.rfport = "A_BALANCED"; // port A (select for rf freq.)
}

void ADRV9009Receiver::handle_sig(int sig)
{
    printf("Waiting for process to finish... Got signal %d\n", sig);
    stop = true;
}

/* check return value of attr_write function */
void ADRV9009Receiver::errchk(int v, const char* what) {
     if (v < 0) { fprintf(stderr, "Error %d writing to channel \"%s\"\nvalue may not be supported.\n", v, what); shutdown(); }
}

/* write attribute: long long int */
void ADRV9009Receiver::wr_ch_lli(struct iio_channel *chn, const char* what, long long val)
{
    errchk(iio_channel_attr_write_longlong(chn, what, val), what);
}

/* write attribute: long long int */
long long ADRV9009Receiver::rd_ch_lli(struct iio_channel *chn, const char* what)
{
    long long val;

    errchk(iio_channel_attr_read_longlong(chn, what, &val), what);

    printf("\t %s: %lld\n", what, val);
    return val;
}

#if 0
/* write attribute: string */
static void wr_ch_str(struct iio_channel *chn, const char* what, const char* str)
{
    errchk(iio_channel_attr_write(chn, what, str), what);
}
#endif

/* helper function generating channel names */
char* ADRV9009Receiver::get_ch_name_mod(const char* type, int id, char modify)
{
    snprintf(tmpstr, sizeof(tmpstr), "%s%d_%c", type, id, modify);
    return tmpstr;
}

/* helper function generating channel names */
char* ADRV9009Receiver::get_ch_name(const char* type, int id)
{
    snprintf(tmpstr, sizeof(tmpstr), "%s%d", type, id);
    return tmpstr;
}

/* returns adrv9009 phy device */
struct iio_device* ADRV9009Receiver:: get_adrv9009_phy(struct iio_context *ctx)
{
    struct iio_device *dev =  iio_context_find_device(ctx, "adrv9009-phy");
    ASSERT(dev && "No adrv9009-phy found");
    return dev;
}

/* finds adrv9009 streaming IIO devices */
bool ADRV9009Receiver::get_adrv9009_stream_dev(struct iio_context *ctx, enum iodev d, struct iio_device **dev)
{
    switch (d) {
    case TX: *dev = iio_context_find_device(ctx, "axi-adrv9009-tx-hpc"); return *dev != NULL;
    case RX: *dev = iio_context_find_device(ctx, "axi-adrv9009-rx-hpc");  return *dev != NULL;
    default: ASSERT(0); return false;
    }
}

/* finds adrv9009 streaming IIO channels */
bool ADRV9009Receiver::get_adrv9009_stream_ch(__notused struct iio_context *ctx, enum iodev d, struct iio_device *dev, int chid, char modify, struct iio_channel **chn)
{
    *chn = iio_device_find_channel(dev, modify ? get_ch_name_mod("voltage", chid, modify) : get_ch_name("voltage", chid), d == TX);
    if (!*chn)
        *chn = iio_device_find_channel(dev, modify ? get_ch_name_mod("voltage", chid, modify) : get_ch_name("voltage", chid), d == TX);
    return *chn != NULL;
}

/* finds adrv9009 phy IIO configuration channel with id chid */
bool ADRV9009Receiver::get_phy_chan(struct iio_context *ctx, enum iodev d, int chid, struct iio_channel **chn)
{
    switch (d) {
    case RX: *chn = iio_device_find_channel(get_adrv9009_phy(ctx), get_ch_name("voltage", chid), false); return *chn != NULL;
    case TX: *chn = iio_device_find_channel(get_adrv9009_phy(ctx), get_ch_name("voltage", chid), true);  return *chn != NULL;
    default: ASSERT(0); return false;
    }
}

/* finds adrv9009 local oscillator IIO configuration channels */
bool ADRV9009Receiver::get_lo_chan(struct iio_context *ctx, struct iio_channel **chn)
{
     // LO chan is always output, i.e. true
    *chn = iio_device_find_channel(get_adrv9009_phy(ctx), get_ch_name("altvoltage", 0), true); return *chn != NULL;
}

/* applies streaming configuration through IIO */
bool ADRV9009Receiver::cfg_adrv9009_streaming_ch(struct iio_context *ctx, struct stream_cfg *cfg, int chid)
{
    struct iio_channel *chn = NULL;

    // Configure phy and lo channels
    printf("* Acquiring ADRV9009 phy channel %d\n", chid);
    if (!get_phy_chan(ctx, RX, chid, &chn)) {	return false; }

    rd_ch_lli(chn, "rf_bandwidth");
    rd_ch_lli(chn, "sampling_frequency");

    // Configure LO channel
    printf("* Acquiring ADRV9009 TRX lo channel\n");
    if (!get_lo_chan(ctx, &chn)) { return false; }
    wr_ch_lli(chn, "frequency", cfg->lo_hz);
    return true;
}


/* simple configuration and streaming */
/* usage:
 * Default context, assuming local IIO devices, i.e., this script is run on ADALM-Pluto for example
 $./a.out
 * URI context, find out the uri by typing `iio_info -s` at the command line of the host PC
 $./a.out usb:x.x.x
 */
int ADRV9009Receiver::CreateReceiver(char* usbCtx, int bufferSize)
{
	printf("* Acquiring IIO context\n");

	ASSERT((ctx = iio_create_context_from_uri(usbCtx)) && "No context");
	ASSERT(iio_context_get_devices_count(ctx) > 0 && "No devices");

    printf("* Acquiring ADRV9009 streaming devices\n");
    ASSERT(get_adrv9009_stream_dev(ctx, RX, &rx) && "No rx dev found");

    printf("* Configuring ADRV9009 for streaming\n");
    ASSERT(cfg_adrv9009_streaming_ch(ctx, &trxcfg, 0) && "TRX device not found");

    printf("* Initializing ADRV9009 IIO streaming channels\n");
    ASSERT(get_adrv9009_stream_ch(ctx, RX, rx, 0, 'i', &rx0_i) && "RX chan i not found");
    ASSERT(get_adrv9009_stream_ch(ctx, RX, rx, 0, 'q', &rx0_q) && "RX chan q not found");
//    ASSERT(get_adrv9009_stream_ch(ctx, RX, rx, 0, 'i', &rx1_i) && "RX chan i not found");
//    ASSERT(get_adrv9009_stream_ch(ctx, RX, rx, 0, 'q', &rx1_q) && "RX chan q not found");

    printf("* Enabling IIO streaming channels\n");
    iio_channel_enable(rx0_i);
    iio_channel_enable(rx0_q);
//    iio_channel_enable(rx1_i);
//    iio_channel_enable(rx1_i);

	printf("* Creating non-cyclic IIO buffers with 1 MiS\n");
	rxbuf = iio_device_create_buffer(rx, bufferSize, false);
	if (!rxbuf) {
		perror("Could not create RX buffer");
		shutdown();
	}

	return 0;
}

void ADRV9009Receiver::pollRXBuffer(double* data, bool isVerbose)
{
	ssize_t nbytes_rx;
	char *p_dat, *p_end;
	ptrdiff_t p_inc;
	int j = 0;

	// Refill RX buffer
	nbytes_rx = iio_buffer_refill(rxbuf);
	if (nbytes_rx < 0) { printf("Error refilling buf %d\n",(int) nbytes_rx); shutdown(); }

	// READ: Get pointers to RX buf and read IQ from RX buf port 0
	p_inc = iio_buffer_step(rxbuf);
	p_end = (char*)iio_buffer_end(rxbuf);
	for (p_dat = (char *)iio_buffer_first(rxbuf, rx0_i); p_dat < p_end; p_dat += p_inc)
	{
		// TO DO: Okunan datayi arraye alalim
		const int16_t i = ((int16_t*)p_dat)[0]; // Real (I)
		const int16_t q = ((int16_t*)p_dat)[1]; // Imag (Q)
		REAL(data,j) = i;
		IMAG(data,j) = q;

		if(isVerbose)
			printf("Sample %5d: %5d %5d\n",j,i,q);

		j++;
	}

}

