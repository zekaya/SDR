#ifndef FFT_H_
#define FFT_H_

#include <stdio.h>
#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_fft_complex.h>

#define REAL(z,i) ((z)[2*(i)])
#define IMAG(z,i) ((z)[2*(i)+1])

class FFTProcessor
{

private:
    double* ogInputSamples;

public:
	FFTProcessor(void);
    int fft(short* inputSamples, double* returnSamples, int nSize);
};

#endif /* FFT_H_ */
