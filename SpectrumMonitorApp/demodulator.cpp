#include "demodulator.h"

Demodulator::Demodulator()
{


}


Demodulator::~Demodulator()
{
    if(f)
        pclose(f);
}

void Demodulator::run()
{
    while(1)
    {
        if(demodActive == true)
        {
            if(newDataArrived == true && demodulationDone == true)
            {
                newDataArrived = false;
                demodulationDone = false;
                demodulate(data, bufferSize);
                demodulationDone = true;
            }
        }
    }
}

void Demodulator::fillBuffer(short* newData, int newSize)
{
    if(demodulationDone == true)
    {
        data = newData;
        bufferSize = newSize;
        newDataArrived = true;
    }
}

void Demodulator::demodulate(short* data, int size)
{
    for(int i=0;i<size;i++)
    {
        fwrite (data , sizeof(short), 2, f);
        data = data + 2;
    }

}

void Demodulator::demodStart()
{

    if(currentDemodType == FM)
    {
        f = popen ("csdr convert_s16_f | csdr shift_addition_cc -0.085 | csdr fir_decimate_cc 10 0.05 HAMMING  | csdr fmdemod_quadri_cf | csdr fractional_decimator_ff 5 | csdr deemphasis_wfm_ff 48000 50e-6 | csdr convert_f_s16 | aplay -c1 -fS16_LE -r48000", "w");
    }
    else if(currentDemodType == AM)
    {

        f = popen ("csdr convert_s16_f | csdr shift_addition_cc 'python -c 'print float(145000000-144400000)/2400000'' | csdr fir_decimate_cc 50 0.005 HAMMING | csdr amdemod_cf | csdr fastdcblock_ff | csdr agc_ff | csdr limit_ff | csdr convert_f_s16 | aplay -c1 -fS16_LE -r48000", "w");
    }
    else if(currentDemodType == SSB)
    {
        f = popen ("csdr convert_s16_f | csdr shift_addition_cc 'python -c 'print float(145000000-144400000)/2400000'' | csdr fir_decimate_cc 50 0.005 HAMMING | csdr bandpass_fir_fft_cc 0 0.1 0.05 | csdr realpart_cf | csdr agc_ff | csdr limit_ff | csdr convert_f_s16  | aplay -c1 -fS16_LE -r48000", "w");

    }
    else
    {
        // Invalid demodulation type.
    }

    if (!f)
    {
        perror ("popen");
        exit(1);
    }

    demodActive = true;

}

void Demodulator::demodStop()
{
    if(f)
    {
        demodActive = false;
        pclose(f);
    }
}

void Demodulator::demodTypeChanged(demodTypes newDemodType)
{
    currentDemodType = newDemodType;

    if (f)
    {
        pclose(f);
    }

    if(currentDemodType == FM)
    {
        f = popen ("csdr convert_s16_f | csdr shift_addition_cc -0.085 | csdr fir_decimate_cc 10 0.05 HAMMING  | csdr fmdemod_quadri_cf | csdr fractional_decimator_ff 5 | csdr deemphasis_wfm_ff 48000 50e-6 | csdr convert_f_s16 | aplay -c1 -fS16_LE -r48000", "w");
    }
    else if(currentDemodType == AM)
    {
//        f = popen ("csdr convert_s16_f | csdr shift_addition_cc 'python -c 'print float(145000000-144400000)/2400000'' | csdr fir_decimate_cc 50 0.005 HAMMING | csdr amdemod_cf | csdr fastdcblock_ff | csdr agc_ff | csdr limit_ff | csdr convert_f_s16 | aplay -c1 -fS16_LE -r48000", "w");
    }
    else if(currentDemodType == SSB)
    {
//        f = popen ("csdr convert_s16_f | csdr shift_addition_cc 'python -c 'print float(145000000-144400000)/2400000'' | csdr fir_decimate_cc 50 0.005 HAMMING | csdr bandpass_fir_fft_cc 0 0.1 0.05 | csdr realpart_cf | csdr agc_ff | csdr limit_ff | csdr convert_f_s16  | aplay -c1 -fS16_LE -r48000", "w");

    }
    else
    {
        // Invalid demodulation type.
    }
}


