#include "fft.h"
using namespace std;

FFTProcessor::FFTProcessor(void)
{

}

int FFTProcessor::fft(short* inputSamples, double* returnSamples, int nSize)
{
    int i = 0;
    ogInputSamples = (double*)malloc(2*nSize*sizeof(double));

    for(i=0;i<nSize;i++)
    {
        REAL(ogInputSamples,i) = (double)REAL(inputSamples,i);
        IMAG(ogInputSamples,i) = (double)IMAG(inputSamples,i);
    }

    gsl_fft_complex_radix2_forward (ogInputSamples, 1, nSize);

    for(i=0;i<nSize;i++)
    {
        REAL(returnSamples,i) = (double)REAL(ogInputSamples,i);
        IMAG(returnSamples,i) = (double)IMAG(ogInputSamples,i);
    }


    free(ogInputSamples);

	return 0;
}

//int main (void)
//{
//  int i; double data[2*128];
//
//  for (i = 0; i < 128; i++)
//    {
//       REAL(data,i) = fft_test_sig[i];
//       IMAG(data,i) = 0.0;
//    }
//
//
//  for (i = 0; i < 128; i++)
//    {
//      printf ("%d %e %e\n", i,
//              REAL(data,i), IMAG(data,i));
//    }
//  printf ("\n\n");
//
//  gsl_fft_complex_radix2_forward (data, 1, 128);
//
//  for (i = 0; i < 128; i++)
//    {
//      printf ("%d %e %e\n", i,
//              REAL(data,i),
//              IMAG(data,i));
//    }
//
//  return 0;
//}

