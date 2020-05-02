#include "fft.h"
using namespace std;

FFTProcessor::FFTProcessor(void)
{

}

int FFTProcessor::fft(double* inputSamples, int nSize)
{
	gsl_fft_complex_radix2_forward (inputSamples, 1, nSize);

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

