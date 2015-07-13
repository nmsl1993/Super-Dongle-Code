
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#define SAMPLE_SIZE 640000
#define SAMPLE_RATE 256000.0f
#define PINGER_FREQ 37500.0f
#define BUFFER_SIZE 64
#define CUAUV_PI 3.14159265358979323846

uint16_t samples_0_0[SAMPLE_SIZE];
uint16_t samples_0_1[SAMPLE_SIZE];
uint16_t samples_1_0[SAMPLE_SIZE];

typedef struct iterating_dtft
{
    float cosTerm;
    float sinTerm;
    float cosNormSum;
    float sinNormSum;
    float phase;
    float avg_phase;
    float sinBuffer[BUFFER_SIZE];
    float cosBuffer[BUFFER_SIZE];
    float sinNormBuffer[BUFFER_SIZE];
    float cosNormBuffer[BUFFER_SIZE];
    float phase_var;
    unsigned int ring_buffer_head; 
} dtft_t;
typedef struct iterating_nco
{
    float cosTerm;
    float sinTerm;
    float phase;
    float nco_step;
    float nco_value;

} nco_t;

void populate_samples_array(uint16_t* , char* );
void init_nco(float freq, nco_t* );
void init_dtft(dtft_t*);
void update_dtft(uint16_t, nco_t*, dtft_t*);
void update_nco(nco_t*);
float phase_difference(float,float);
void printArr(uint16_t * arr, size_t len);
int main(int argc, char * argv[])
{
//filename = argv[1]
    if(argc != 4)
    {
        printf("In order to use this program you must enter three arguments, the path to sample_0_0, the  path to sample_0_1, and the path sample_1_0\n");
        exit(1);
    }
    else
    {
        
        printf("Loading %s ...\n",argv[1]);
        populate_samples_array(samples_0_0,argv[1]);
        
        /*
        long t = 0;
        for(int i = 0; i<SAMPLE_SIZE; i++)
        {
            t += samples_0_0[i];
        }
        printf("sum is: %li\n", t);
        */
        printf("Loading %s ...\n",argv[2]);
        populate_samples_array(samples_0_1,argv[2]);
        printf("Loading %s ...\n",argv[3]);
        populate_samples_array(samples_1_0,argv[3]);
        //long tic = 
        //printArr(samples_0_1,SAMPLE_SIZE);
        printf("Processing data...\n");

        nco_t nco;
        dtft_t dtft_0_0;
        dtft_t dtft_0_1;
        dtft_t dtft_1_0;

        
        init_nco(PINGER_FREQ,&nco);
        init_dtft(&dtft_0_0);
        init_dtft(&dtft_0_1);
        init_dtft(&dtft_1_0);
        float max_sample = -1;
        int max_i = 0;
        
        float var_min = 1e30f;
        float var_max = 0.0f;
        float delta_phase_buffer[32];
        int i;
        for(i = 0; i < SAMPLE_SIZE && 1; i++)
        {
            update_dtft(samples_0_0[i],&nco,&dtft_0_0);
            update_dtft(samples_0_1[i],&nco,&dtft_0_1);
            update_dtft(samples_1_0[i],&nco,&dtft_1_0);
             
            if(max_sample < samples_0_0[i])
            {
            max_i = i;
            max_sample = samples_0_0[i];
            }
            update_nco(&nco);
            float delta_phase_y = phase_difference(dtft_0_1.phase, dtft_0_0.phase);
            float delta_phase_x = phase_difference(dtft_1_0.phase, dtft_0_0.phase);
            
            float heading = atan2(delta_phase_y,delta_phase_x);
            

            float delta_phase_x_var = 0 ;
            delta_phase_buffer[i%32] = delta_phase_x;
            
            float delta_phase_x_mean = 0;
            int t;
            for(t = 0; t < 32; t++)
            {
                delta_phase_x_mean += delta_phase_buffer[t];
            }
            delta_phase_x_mean /=32;
            for(t = 0; t < 32; t++)
            {
            delta_phase_x_var += pow(phase_difference(delta_phase_buffer[t],delta_phase_x_mean),2); 
            }
            
            delta_phase_x_var /= 32;

            var_min = var_min > delta_phase_x_var ? delta_phase_x_var : var_min;
            
            var_max = var_max < delta_phase_x_var ? delta_phase_x_var : var_max;
            //if(dtft_0_0.phase_var < 0.05)
            //if(i > 128 && delta_phase_x_var < 0.0002)
            
            //if(i > 201800 && i < 202200)
            if(i>50 && delta_phase_x_var < .00002)
            {
                printf("low phase @ varuabce %i: %f\n", i, delta_phase_x_var);
             
            printf("\nindex is = %i. phase var=%f, avg_phase=%f\n",i,dtft_0_0.phase_var,dtft_0_0.avg_phase);
            printf("DELTA PHASE X VAR %f\n",delta_phase_x_var);
            printf("nco value: %f, nco cos %f\n",nco.nco_value, nco.cosTerm);
            printf("Delta phy %f Delta phx %f\n",delta_phase_y, delta_phase_x);
            printf("phase_0_0 %f, phase_0_1 %f, phase_1_0 %f, heading (rads): %f, heading (degs) %f\n",dtft_0_0.phase,dtft_0_1.phase,dtft_1_0.phase,heading,heading*180/CUAUV_PI);
            
            }
        }
        printf("\n\nmax of %f @ %i\n",max_sample,max_i);
        printf("varmin = %.16f, varmax=%.16f\n",var_min,var_max);
        return 0;
    }
}

void init_nco(float freq,nco_t* nco)
{
    float pinger_period = 1.0/freq;
    float sample_period = 1.0/SAMPLE_RATE;
    float samples_per_cycle = pinger_period/sample_period;

    nco->nco_step = 2.0*CUAUV_PI/samples_per_cycle;
    nco->nco_value = 0; 
    nco->cosTerm = cos(nco->nco_value); // cos(0)=1
    nco->sinTerm = sin(nco->nco_value); // sin(0)=0
    nco->phase = atan2(nco->sinTerm,nco->cosTerm);
}
void init_dtft(dtft_t* dtft)
{
    dtft->ring_buffer_head=0;
    dtft->cosTerm = 0;
    dtft->sinTerm = 0;
    dtft->cosNormSum = 0;
    dtft->sinNormSum = 0;
    dtft->phase_var=0;
    memset(&dtft->cosBuffer,0,sizeof(dtft->cosBuffer));
    memset(&dtft->sinBuffer,0,sizeof(dtft->sinBuffer));
    memset(&dtft->cosNormBuffer,0,sizeof(dtft->cosNormBuffer));
    memset(&dtft->sinNormBuffer,0,sizeof(dtft->sinNormBuffer));

}
void zero_bufffer(float * buf, unsigned int len)
{
    unsigned int c;
    for(c = 0; c < len; c++)
    {
        buf[c] = 0.0f;
    }
}
void update_nco(nco_t* nco)
{
    nco->nco_value += nco->nco_step;
    nco->cosTerm = cos(nco->nco_value);
    nco->sinTerm = sin(nco->nco_value);
    nco->phase = atan2(nco->sinTerm,nco->cosTerm);
}

//This method particularly stolen from SHARC dsp.c
void update_dtft(uint16_t sample, nco_t * nco ,dtft_t* dtft)
{
    float cosProduct = sample * nco->cosTerm;
    float sinProduct = sample * nco->sinTerm;
    
    //Sliding integral
    dtft->cosTerm -= dtft->cosBuffer[dtft->ring_buffer_head];
    dtft->sinTerm -= dtft->sinBuffer[dtft->ring_buffer_head];
    
    dtft->cosTerm += cosProduct;
    dtft->sinTerm += sinProduct;

    dtft->phase = atan2(dtft->sinTerm,dtft->cosTerm);
    

    

    dtft->cosBuffer[dtft->ring_buffer_head] = cosProduct;
    dtft->sinBuffer[dtft->ring_buffer_head] = sinProduct;



    float sample_magnitude = sqrt(pow(cosProduct,2) + pow(sinProduct,2));
    
    float cosNorm = cosProduct/sample_magnitude;
    float sinNorm = sinProduct/sample_magnitude;
    
    dtft->cosNormSum -= dtft->cosNormBuffer[dtft->ring_buffer_head];
    dtft->sinNormSum -= dtft->sinNormBuffer[dtft->ring_buffer_head];

    dtft->cosNormSum += cosNorm;
    dtft->sinNormSum += sinNorm;


    dtft->cosNormBuffer[dtft->ring_buffer_head] = cosNorm;
    dtft->sinNormBuffer[dtft->ring_buffer_head] = sinNorm;


    float CS = 0;
    float SS = 0;
    unsigned int r;
    for(r = 0; r < BUFFER_SIZE; r++)
    {   
        CS += dtft->cosNormBuffer[r];
        SS += dtft->sinNormBuffer[r];
    }
    dtft->avg_phase = atan2(SS,CS); 
    //atan2(dtft->sinNormSum,dtft->cosNormSum);
   

    //dtft->phase_var = pow(phase_difference(atan2(sinNorm,cosNorm),dtft->avg_phase),2);

    dtft->phase_var = pow(atan2(sinNorm,cosNorm) -dtft->avg_phase,2);

     
    dtft->ring_buffer_head++;
    if(dtft->ring_buffer_head >= BUFFER_SIZE)
    {
        dtft->ring_buffer_head = 0;
    }


}
float phase_difference(float phase_a, float phase_b)
{
    float diff = phase_a - phase_b;
    while(diff > CUAUV_PI)
    {
        diff -= 2*CUAUV_PI;
    }
    while(diff < -1*CUAUV_PI)
    {
        diff += 2*CUAUV_PI;
    }
    return diff;
}
//This function brought to you by stackoverflow
void populate_samples_array(uint16_t * sample_arr, char * filename)
{
    
    char * buffer;
    char * record;
    int index = 0;
    FILE * f = fopen(filename,"r");
    fseek (f, 0, SEEK_END);
    long length = ftell (f);
    fseek (f, 0, SEEK_SET);
    buffer = malloc (length);
    if (buffer)
    {
    fread (buffer, sizeof(char), length, f);
    }
    fclose (f);
    record = strtok(buffer,",");
    index++;
        while(record != NULL)
        {
            if(index < SAMPLE_SIZE)
            {
            sample_arr[index] = (int) atof(record);
            index++;
            }

            record=strtok(NULL,",");
            
        }
    free(buffer);
    printf("%i\n",index); 

    
}
void printArr(uint16_t * arr, size_t len)
{
int i;
for(i = 0; i < len; i++)
{
    printf("%i,",arr[i]);
}
printf("\n");
}
