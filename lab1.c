#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 2000                     // Measurements/измерения
#define NORMAL_MIN 800           // range/диапазон
#define NORMAL_MAX 1200           //check the value /провер значение
#define ANOMALY_PERCENT 5
#define ANOMALY_LOW_MIN 0         
#define ANOMALY_LOW_MAX 100
#define ANOMALY_HIGH_MIN 5000     
#define ANOMALY_HIGH_MAX 10000
#define NEIGHBOR_RADIUS 2         // Radius for neighbor average filtering/Радиус для усреднения соседних значений

int isAnomaly(int value) {
    return value < NORMAL_MIN || value > NORMAL_MAX;
}


void sortArray(int arr[], int n) {
    for(int i=0;i<n-1;i++)
        for(int j=0;j<n-i-1;j++) //comparing neighbors /сравнивая соседей
            if(arr[j]>arr[j+1]){ 
                int tmp=arr[j]; arr[j]=arr[j+1]; arr[j+1]=tmp; //swaping them / заваливать их
            }
}


void printFirst50(int data[], int n, const char *title) {
    printf("\n %s (first 50 measurements)\n", title);

    // here we find max value to scale bars
    int max = data[0];
    for (int i = 1; i < n && i < 50; i++)
        if (data[i] > max) max = data[i];

    double scale = (max > 0) ? 50.0 / max : 1;  // Max bar length should be 50 so its more readable

    printf("Idx\tValue\tBar\n");
    for (int i = 0; i < n && i < 50; i++) {
        int barLen = (int)(data[i] * scale);
        if (barLen < 1) barLen = 1;

        printf("%3d\t%5d\t", i+1, data[i]);
        if (isAnomaly(data[i])) {
            for (int j=0;j<barLen;j++) printf("!");
        } else {
            for (int j=0;j<barLen;j++) printf("*");
        }
        printf("\n");
    }
}


// ASCII histogram for all data /Original Milk Data Distribution
void printHistogram(int data[], int n, const char *title) {
    printf("\n=== %s ===\n", title);

    int low=0, high=0, bins[10]={0};



    // Counting values
    for(int i=0;i<n;i++){
        if(data[i]<NORMAL_MIN) low++;
        else if(data[i]>NORMAL_MAX) high++;
        else bins[(data[i]-NORMAL_MIN)*10/(NORMAL_MAX-NORMAL_MIN+1)]++;
    }

   
    int maxCount = low>high ? low : high;
    for(int i=0;i<10;i++) if(bins[i]>maxCount) maxCount=bins[i];

    double scale = (maxCount>0) ? 50.0 / maxCount : 1; // Max bar length = 50

    
    printf("Low anomalies  : "); 
    int len = (int)(low * scale + 0.5);
    for(int i=0;i<len;i++) printf("!");
    printf(" (%d)\n",low);

    
    for(int i=0;i<10;i++){
        int start=NORMAL_MIN+i*(NORMAL_MAX-NORMAL_MIN+1)/10;
        int end=NORMAL_MIN+(i+1)*(NORMAL_MAX-NORMAL_MIN+1)/10-1;
        printf("%4d-%4d : ", start,end);
        len = (int)(bins[i]*scale + 0.5);
        for(int j=0;j<len;j++) printf("#");
        printf(" (%d)\n", bins[i]);
    }

    
    printf("High anomalies : "); 
    len = (int)(high * scale + 0.5);
    for(int i=0;i<len;i++) printf("!");
    printf(" (%d)\n",high);
}

int main() {
    srand(time(NULL));

    int data[N], filtered[N], anomalies[N]={0};


    // mini story part cuz why not

    printf("Hello Stranger!(or Mr.Sorokin) Welcome to the My Future Farm!\n");
    printf("The milkmaid robot (AKA Doyarka) collects data every 5 minutes.\n");
    printf("But watch out! Some cows kick the robot or trick it with water buckets!\n");


    // Normal data
    for(int i=0;i<N;i++) 
        data[i] = NORMAL_MIN + rand()%(NORMAL_MAX-NORMAL_MIN+1);

    // Inserting anomalies randomly (should be %5)
    int anomaly_needed = (N*ANOMALY_PERCENT+50)/100;  
    for(int i=0;i<anomaly_needed;){
        int pos = rand()%N;
        if(!isAnomaly(data[pos])) {   
            if(rand()%2==0)
                data[pos] = ANOMALY_LOW_MIN + rand()%(ANOMALY_LOW_MAX-ANOMALY_LOW_MIN+1);
            else
                data[pos] = ANOMALY_HIGH_MIN + rand()%(ANOMALY_HIGH_MAX-ANOMALY_HIGH_MIN+1);
            i++;
        }
    }

    
    int global_sum=0, global_count=0;
    for(int i=0;i<N;i++) if(!isAnomaly(data[i])) { global_sum+=data[i]; global_count++; }
    int global_avg = global_count>0 ? global_sum/global_count : (NORMAL_MIN+NORMAL_MAX)/2;

    for(int i=0;i<N;i++){
        filtered[i] = data[i];
        anomalies[i] = 0;
        if(isAnomaly(data[i])){
            anomalies[i]=1;
            int sum=0, count=0;
            
            for(int j=-NEIGHBOR_RADIUS;j<=NEIGHBOR_RADIUS;j++){ 
                int idx=i+j;
                if(idx>=0 && idx<N && !isAnomaly(data[idx])) { sum+=data[idx]; count++; }
            }
            filtered[i] = count>0 ? sum/count : global_avg;
        }
    }

    // basic statistics
    double sum_before=0, sum_after=0;
    int temp[N];
    for(int i=0;i<N;i++){ sum_before+=data[i]; sum_after+=filtered[i]; temp[i]=data[i]; }
    double mean_before = sum_before/N;
    double mean_after = sum_after/N;

    sortArray(temp,N);
    double median_before = N%2==0 ? (temp[N/2-1]+temp[N/2])/2.0 : temp[N/2];
    for(int i=0;i<N;i++) temp[i]=filtered[i];
    sortArray(temp,N);
    double median_after = N%2==0 ? (temp[N/2-1]+temp[N/2])/2.0 : temp[N/2];

    int min_normal=NORMAL_MAX+1,max_normal=NORMAL_MIN-1;
    int min_normal_after=NORMAL_MAX+1,max_normal_after=NORMAL_MIN-1;
    for(int i=0;i<N;i++){
        if(!isAnomaly(data[i])) { if(data[i]<min_normal) min_normal=data[i]; if(data[i]>max_normal) max_normal=data[i]; }
        if(!isAnomaly(filtered[i])) { if(filtered[i]<min_normal_after) min_normal_after=filtered[i]; if(filtered[i]>max_normal_after) max_normal_after=filtered[i]; }
    }

   
    printf("\n Statistics \n");
    printf("Before filtering: mean=%.2f median=%.2f min=%d max=%d\n",mean_before,median_before,min_normal,max_normal);
    printf("After filtering : mean=%.2f median=%.2f min=%d max=%d\n",mean_after,median_after,min_normal_after,max_normal_after);

    
    printFirst50(data,N,"Original Data");
    printFirst50(filtered,N,"Filtered Data");

    
    printHistogram(data,N,"Original Milk Data Distribution");
    printHistogram(filtered,N,"Filtered Milk Data Distribution");

    
    int anomaly_count=0; for(int i=0;i<N;i++) if(anomalies[i]) anomaly_count++;
    printf("\nTotal anomalies corrected: %d (%.5f%%)\n",anomaly_count,(double)anomaly_count*100/N);

    printf("\n That's pretty much it\n");

    return 0;
}