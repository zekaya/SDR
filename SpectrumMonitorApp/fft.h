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
public:
	FFTProcessor(void);
	int fft(double* inputSamples, int nSize);
};



#endif /* FFT_H_ */
