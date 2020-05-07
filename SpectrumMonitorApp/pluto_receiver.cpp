#include "pluto_receiver.h"
#include "fft.h"
#include <thread>
#include <string>

PlutoReceiver::PlutoReceiver(long long bw, long long fs, long long flo )
{
	int i = 0;

	for(i=0;i<64;i++)
		tmpstr[i] = 'a';

	/* IIO structs required for streaming */
	ctx   = NULL;
	rx0_i = NULL;
	rx0_q = NULL;
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
	rxcfg.bw_hz = bw;
	rxcfg.fs_hz = fs;
	rxcfg.lo_hz = flo;
	rxcfg.rfport = "A_BALANCED"; // port A (select for rf freq.)
}

PlutoReceiver::PlutoReceiver()
{
    int i = 0;

    for(i=0;i<64;i++)
        tmpstr[i] = 'a';

    /* IIO structs required for streaming */
    ctx   = NULL;
    rx0_i = NULL;
    rx0_q = NULL;
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

void PlutoReceiver::setStartupParams(long long bw, long long fs, long long flo)
{
    // Stream configurations
    // RX stream config
    rxcfg.bw_hz = bw;
    rxcfg.fs_hz = fs;
    rxcfg.lo_hz = flo;
    rxcfg.rfport = "A_BALANCED"; // port A (select for rf freq.)
}

void PlutoReceiver::updateParams(long long bw, long long fs, long long flo)
{
    // Stream configurations
    // RX stream config
    rxcfg.bw_hz = bw;
    rxcfg.fs_hz = fs;
    rxcfg.lo_hz = flo;
    rxcfg.rfport = "A_BALANCED"; // port A (select for rf freq.)

    ASSERT(cfg_ad9361_streaming_ch(ctx, &rxcfg, RX, 0) && "RX port 0 not found");
}

/* cleanup and exit */
void PlutoReceiver::shutdown()
{
	printf("* Destroying buffers\n");
	if (rxbuf) { iio_buffer_destroy(rxbuf); }
	if (txbuf) { iio_buffer_destroy(txbuf); }

	printf("* Disabling streaming channels\n");
	if (rx0_i) { iio_channel_disable(rx0_i); }
	if (rx0_q) { iio_channel_disable(rx0_q); }
	if (tx0_i) { iio_channel_disable(tx0_i); }
	if (tx0_q) { iio_channel_disable(tx0_q); }

	printf("* Destroying context\n");
	if (ctx) { iio_context_destroy(ctx); }

//    exit(0);
}

/* check return value of attr_write function */
void PlutoReceiver::errchk(int v, const char* what) {
	 if (v < 0) { fprintf(stderr, "Error %d writing to channel \"%s\"\nvalue may not be supported.\n", v, what); shutdown(); }
}

/* write attribute: long long int */
void PlutoReceiver::wr_ch_lli(struct iio_channel *chn, const char* what, long long val)
{
	errchk(iio_channel_attr_write_longlong(chn, what, val), what);
}

/* write attribute: string */
void PlutoReceiver::wr_ch_str(struct iio_channel *chn, const char* what, const char* str)
{
	errchk(iio_channel_attr_write(chn, what, str), what);
}

/* helper function generating channel names */
char* PlutoReceiver::get_ch_name(const char* type, int id)
{
	snprintf(tmpstr, sizeof(tmpstr), "%s%d", type, id);
	return tmpstr;
}

/* returns ad9361 phy device */
struct iio_device* PlutoReceiver::get_ad9361_phy(struct iio_context *ctx)
{
	struct iio_device *dev =  iio_context_find_device(ctx, "ad9361-phy");
	ASSERT(dev && "No ad9361-phy found");
	return dev;
}

/* finds AD9361 streaming IIO devices */
bool PlutoReceiver::get_ad9361_stream_dev(struct iio_context *ctx, enum iodev d, struct iio_device **dev)
{
	switch (d) {
	case TX: *dev = iio_context_find_device(ctx, "cf-ad9361-dds-core-lpc"); return *dev != NULL;
	case RX: *dev = iio_context_find_device(ctx, "cf-ad9361-lpc");  return *dev != NULL;
	default: ASSERT(0); return false;
	}
}

/* finds AD9361 streaming IIO channels */
bool PlutoReceiver::get_ad9361_stream_ch(__notused struct iio_context *ctx, enum iodev d, struct iio_device *dev, int chid, struct iio_channel **chn)
{
	*chn = iio_device_find_channel(dev, get_ch_name("voltage", chid), d == TX);
	if (!*chn)
		*chn = iio_device_find_channel(dev, get_ch_name("altvoltage", chid), d == TX);
	return *chn != NULL;
}

/* finds AD9361 phy IIO configuration channel with id chid */
bool PlutoReceiver::get_phy_chan(struct iio_context *ctx, enum iodev d, int chid, struct iio_channel **chn)
{
	switch (d) {
	case RX: *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("voltage", chid), false); return *chn != NULL;
	case TX: *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("voltage", chid), true);  return *chn != NULL;
	default: ASSERT(0); return false;
	}
}

/* finds AD9361 local oscillator IIO configuration channels */
bool PlutoReceiver::get_lo_chan(struct iio_context *ctx, enum iodev d, struct iio_channel **chn)
{
	switch (d) {
	 // LO chan is always output, i.e. true
	case RX: *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("altvoltage", 0), true); return *chn != NULL;
	case TX: *chn = iio_device_find_channel(get_ad9361_phy(ctx), get_ch_name("altvoltage", 1), true); return *chn != NULL;
	default: ASSERT(0); return false;
	}
}

/* applies streaming configuration through IIO */
bool PlutoReceiver::cfg_ad9361_streaming_ch(struct iio_context *ctx, struct stream_cfg *cfg, enum iodev type, int chid)
{
	struct iio_channel *chn = NULL;

	// Configure phy and lo channels
	printf("* Acquiring AD9361 phy channel %d\n", chid);
	if (!get_phy_chan(ctx, type, chid, &chn)) {	return false; }
	wr_ch_str(chn, "rf_port_select",     cfg->rfport);
	wr_ch_lli(chn, "rf_bandwidth",       cfg->bw_hz);
	wr_ch_lli(chn, "sampling_frequency", cfg->fs_hz);

	// Configure LO channel
	printf("* Acquiring AD9361 %s lo channel\n", type == TX ? "TX" : "RX");
	if (!get_lo_chan(ctx, type, &chn)) { return false; }
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
int PlutoReceiver::CreateReceiver(char* usbCtx, int bufferSize)
{
	printf("* Acquiring IIO context\n");

	ASSERT((ctx = iio_create_context_from_uri(usbCtx)) && "No context");
	ASSERT(iio_context_get_devices_count(ctx) > 0 && "No devices");

	printf("* Acquiring AD9361 streaming devices\n");
	ASSERT(get_ad9361_stream_dev(ctx, RX, &rx) && "No rx dev found");

	printf("* Configuring AD9361 for streaming\n");
	ASSERT(cfg_ad9361_streaming_ch(ctx, &rxcfg, RX, 0) && "RX port 0 not found");

	printf("* Initializing AD9361 IIO streaming channels\n");
	ASSERT(get_ad9361_stream_ch(ctx, RX, rx, 0, &rx0_i) && "RX chan i not found");
	ASSERT(get_ad9361_stream_ch(ctx, RX, rx, 1, &rx0_q) && "RX chan q not found");

	printf("* Enabling IIO streaming channels\n");
	iio_channel_enable(rx0_i);
	iio_channel_enable(rx0_q);

	rxbuf = iio_device_create_buffer(rx, bufferSize, false);

	if (!rxbuf) {
		perror("Could not create RX buffer");
		shutdown();
	}

	return 0;
}

void PlutoReceiver::pollRXBuffer(short* data)
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

        j++;
	}

}

void PlutoReceiver::updateBufferSize(int bufSize)
{
    iio_buffer_destroy(rxbuf);

    rxbuf = iio_device_create_buffer(rx, bufSize, false);

    if (!rxbuf) {
        perror("Could not update RX buffer size!");
        shutdown();
    }
}


