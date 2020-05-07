#ifndef PLUTO_RECEIVER_H_
#define PLUTO_RECEIVER_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <iio.h>

/* helper macros */
#define KHZ(x) ((long long)(x*1000.0 + .5))
#define MHZ(x) ((long long)(x*1000000.0 + .5))
#define GHZ(x) ((long long)(x*1000000000.0 + .5))

#define ASSERT(expr) { \
	if (!(expr)) { \
		(void) fprintf(stderr, "assertion failed (%s:%d)\n", __FILE__, __LINE__); \
		(void) abort(); \
	} \
}

/* RX is input, TX is output */
enum iodev { RX, TX };

class PlutoReceiver
{
private:
	/* common RX and TX streaming params */
	struct stream_cfg {
		long long bw_hz; // Analog banwidth in Hz
		long long fs_hz; // Baseband sample rate in Hz
		long long lo_hz; // Local oscillator frequency in Hz
		const char* rfport; // Port name
	};

	/* static scratch mem for strings */
	char tmpstr[64];

	/* IIO structs required for streaming */
	struct iio_context *ctx   = NULL;
	struct iio_channel *rx0_i = NULL;
	struct iio_channel *rx0_q = NULL;
	struct iio_channel *tx0_i = NULL;
	struct iio_channel *tx0_q = NULL;
	struct iio_buffer  *rxbuf = NULL;
	struct iio_buffer  *txbuf = NULL;

	bool stop;
	bool threadActive = 0;

	// Streaming devices
	struct iio_device *rx;

	// RX and TX sample counters
	size_t nrx = 0;

	// Stream configurations
	struct stream_cfg rxcfg;

public:
    PlutoReceiver();
	PlutoReceiver(long long bw, long long fs, long long flo );
    void setStartupParams(long long bw, long long fs, long long flo );
    void updateParams(long long bw, long long fs, long long flo );
	void shutdown();
	void errchk(int v, const char* what);
	void wr_ch_lli(struct iio_channel *chn, const char* what, long long val);
	void wr_ch_str(struct iio_channel *chn, const char* what, const char* str);
	char* get_ch_name(const char* type, int id);
	struct iio_device* get_ad9361_phy(struct iio_context *ctx);
	bool get_ad9361_stream_dev(struct iio_context *ctx, enum iodev d, struct iio_device **dev);
	bool get_ad9361_stream_ch(__notused struct iio_context *ctx, enum iodev d, struct iio_device *dev, int chid, struct iio_channel **chn);
	bool get_phy_chan(struct iio_context *ctx, enum iodev d, int chid, struct iio_channel **chn);
	bool get_lo_chan(struct iio_context *ctx, enum iodev d, struct iio_channel **chn);
	bool cfg_ad9361_streaming_ch(struct iio_context *ctx, struct stream_cfg *cfg, enum iodev type, int chid);
	int CreateReceiver(char* usbCtx, int bufferSize);
    void pollRXBuffer(short* data);
    void updateBufferSize(int bufSize);
};

#endif /* PLUTO_RECEIVER_H_ */
